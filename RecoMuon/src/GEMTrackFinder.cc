/** \class GEMTrackFinder  
 * Produces a collection of tracks's in the test beam set up. 
 *
 * \author Jason Lee
 */

#include "gemsw/RecoMuon/interface/GEMTrackFinder.h"

#include "Geometry/CommonTopologies/interface/GEMStripTopology.h"

using namespace std;

GEMTrackFinder::GEMTrackFinder(const edm::ParameterSet& ps) 
  : gemg_(esConsumes<edm::Transition::BeginRun>())
{
  trackChi2_ = ps.getParameter<double>("trackChi2");
  direction_ = ps.getParameter<vector<double>>("direction");
  doFit_ = ps.getParameter<bool>("doFit");
  maxClusterSize_ = ps.getParameter<int>("maxClusterSize");
  minClusterSize_ = ps.getParameter<int>("minClusterSize");
  residualCut_ = ps.getParameter<double>("residual");
  theGEMRecHitToken_ = consumes<GEMRecHitCollection>(ps.getParameter<edm::InputTag>("gemRecHitLabel"));
  // register what this produces
  edm::ParameterSet serviceParameters = ps.getParameter<edm::ParameterSet>("ServiceParameters");
  theService_ = new MuonServiceProxy(serviceParameters, consumesCollector(), MuonServiceProxy::UseEventSetupIn::RunAndEvent);
  edm::ParameterSet smootherPSet = ps.getParameter<edm::ParameterSet>("MuonSmootherParameters");
  theSmoother_ = new StandAloneMuonSmoother(smootherPSet, theService_);

  produces<reco::TrackCollection>();
  produces<TrackingRecHitCollection>();
  produces<reco::TrackExtraCollection>();
  produces<vector<Trajectory> >();
  produces<vector<TrajectorySeed> >();
}

void GEMTrackFinder::beginRun(const edm::Run& run, const edm::EventSetup& setup) {
  edm::ESHandle<GEMGeometry> gemg = setup.getHandle(gemg_);
//  setup.get<MuonGeometryRecord>().get(gemg);
  const GEMGeometry* mgeom = &*gemg;

  detLayerMap_.clear();
  for (auto et : mgeom->etaPartitions()){
    detLayerMap_.insert( make_pair(alongDirection(et->position()), et) );
    excludingChambers_[et->id()] = false;
  }

  setSeedingLayers();
}

void GEMTrackFinder::produce(edm::Event& ev, const edm::EventSetup& setup) {
  //cout << "GEMTrackFinder::start producing segments for " << ++iev << "th event with gem data" << endl;  
  unique_ptr<reco::TrackCollection >          trackCollection( new reco::TrackCollection() );
  unique_ptr<TrackingRecHitCollection >       trackingRecHitCollection( new TrackingRecHitCollection() );
  unique_ptr<reco::TrackExtraCollection >     trackExtraCollection( new reco::TrackExtraCollection() );
  unique_ptr<vector<Trajectory> >             trajectorys( new vector<Trajectory>() );
  unique_ptr<vector<TrajectorySeed> >         trajectorySeeds( new vector<TrajectorySeed>() );
  TrackingRecHitRef::key_type recHitsIndex = 0;
  TrackingRecHitRefProd recHitCollectionRefProd = ev.getRefBeforePut<TrackingRecHitCollection>();
  reco::TrackExtraRef::key_type trackExtraIndex = 0;
  reco::TrackExtraRefProd trackExtraCollectionRefProd = ev.getRefBeforePut<reco::TrackExtraCollection>();
  
  theService_->update(setup);
  
  // get the collection of GEMRecHit
  edm::Handle<GEMRecHitCollection> gemRecHits;
  ev.getByToken(theGEMRecHitToken_,gemRecHits);

  if (gemRecHits->size() <3){
    ev.put(move(trajectorySeeds));
    ev.put(move(trackCollection));
    ev.put(move(trackingRecHitCollection));
    ev.put(move(trackExtraCollection));
    ev.put(move(trajectorys));
    return;
  }

  trajectorySeedCands_.clear();
  makeTrajectorySeeds(gemRecHits.product());

  //need to loop over seeds, make best track and save only best track
  Trajectory bestTrajectory;
  TrajectorySeed bestSeed;
  float maxChi2 = trackChi2_;
  for (auto seed : trajectorySeedCands_){
    Trajectory smoothed = makeTrajectory(seed, gemRecHits.product());
    if (smoothed.isValid()){
      if (maxChi2 > smoothed.chiSquared()/float(smoothed.ndof())){
        maxChi2 = smoothed.chiSquared()/float(smoothed.ndof());
        bestTrajectory = smoothed;
        bestSeed = seed;
      }
    }
  }
  if (!bestTrajectory.isValid()){
    ev.put(move(trajectorySeeds));
    ev.put(move(trackCollection));
    ev.put(move(trackingRecHitCollection));
    ev.put(move(trackExtraCollection));
    ev.put(move(trajectorys));
    return;
  }
  // make track
  const FreeTrajectoryState* ftsAtVtx = bestTrajectory.geometricalInnermostState().freeState();
  
  GlobalPoint pca = ftsAtVtx->position();
  math::XYZPoint persistentPCA(pca.x(),pca.y(),pca.z());
  GlobalVector p = ftsAtVtx->momentum();
  math::XYZVector persistentMomentum(p.x(),p.y(),p.z());
  
  reco::Track track(bestTrajectory.chiSquared(), 
                    bestTrajectory.ndof(true),
                    persistentPCA,
                    persistentMomentum,
                    ftsAtVtx->charge(),
                    ftsAtVtx->curvilinearError());

  //sets the outermost and innermost TSOSs
  TrajectoryStateOnSurface outertsos;
  TrajectoryStateOnSurface innertsos;
  unsigned int innerId, outerId;

  if (bestTrajectory.direction() == alongMomentum) {
    outertsos = bestTrajectory.lastMeasurement().updatedState();
    innertsos = bestTrajectory.firstMeasurement().updatedState();
    outerId = bestTrajectory.lastMeasurement().recHit()->geographicalId().rawId();
    innerId = bestTrajectory.firstMeasurement().recHit()->geographicalId().rawId();
  } else {
    outertsos = bestTrajectory.firstMeasurement().updatedState();
    innertsos = bestTrajectory.lastMeasurement().updatedState();
    outerId = bestTrajectory.firstMeasurement().recHit()->geographicalId().rawId();
    innerId = bestTrajectory.lastMeasurement().recHit()->geographicalId().rawId();
  }

  //build the TrackExtra
  GlobalPoint gv = outertsos.globalParameters().position();
  GlobalVector gp = outertsos.globalParameters().momentum();
  math::XYZVector outmom(gp.x(), gp.y(), gp.z());
  math::XYZPoint outpos(gv.x(), gv.y(), gv.z());
  gv = innertsos.globalParameters().position();
  gp = innertsos.globalParameters().momentum();
  math::XYZVector inmom(gp.x(), gp.y(), gp.z());
  math::XYZPoint inpos(gv.x(), gv.y(), gv.z());
  
  auto tx = reco::TrackExtra(outpos,
                             outmom,
                             true,
                             inpos,
                             inmom,
                             true,
                             outertsos.curvilinearError(),
                             outerId,
                             innertsos.curvilinearError(),
                             innerId,
                             bestSeed.direction(),
                             bestTrajectory.seedRef());

  TrackingRecHit::ConstRecHitContainer transHits = findMissingHits(bestTrajectory);
  unsigned int nHitsAdded = 0;
  for (Trajectory::RecHitContainer::const_iterator recHit = transHits.begin(); recHit != transHits.end(); ++recHit)
  {
    TrackingRecHit *singleHit = (**recHit).hit()->clone();
    trackingRecHitCollection->push_back( singleHit );
    ++nHitsAdded;
  }

  tx.setHits(recHitCollectionRefProd, recHitsIndex, nHitsAdded);
  recHitsIndex += nHitsAdded;

  trackExtraCollection->push_back(tx);

  reco::TrackExtraRef trackExtraRef(trackExtraCollectionRefProd, trackExtraIndex++ );
  track.setExtra(trackExtraRef);

  trackCollection->emplace_back(track);
  trajectorys->emplace_back(bestTrajectory);      
  trajectorySeeds->emplace_back(bestSeed);
  
  // fill the collection
  // put collection in event

  ev.put(move(trajectorySeeds));
  ev.put(move(trackCollection));
  ev.put(move(trackingRecHitCollection));
  ev.put(move(trackExtraCollection));
  ev.put(move(trajectorys));
  return;
}

void GEMTrackFinder::makeSeedsFromHits(MuonTransientTrackingRecHit::MuonRecHitContainer frontHits,
                                       MuonTransientTrackingRecHit::MuonRecHitContainer rearHits)
{
  unique_ptr<vector<TrajectorySeed> > trajectorySeeds( new vector<TrajectorySeed>());
  if (frontHits.size() > 0 && rearHits.size() > 0){
    
    for (auto fronthit : frontHits){
      for (auto rearhit : rearHits){
        GlobalVector segDirGV = rearhit->globalPosition() - fronthit->globalPosition();

        int charge= -1;
        AlgebraicSymMatrix mat(5,0);
        mat = fronthit->parametersError().similarityT( fronthit->projectionMatrix() );
        LocalTrajectoryError error(asSMatrix<5>(mat));
        LocalPoint segPos = fronthit->localPosition();
    
        LocalVector segDir = fronthit->det()->toLocal(segDirGV);
        LocalTrajectoryParameters param(segPos, segDir, charge);
        TrajectoryStateOnSurface tsos(param, error, fronthit->det()->surface(), &*theService_->magneticField());
          
        PTrajectoryStateOnDet seedTSOS = trajectoryStateTransform::persistentState(tsos, fronthit->rawId());
        
        edm::OwnVector<TrackingRecHit> seedHits;
        seedHits.push_back(fronthit->cloneHit());
        seedHits.push_back(rearhit->cloneHit());
    
        TrajectorySeed seed(seedTSOS,seedHits,alongMomentum);
        trajectorySeedCands_.emplace_back(seed);
      }
    }
  }
}

Trajectory GEMTrackFinder::makeTrajectory(TrajectorySeed& seed,
                                          const GEMRecHitCollection* gemHits)
{
  PTrajectoryStateOnDet ptsd1(seed.startingState());
  DetId did(ptsd1.detId());
  const BoundPlane& bp = theService_->trackingGeometry()->idToDet(did)->surface();
  TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService_->magneticField());
  Trajectory traj = Trajectory(seed);

  TrackingRecHit::ConstRecHitContainer consRecHits;
  
  float previousLayer = -200;//skip first layer
  for (auto chmap : detLayerMap_){
    //// skip same layers
    if (chmap.first == previousLayer) continue;
    previousLayer = chmap.first;
    
    auto refChamber = chmap.second;
    shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;

    tsos = theService_->propagator("StraightLinePropagator")->propagate(tsos,refChamber->surface());
    if (!tsos.isValid()){
      continue;
    }
    
    GlobalPoint tsosGP = tsos.globalPosition();
    //cout << "tsos gp   "<< tsosGP << refChamber->id() <<endl;
    
    float maxR = 5000;
    // find best in all layers
    float maxDist = 500000;
    for (auto col : detLayerMap_){
      // only look in same layer
      if (chmap.first != col.first) continue;
      auto etaPart = col.second;

      LocalPoint tsosLP = etaPart->toLocal(tsosGP);
      float tsosLP_strip = etaPart->strip(tsosLP);
      LocalPoint stripCentre = etaPart->centreOfStrip(tsosLP_strip);
      float dist = (stripCentre - tsosLP).mag();
      if (dist < maxDist) {
        refChamber = etaPart;
        maxDist = dist;
      }
    }
    for (auto col : detLayerMap_){
      // only look in same layer
      if (chmap.first != col.first) continue;      
      auto etaPart = col.second;
      GEMDetId etaPartID = etaPart->id();
      if (abs(etaPartID.ieta() - refChamber->id().ieta()) > 1) continue;
      
      GEMRecHitCollection::range range = gemHits->get(etaPartID);

      LocalPoint tsosLP = etaPart->toLocal(tsosGP);

      for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
 
        if (rechit->clusterSize() > maxClusterSize_ or rechit->clusterSize() < minClusterSize_) continue;
        LocalPoint rhLP = (*rechit).localPosition();

        auto tsosStrip = etaPart->strip(tsosLP);
        auto rhStrip = etaPart->strip(rhLP);

        if (abs(tsosStrip - rhStrip) > residualCut_) continue;

        // need to find best hits per chamber
        float deltaR = (rhLP - tsosLP).mag();
        if (maxR > deltaR){
          const GeomDet* geomDet(etaPart);  
          tmpRecHit = MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit);
          maxR = deltaR;
          if (excludingChambers_[etaPartID]) tmpRecHit->invalidateHit();
        }
      } 
    }
    
    if (tmpRecHit){      
      if (tmpRecHit->isValid()) consRecHits.emplace_back(tmpRecHit);
      traj.push(TrajectoryMeasurement(tsos, tmpRecHit), maxR);
    }
    else {
      const GeomDet* geomDet(refChamber);  
      GEMRecHit rechit(refChamber->id(), 0, refChamber->toLocal(tsosGP));
      tmpRecHit = MuonTransientTrackingRecHit::specificBuild(geomDet, &rechit);
      tmpRecHit->invalidateHit();
      traj.push(TrajectoryMeasurement(tsos, tmpRecHit), 0);
    }
  }
  if (consRecHits.size() <3) return Trajectory();

  if (not doFit_) return traj;

  auto result = theSmoother_->smooth(traj);
  auto fitted = result.second;

  if (fitted.chiSquared() < 0) return Trajectory();
  else return fitted;
}

TrackingRecHit::ConstRecHitContainer GEMTrackFinder::findMissingHits(Trajectory& track)
{
  TrajectoryStateOnSurface tsos = track.geometricalInnermostState();
  TrackingRecHit::ConstRecHitContainer recHits = track.recHits();
  auto measurements = track.measurements();
  
  float previousLayer = -200;//skip first layer
  int nmissing=0;
  for (auto chmap : detLayerMap_){
    //// skip same layers
    if (chmap.first == previousLayer) continue;
    previousLayer = chmap.first;

    bool hasHit = false;
    for (auto hit : recHits){
      if (abs(chmap.first - alongDirection(hit->globalPosition())) < 0.05 ){
        hasHit = true;
        break;
      }
    }
    if (hasHit) continue;

    auto refChamber = chmap.second;

    bool validTsos = false;
    for (auto measurement : measurements) {
      GEMDetId detId = measurement.recHit()->geographicalId();
      if (detId == refChamber->id()) {
        tsos = measurement.predictedState();
        validTsos = true;
        break;
      }
    }
    if (!validTsos){
      continue;
    }
    
    GlobalPoint tsosGP = tsos.globalPosition();

    shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
    ////no rechit, make missing hit
    for (auto col : detLayerMap_){
      // only look in same layer
      if (chmap.first != col.first) continue;
      auto etaPart = col.second;
      const LocalPoint pos = etaPart->toLocal(tsosGP);
      const LocalPoint pos2D(pos.x(), pos.y(), 0);
      const BoundPlane& bps(etaPart->surface());
        
      if (bps.bounds().inside(pos2D)){
        auto missingHit = make_unique<GEMRecHit>(etaPart->id(), -10, pos2D);
        const GeomDet* geomDet(etaPart);
        tmpRecHit = MuonTransientTrackingRecHit::specificBuild(geomDet,missingHit.get());
        tmpRecHit->invalidateHit();
        break;
      }
    }
    
    if (tmpRecHit){      
      recHits.push_back(tmpRecHit);
      ++nmissing;
    }
  }
  
  return recHits;
}

float GEMTrackFinder::alongDirection(const GlobalPoint& p) {
  return p.x() * direction_[0] + p.y() * direction_[1] + p.z() * direction_[2];
}

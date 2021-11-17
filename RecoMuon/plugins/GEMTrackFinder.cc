/** \class GEMTrackFinder  
 * Produces a collection of tracks's in the test beam set up. 
 *
 * \author Jason Lee
 */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include <Geometry/GEMGeometry/interface/GEMGeometry.h>
#include "Geometry/CommonTopologies/interface/GEMStripTopology.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrajectorySeed/interface/PropagationDirection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackingRecHit/interface/InvalidTrackingRecHit.h"

#include "RecoMuon/TransientTrackingRecHit/interface/MuonTransientTrackingRecHit.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "gemsw/RecoMuon/interface/MuonSmoother.h"
#include "RecoMuon/StandAloneTrackFinder/interface/StandAloneMuonSmoother.h"
//#include "TrackingTools/PatternTools/interface/MuonSmoother.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "RecoTracker/TrackProducer/src/TrajectoryToResiduals.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"

using namespace std;

class GEMTrackFinder : public edm::stream::EDProducer<> {
public:
  /// Constructor
  explicit GEMTrackFinder(const edm::ParameterSet&);
  /// Destructor
  virtual ~GEMTrackFinder() {}
  /// Produce the GEMSegment collection
  void produce(edm::Event&, const edm::EventSetup&) override;
  double trackChi2_;
  bool skipLargeChamber_;

  multimap<float,const GEMEtaPartition*> detLayerMap_;
  vector<TrajectorySeed> trajectorySeedCands_;
  
private:
  int iev; // events through
  edm::EDGetTokenT<GEMRecHitCollection> theGEMRecHitToken_;
  MuonSmoother* theSmoother_;
  MuonServiceProxy* theService_;
  KFUpdator* theUpdator_;

  MuonTransientTrackingRecHit::MuonRecHitContainer BuildSeedHits(const GEMEtaPartition* etaPartX, 
                                                                 const GEMEtaPartition* etaPartY, 
                                                                 const GEMRecHitCollection* gemHits);
  
  void findSeeds(MuonTransientTrackingRecHit::MuonRecHitContainer frontSeeds,
                 MuonTransientTrackingRecHit::MuonRecHitContainer rearSeeds);
  Trajectory makeTrajectory(TrajectorySeed& seed, const GEMRecHitCollection* gemHits);
  TrackingRecHit::ConstRecHitContainer findMissingHits(Trajectory& track);
  
};

GEMTrackFinder::GEMTrackFinder(const edm::ParameterSet& ps) : iev(0) {
  trackChi2_ = ps.getParameter<double>("trackChi2");
  skipLargeChamber_ = ps.getParameter<bool>("skipLargeChamber");
  theGEMRecHitToken_ = consumes<GEMRecHitCollection>(ps.getParameter<edm::InputTag>("gemRecHitLabel"));
  // register what this produces
  edm::ParameterSet serviceParameters = ps.getParameter<edm::ParameterSet>("ServiceParameters");
  theService_ = new MuonServiceProxy(serviceParameters, consumesCollector(), MuonServiceProxy::UseEventSetupIn::RunAndEvent);
  edm::ParameterSet smootherPSet = ps.getParameter<edm::ParameterSet>("MuonSmootherParameters");
  theSmoother_ = new MuonSmoother(smootherPSet,theService_);
  theUpdator_ = new KFUpdator();
  produces<reco::TrackCollection>();
  produces<TrackingRecHitCollection>();
  produces<reco::TrackExtraCollection>();
  produces<vector<Trajectory> >();
  produces<vector<TrajectorySeed> >();
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

  edm::ESHandle<GEMGeometry> gemg;
  setup.get<MuonGeometryRecord>().get(gemg);
  const GEMGeometry* mgeom = &*gemg;
  
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

  detLayerMap_.clear();
  vector<const GEMEtaPartition*> xChamb(4), yChamb(4);
  for (auto et : mgeom->etaPartitions()){    
    int ieta = et->id().ieta();
    int ch = et->id().chamber();
    int st = et->id().station();
    if ( st == 1 ) {
      int idx_offset = ch/2 - 1;
      if (ieta%2 == 1) {
        int local_idx = ieta/2;
        int idx = idx_offset*2 + local_idx;
        xChamb[idx] = et;
        detLayerMap_.insert( make_pair(et->position().y(), et) );
      } else {
        int local_idx = (ieta-1)/2;
        int idx = idx_offset*2 + local_idx;
        yChamb[idx] = et;
        detLayerMap_.insert( make_pair(et->position().y(), et) );
      }
    } else {
      detLayerMap_.insert( make_pair(et->position().y(), et) );
    }
    //cout << ch->id() << " y = " <<ch->position().y() <<endl;
    // save key as neg to sort from top to bottom
  }

  trajectorySeedCands_.clear();
  for (int i = 0; i < 2; i++) {
    auto frontSeeds = BuildSeedHits(xChamb[i], yChamb[i], gemRecHits.product());
    for (int j = 2; j < 4; j++) {
      auto rearSeeds = BuildSeedHits(xChamb[j], yChamb[j], gemRecHits.product());
      // Push_back trajectory candidates in trajectorySeedCands_
      findSeeds(frontSeeds, rearSeeds);
    }
  }
  //cout << "GEMTrackFinder::frontSeeds->size() " << frontSeeds.size() << endl;
  //cout << "GEMTrackFinder::trajectorySeeds->size() " << trajectorySeeds->size() << endl;

  //need to loop over seeds, make best track and save only best track
  //TrajectorySeed seed =trajectorySeeds->at(0);
  Trajectory bestTrajectory;
  TrajectorySeed bestSeed;
  float maxChi2 = trackChi2_;
  for (auto seed : trajectorySeedCands_){
    Trajectory smoothed = makeTrajectory(seed, gemRecHits.product());
    if (smoothed.isValid()){
      //cout << "GEMTrackFinder::Trajectory " << smoothed.foundHits() << endl;
      //cout << "GEMTrackFinder::Trajectory chiSquared/ndof " << smoothed.chiSquared()/float(smoothed.ndof()) << endl;
      //if (( maxChi2 > smoothed.chiSquared()/float(smoothed.ndof())) and ( smoothed.chiSquared()/float(smoothed.ndof()) > 7.0)){
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
  //cout << maxChi2 << endl;
  //cout << "GEMTrackFinder::bestTrajectory " << bestTrajectory.foundHits() << endl;
  //cout << "GEMTrackFinder::bestTrajectory chiSquared/ ndof " << bestTrajectory.chiSquared()/float(bestTrajectory.ndof()) << endl;
  //cout << maxChi2 << endl;
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

  reco::TrackExtra tx;
  //adding rec hits
  TrackingRecHit::ConstRecHitContainer transHits = findMissingHits(bestTrajectory);
  unsigned int nHitsAdded = 0;
  for (Trajectory::RecHitContainer::const_iterator recHit = transHits.begin(); recHit != transHits.end(); ++recHit) {
    TrackingRecHit *singleHit = (**recHit).hit()->clone();
    trackingRecHitCollection->push_back( singleHit );  
    ++nHitsAdded;
  }
  tx.setHits(recHitCollectionRefProd, recHitsIndex, nHitsAdded);
  tx.setResiduals(trajectoryToResiduals(bestTrajectory));
  tx.setSeedRef(bestTrajectory.seedRef());
  recHitsIndex +=nHitsAdded;

  trackExtraCollection->emplace_back(tx);
  reco::TrackExtraRef trackExtraRef(trackExtraCollectionRefProd, trackExtraIndex++ );
  track.setExtra(trackExtraRef);
  trackCollection->emplace_back(track);
  trajectorySeeds->emplace_back(bestSeed);
  trajectorys->emplace_back(bestTrajectory);      
  
  // fill the collection
  // put collection in event

  ev.put(move(trajectorySeeds));
  ev.put(move(trackCollection));
  ev.put(move(trackingRecHitCollection));
  ev.put(move(trackExtraCollection));
  ev.put(move(trajectorys));
  
}

void GEMTrackFinder::findSeeds(MuonTransientTrackingRecHit::MuonRecHitContainer frontSeeds,
                                    MuonTransientTrackingRecHit::MuonRecHitContainer rearSeeds)
{
  unique_ptr<vector<TrajectorySeed> > trajectorySeeds( new vector<TrajectorySeed>());
  if (frontSeeds.size() > 0 && rearSeeds.size() > 0){
    
    for (auto fronthit : frontSeeds){
      for (auto rearhit : rearSeeds){
        GlobalVector segDirGV = rearhit->globalPosition() - fronthit->globalPosition();

        int charge= -1;
        AlgebraicSymMatrix mat(5,0);
        mat = fronthit->parametersError().similarityT( fronthit->projectionMatrix() );
        LocalTrajectoryError error(asSMatrix<5>(mat));
        LocalPoint segPos = fronthit->localPosition();
    
        LocalVector segDir = fronthit->det()->toLocal(segDirGV);
        LocalTrajectoryParameters param(segPos, segDir, charge);
        TrajectoryStateOnSurface tsos(param, error, fronthit->det()->surface(), &*theService_->magneticField());
    
        //auto tsosBot = theService_->propagator("StraightLinePropagator")->propagate(tsos,rearhit->det()->surface());
        //cout << "GEMTrackFinder::tsos        " << tsos << endl;
        //tsos = theUpdator_->update(tsosBot, *rearhit);
        //cout << "GEMTrackFinder::tsos update " << tsos << endl;
          
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
    
    float maxR = 500;
    // find best in all layers
    for (auto col : detLayerMap_){
      // only look in same layer
      if (chmap.first != col.first) continue;      
      auto etaPart = col.second;
      GEMDetId etaPartID = etaPart->id();
      if (skipLargeChamber_ and etaPartID.station() != 1) continue;
      
      GEMRecHitCollection::range range = gemHits->get(etaPartID);
      //cout<< "Number of GEM rechits available , from chamber: "<< etaPartID<<endl;
      const GEMStripTopology* top_(dynamic_cast<const GEMStripTopology*>(&(etaPart->topology())));
      const float stripLength(top_->stripLength());
      const float stripPitch(etaPart->pitch());
      for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit){

        LocalPoint tsosLP = etaPart->toLocal(tsosGP);
        LocalPoint rhLP = (*rechit).localPosition();
        //double y_err = (*rechit).localPositionError().yy();
        if (abs(rhLP.x() - tsosLP.x()) > stripPitch*5) continue;
        if (abs(rhLP.y() - tsosLP.y()) > stripLength/2) continue;
        // need to find best hits per chamber
        float deltaR = (rhLP - tsosLP).mag();
        if (maxR > deltaR){
          // cout << " found hit   "<< etaPartID << " pos = "<< rhLP << " R = "<<deltaR <<endl;
          const GeomDet* geomDet(etaPart);  
          tmpRecHit = MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit);
          maxR = deltaR;
        }
      }
    }
    
    if (tmpRecHit){      
      consRecHits.emplace_back(tmpRecHit);
    }
  }
  if (consRecHits.size() <3) return Trajectory();

  auto firstHit = consRecHits.front();
  tsos = theService_->propagator("StraightLinePropagator")->propagate(tsos,*(firstHit->surface()));

  vector<Trajectory> fitted = theSmoother_->trajectories(seed, consRecHits, tsos);
  if(fitted.size() == 0) return Trajectory();
  else if (fitted.front().chiSquared() < 0) return Trajectory();
  else return fitted.front();
}

TrackingRecHit::ConstRecHitContainer GEMTrackFinder::findMissingHits(Trajectory& track)
{
  TrajectoryStateOnSurface tsos = track.geometricalInnermostState();
  TrackingRecHit::ConstRecHitContainer recHits = track.recHits();
  
  float previousLayer = -200;//skip first layer
  int nmissing=0;
  for (auto chmap : detLayerMap_){
    //// skip same layers
    if (chmap.first == previousLayer) continue;
    previousLayer = chmap.first;

    bool hasHit = false;
    for (auto hit : recHits){
      // cout <<" chmap.first "<< chmap.first
      //      << " -1*hit->globalPosition().y() "<< -1*hit->globalPosition().y()
      //      <<endl;
      if (abs(chmap.first - hit->globalPosition().y()) < 1. ){
        hasHit = true;
        break;
      }
    }
    if (hasHit) continue;

    auto refChamber = chmap.second;

    tsos = theService_->propagator("StraightLinePropagator")->propagate(tsos,refChamber->surface());
    if (!tsos.isValid()){
      continue;
    }
    
    GlobalPoint tsosGP = tsos.globalPosition();

    //cout << "tsos gp   "<< tsosGP << refChamber->id() <<endl;
    //cout << "tsos error "<< tsos.localError().positionError() << endl;
    
    shared_ptr<MuonTransientTrackingRecHit> tmpRecHit;
    ////no rechit, make missing hit
    for (auto col : detLayerMap_){
      // only look in same layer
      if (chmap.first != col.first) continue;
      auto etaPart = col.second;
      const LocalPoint pos = etaPart->toLocal(tsosGP);
      const LocalPoint pos2D(pos.x(), pos.y(), 0);
      const BoundPlane& bps(etaPart->surface());
        
      //if(abs(pos.y()) < 17 && abs(pos.x()) < 40 )
      //  cout << " missing hit "<< etaPart->id() << " pos = "<<pos<< " R = "<<pos.mag() <<" inside "
      //       <<  bps.bounds().inside(pos2D) <<endl;
        
      if (bps.bounds().inside(pos2D)){
        //if (!bp.bounds().inside(pos)) continue;
        //cout << "made missing hit "<<etaPartID   <<endl;
          
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
  //cout << "found "<<nmissing <<" missing hits"<<endl;
  
  return recHits;
}

MuonTransientTrackingRecHit::MuonRecHitContainer GEMTrackFinder::BuildSeedHits(const GEMEtaPartition* etaPartX, 
                                                                               const GEMEtaPartition* etaPartY, 
                                                                               const GEMRecHitCollection* gemHits)
{
  MuonTransientTrackingRecHit::MuonRecHitContainer hits;
  GEMDetId etaPartIDx = etaPartX->id();
  GEMDetId etaPartIDy = etaPartY->id();
  
  GEMRecHitCollection::range rangeX = gemHits->get(etaPartIDx);
  GEMRecHitCollection::range rangeY = gemHits->get(etaPartIDy);
  for (GEMRecHitCollection::const_iterator rechitX = rangeX.first; rechitX!=rangeX.second; ++rechitX){
    const GeomDet* geomDet(etaPartX);
    auto localPointX = rechitX->localPosition();
    auto localErrX = rechitX->localPositionError();
    for (GEMRecHitCollection::const_iterator rechitY = rangeY.first; rechitY!=rangeY.second; ++rechitY){
      auto localPointY = rechitY->localPosition();
      auto localErrY = rechitY->localPositionError();

      Float_t lpx = localPointX.x();
      Float_t lpy = -localPointY.x();
      Float_t lpz = localPointX.z();
      Float_t lpxx = localErrX.xx();
      Float_t lpyy = localErrY.xx();
      Float_t lpxy = 0;

      auto localPoint = LocalPoint(lpx, lpy, lpz);
      auto localError = LocalError(lpxx, lpxy, lpyy);
      auto rechit = rechitX->clone();
      rechit->setPositionAndError(localPoint, localError);

      hits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
    }
  }
  return hits;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GEMTrackFinder);

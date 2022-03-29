// -*- C++ -*-
//
// Package:    TB2021/GTRTrakExtrapoler
// Class:      GTRTrakExtrapoler
//
/**\class GTRTrakExtrapoler GTRTrakExtrapoler.cc TB2021/GTRTrakExtrapoler/plugins/GTRTrakExtrapoler.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  mmaggi
//         Created:  Tue, 25 Jan 2022 14:39:09 GMT
//
//

// system include files
#include <memory>
#include <utility> 

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHitFwd.h"
#include "TrackingTools/PatternTools/interface/Trajectory.h"
#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include <Geometry/GEMGeometry/interface/GEMGeometry.h>
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

// Alignments
#include "CondFormats/Alignment/interface/DetectorGlobalPosition.h"
#include "CondFormats/Alignment/interface/AlignmentErrorsExtended.h"
#include "CondFormats/AlignmentRecord/interface/GlobalPositionRcd.h"
#include "CondFormats/AlignmentRecord/interface/GEMAlignmentRcd.h"
#include "CondFormats/AlignmentRecord/interface/GEMAlignmentErrorExtendedRcd.h"
#include "Geometry/CommonTopologies/interface/GeometryAligner.h"
//
// class declaration
//

class GTRTrakExtrapoler : public edm::stream::EDProducer<> {
public:
  explicit GTRTrakExtrapoler(const edm::ParameterSet&);
  ~GTRTrakExtrapoler();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginStream(edm::StreamID) override;
  void produce(edm::Event&, const edm::EventSetup&) override;
  void endStream() override;


  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  //virtual void beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;
  //virtual void endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------
private:
  MuonServiceProxy* theService_;
  edm::EDGetTokenT<std::vector<Trajectory> > TrajToken_;
  edm::EDGetTokenT<GEMRecHitCollection> theGEMRecHitToken_;
  bool prout;
  unsigned int iTracker;

private:
  const GEMGeometry* mgeom;
  edm::ESGetToken<Alignments, GEMAlignmentRcd> alignmentsToken_;
  edm::ESGetToken<AlignmentErrorsExtended, GEMAlignmentErrorExtendedRcd> alignmentErrorsToken_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> geometry_token_;

};

//
// constants, enums and typedefs
//


//
// static data member definitions
//

//
// constructors and destructor
//
GTRTrakExtrapoler::GTRTrakExtrapoler(const edm::ParameterSet& iConfig)
{
  //register your products
  produces<GEMRecHitCollection>("Extrapolated");

  TrajToken_ = consumes<std::vector<Trajectory> >(iConfig.getParameter<edm::InputTag>("trackLabel"));
  theGEMRecHitToken_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitLabel"));
  edm::ParameterSet serviceParameters = iConfig.getParameter<edm::ParameterSet>("ServiceParameters");
  theService_ = new MuonServiceProxy(serviceParameters, consumesCollector(), MuonServiceProxy::UseEventSetupIn::RunAndEvent);
  iTracker = iConfig.getParameter<unsigned int>("TrackChamber");
  prout=iConfig.getParameter<bool>("doprint");

  alignmentsToken_ = esConsumes<Alignments, GEMAlignmentRcd, edm::Transition::BeginRun>();
  alignmentErrorsToken_ = esConsumes<AlignmentErrorsExtended, GEMAlignmentErrorExtendedRcd, edm::Transition::BeginRun>();
  geometry_token_ = esConsumes<GEMGeometry, MuonGeometryRecord, edm::Transition::BeginRun>();
}

GTRTrakExtrapoler::~GTRTrakExtrapoler() {
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void GTRTrakExtrapoler::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  edm::Handle<std::vector<Trajectory> > trajs;
  iEvent.getByToken(TrajToken_, trajs);
  GEMRecHitCollection rechits=iEvent.get(theGEMRecHitToken_);  
  if (trajs->size() != 1) return;

  theService_->update(iSetup);
  edm::ESHandle<Propagator>&& propagator1 = theService_->propagator("StraightLinePropagator");
  edm::ESHandle<Propagator>&& propagator2 = theService_->propagator("SteppingHelixPropagatorAny");

  std::vector<const GEMEtaPartition*> xCham(4),yCham(4);
  const GEMEtaPartition* TrCh=0;
  
  // store the x view of the tracker selected that should not 
  for (auto et : mgeom->etaPartitions()){
    int st = et->id().station();
    if (st == 1 ){
      int ieta = et->id().ieta();
      int ch = et->id().chamber();
      unsigned int idx = (ch/2-1)*2 + ieta/2 + ieta%2 ;
      if (idx == iTracker && ieta%2 == 1) TrCh = et;      
    }
  }
  std::stringstream os;
  os << "BARI" << std::setw(4) << std::setfill('0') << iTracker;
  std::string chname(os.str());
  if (prout) std::cout << " GEM Tracker Chamber " << chname << std::endl;

  if (TrCh == 0) {
    std::cout << chname << " +++++ The Tracker Chamber " << chname << "was not found!! " << std::endl;
    return;
  }
  if (prout) std::cout << chname << " == Tracker GEM selected " << TrCh->id() << std::endl;
  const BoundPlane& bp  = mgeom->idToDet(TrCh->id())->surface();

  // This will store the Trajectory measurements for all layers as tracking Rechits.
  std::unique_ptr<GEMRecHitCollection > gemRecHitCollection( new GEMRecHitCollection() );

  std::vector<TrajectoryMeasurement> meass = trajs->front().measurements();
  
  if (meass.size() != 6) {
    std::cout << chname << " +++++ Wrong number of track measurents! Id aspect 6 insetad I see " << meass.size() << std::endl;
  }

  if (prout) std::cout << chname << " Propagation to the Tracker Chamber starting from each measurement" << std::endl;
  // take the most precise extrapolation 
  double bestprecision = 1000.;
  TrajectoryStateOnSurface btsos;
  GlobalPoint bestPoint;
  for (std::vector<TrajectoryMeasurement>::iterator ms=meass.begin(); ms!=meass.end(); ms++) {
    if (prout) std::cout << chname << " == Measurement " << ms->updatedState().globalPosition() << " Rechit "  << ms->recHit()->globalPosition() << std::endl;       
    TrajectoryStateOnSurface tsosGE21_1 = propagator1->propagate(ms->updatedState(), bp);
    TrajectoryStateOnSurface tsosGE21_2 = propagator2->propagate(ms->updatedState(), bp);
    if(prout)
      std::cout << chname << " TSOS on " << chname << " :  straigthline  " << tsosGE21_1.globalPosition() << " +- ("
                << sqrt(tsosGE21_1.cartesianError().position().cxx()) << ", 0., "
                << sqrt(tsosGE21_1.cartesianError().position().czz()) << " )"
                << " | Helix " <<tsosGE21_2.globalPosition() << " +- ("
                << sqrt(tsosGE21_2.cartesianError().position().cxx()) << ", 0., "
                << sqrt(tsosGE21_2.cartesianError().position().czz()) << " )" << std::endl;
    
    double precision = tsosGE21_1.cartesianError().position().cxx() + tsosGE21_1.cartesianError().position().czz();

    if (precision < bestprecision) {
      bestprecision = precision;
      bestPoint = tsosGE21_1.globalPosition();
      btsos = tsosGE21_1;
    }
  }
  if (prout)
    std::cout << chname << " Best point " << bestPoint << " precision " << sqrt(bestprecision)*10000 << " um" << std::endl;
  
  LocalPoint lp = TrCh->toLocal(bestPoint);
  LocalPoint lpm(lp.x(), lp.y(), 0.);
  LocalError elo = btsos.localError().positionError();      
  
  if ( bp.bounds().inside(lpm,elo, 100.)) {
    if (prout) std::cout << chname << " Local P " << lp << " " 
                         << lpm << " Error on x " << sqrt(elo.xx()) 
                         << " Error on y " << sqrt(elo.yy()) 
                         << " inside ? " << bp.bounds().inside(lpm, elo, 100.) << std::endl;
    OwnVector<GEMRecHit> recHits;
    GEMRecHit* recHit = new GEMRecHit(TrCh->id(), 0, 0, 1, lpm, elo);
    recHits.push_back(recHit);
    gemRecHitCollection->put(TrCh->id(), recHits.begin(), recHits.end());
    GEMRecHitCollection::range rh = rechits.get(TrCh->id());
    int nrh = 0;
    for (auto ix = rh.first; ix != rh.second; ++ix) {
      if (prout) std::cout << chname << " RECHITS " << ix->localPosition() << std::endl;
      nrh+=1;
    }
    if (prout) std::cout << chname << " +++ Number of Rechits " << nrh << std::endl;
  }
  iEvent.put(move(gemRecHitCollection), "Extrapolated");
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void GTRTrakExtrapoler::beginStream(edm::StreamID) {
  // please remove this method if not needed
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void GTRTrakExtrapoler::endStream() {
  // please remove this method if not needed
}

// ------------ method called when starting to processes a run  ------------

void
GTRTrakExtrapoler::beginRun(edm::Run const& run, edm::EventSetup const& setup)
{

  edm::ESHandle<GEMGeometry> gemg = setup.getHandle(geometry_token_);
  mgeom = &*gemg;

  const auto& alignments = setup.getData(alignmentsToken_);
  const auto& alignmentErrors = setup.getData(alignmentErrorsToken_);
  GeometryAligner aligner;

  aligner.applyAlignments<GEMGeometry>(mgeom,
                                       &alignments,
                                       &alignmentErrors,
                                       AlignTransform());
}


// ------------ method called when ending the processing of a run  ------------
/*
void
GTRTrakExtrapoler::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when starting to processes a luminosity block  ------------
/*
void
GTRTrakExtrapoler::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
GTRTrakExtrapoler::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void GTRTrakExtrapoler::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GTRTrakExtrapoler);

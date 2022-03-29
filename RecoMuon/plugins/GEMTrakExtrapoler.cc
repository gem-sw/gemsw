// -*- C++ -*-
//
// Package:    TB2021/GEMTrakExtrapoler
// Class:      GEMTrakExtrapoler
//
/**\class GEMTrakExtrapoler GEMTrakExtrapoler.cc TB2021/GEMTrakExtrapoler/plugins/GEMTrakExtrapoler.cc

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

class GEMTrakExtrapoler : public edm::stream::EDProducer<> {
public:
  explicit GEMTrakExtrapoler(const edm::ParameterSet&);
  ~GEMTrakExtrapoler();

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
GEMTrakExtrapoler::GEMTrakExtrapoler(const edm::ParameterSet& iConfig)
{
  //register your products
  produces<GEMRecHitCollection>("Extrapolated");
  prout = false;
  TrajToken_ = consumes<std::vector<Trajectory> >(iConfig.getParameter<edm::InputTag>("trackLabel"));
  theGEMRecHitToken_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitLabel"));
  edm::ParameterSet serviceParameters = iConfig.getParameter<edm::ParameterSet>("ServiceParameters");
  theService_ = new MuonServiceProxy(serviceParameters, consumesCollector(), MuonServiceProxy::UseEventSetupIn::RunAndEvent);
  prout=iConfig.getParameter<bool>("doprint");

  alignmentsToken_ = esConsumes<Alignments, GEMAlignmentRcd, edm::Transition::BeginRun>();
  alignmentErrorsToken_ = esConsumes<AlignmentErrorsExtended, GEMAlignmentErrorExtendedRcd, edm::Transition::BeginRun>();
  geometry_token_ = esConsumes<GEMGeometry, MuonGeometryRecord, edm::Transition::BeginRun>();

}

GEMTrakExtrapoler::~GEMTrakExtrapoler() {
  // do anything here that needs to be done at destruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called to produce the data  ------------
void GEMTrakExtrapoler::produce(edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  edm::Handle<std::vector<Trajectory> > trajs;
  iEvent.getByToken(TrajToken_, trajs);
  GEMRecHitCollection rechits = iEvent.get(theGEMRecHitToken_);  
  if (prout) std::cout << "GE21 Number of Trakectory found " << trajs->size() << std::endl;
  if (trajs->size() != 1) return;

  theService_->update(iSetup);
  edm::ESHandle<Propagator>&& propagator1 = theService_->propagator("StraightLinePropagator");
  edm::ESHandle<Propagator>&& propagator2 = theService_->propagator("SteppingHelixPropagatorAny");

  GEMDetId GEM2Id,GEM2etaId;
  const GEMChamber* ch21=0;
  for (auto ch : mgeom->chambers() ){
    // std::cout <<" ID "<<ch->id()<<std::endl;
    if (ch->id().station() == 2) {
      GEM2Id = ch->id();
      ch21 = ch;
    }
  }
  for (auto etref : mgeom->etaPartitions() ) {
    // std::cout <<" ID "<<etref->id()<<std::endl;
    if (etref->id().station() == 2 && etref->id().ieta() == 16) {
      GEM2etaId=etref->id();
    }
  }
  // const BoundPlane& bp  = mgeom->idToDet(GEM2Id)->surface();
  const BoundPlane& bpe = mgeom->idToDet(GEM2etaId)->surface();
  // std::cout <<" Target ID " <<GEM2Id<<std::endl;

  // This will store the Trajectory measurements for all layers as tracking Rechits.
  std::unique_ptr<GEMRecHitCollection > gemRecHitCollection( new GEMRecHitCollection() );

  std::vector<TrajectoryMeasurement> meass = trajs->front().measurements();
  if (prout) std::cout << "GE21 Number of measuremnents " << meass.size() << std::endl;
  for (std::vector<TrajectoryMeasurement>::iterator ms=meass.begin(); ms!=meass.end(); ms++) {
    // std::cout <<" Measurement "<<ms->updatedState().globalPosition()<<" Rechit " <<ms->recHit()->globalPosition()<<std::endl;   
  }
  if(prout) std::cout << "GE21 Propagation to the GE2/1 starting from each measurement" << std::endl;
  // take the most precise extrapolation 
  double bestprecision = 1000.;
  TrajectoryStateOnSurface btsos;
  GlobalPoint bestPoint;
  for (std::vector<TrajectoryMeasurement>::iterator ms=meass.begin(); ms!=meass.end(); ms++) {
    TrajectoryStateOnSurface tsosGE21_1 = propagator1->propagate(ms->updatedState(),bpe);
    TrajectoryStateOnSurface tsosGE21_2 = propagator2->propagate(ms->updatedState(),bpe);
    /*
    std::cout << " TSOS on GE21 :  straigthline  " << tsosGE21_1.globalPosition() << " +- ("
              << sqrt(tsosGE21_1.cartesianError().position().cxx()) << ", 0., "
              << sqrt(tsosGE21_1.cartesianError().position().czz()) << " )"
              << " | Helix " <<tsosGE21_2.globalPosition() << " +- ("
              << sqrt(tsosGE21_2.cartesianError().position().cxx()) << ", 0., "
              << sqrt(tsosGE21_2.cartesianError().position().czz()) << " )" << std::endl;
    */
    double precision = tsosGE21_1.cartesianError().position().cxx() + tsosGE21_1.cartesianError().position().czz();

    if (precision < bestprecision) {
      bestprecision = precision;
      bestPoint = tsosGE21_1.globalPosition();
      btsos = tsosGE21_1;
    }
  }
  if (prout)
    std::cout << "GE21 Best point " << bestPoint << " precision " << sqrt(bestprecision)*10000 << " um" << std::endl;
  
  //Loop over the etapartition of the GE21 to see which to associate
  //  std::cout <<"NUmer of partition of the GE21 "<<ch21->nEtaPartitions()<<std::endl;
  std::vector<const GEMEtaPartition*> etas = ch21->etaPartitions();
  for (auto eta = etas.begin(); eta!=etas.end(); eta++) {
    //check if the extrapolated point is inside the bounds of the eta partition
    // to do that we tranform the global coord into local and check inside the bounds.
    // std::cout <<" ETA = "<<(*eta)->id().ieta()<<std::endl;
    LocalPoint lp = (*eta)->toLocal(bestPoint);
    LocalPoint lpm(lp.x(),lp.y(),0.);
    LocalError elo = btsos.localError().positionError();      
    const BoundPlane& bpp = (*eta)->surface();
    if ( bpp.bounds().inside(lpm, elo, 100.) ) {
      if (prout) {
        std::cout << "GE21 Local P " << lp << " " << lpm 
                  << " Error on x " << sqrt(elo.xx()) 
                  << " Error on y " << sqrt(elo.yy()) 
                  << " inside ? " << bpp.bounds().inside(lpm, elo, 100.) << std::endl;
      }
      OwnVector<GEMRecHit> recHits;
      GEMRecHit* recHit = new GEMRecHit((*eta)->id(), 0, 0, 1, lpm, elo);
      recHits.push_back(recHit);
      gemRecHitCollection->put((*eta)->id(), recHits.begin(), recHits.end());
      GEMRecHitCollection::range rh = rechits.get((*eta)->id());
      int nrh = 0;
      for (auto ix = rh.first; ix != rh.second; ++ix) {
        if (prout) std::cout << "GE21 RECHITS " << ix->localPosition() << std::endl;
        nrh+=1;
      }
      if (prout)
        std::cout << "GE21 +++ Number of Rechits " << nrh << std::endl;
    }
  }

  iEvent.put(move(gemRecHitCollection), "Extrapolated");

}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void GEMTrakExtrapoler::beginStream(edm::StreamID) {
  // please remove this method if not needed
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void GEMTrakExtrapoler::endStream() {
  // please remove this method if not needed
}

// ------------ method called when starting to processes a run  ------------

void
GEMTrakExtrapoler::beginRun(edm::Run const& run , edm::EventSetup const& setup)
{

  edm::ESHandle<GEMGeometry> gemg = setup.getHandle(geometry_token_);
  mgeom = &*gemg;

  const auto& alignments = setup.getData(alignmentsToken_);
  //  std::cout<<" Get alignement"<<std::endl;                                                                                                                
  const auto& alignmentErrors = setup.getData(alignmentErrorsToken_);
  // std::cout<<" Get alignement errro"<<std::endl;                                                                                                           
  GeometryAligner aligner;
  // std::cout <<" Applying alignemnt"<<std::endl;                                                                                                            
  aligner.applyAlignments<GEMGeometry>(mgeom,
                                       &alignments,
                                       &alignmentErrors,
                                       AlignTransform());
}


// ------------ method called when ending the processing of a run  ------------
/*
void
GEMTrakExtrapoler::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when starting to processes a luminosity block  ------------
/*
void
GEMTrakExtrapoler::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
GEMTrakExtrapoler::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void GEMTrakExtrapoler::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GEMTrakExtrapoler);

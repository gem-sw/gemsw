// cd /cms/ldap_home/iawatson/scratch/GEM/CMSSW_10_1_5/src/ && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// cd ../../.. && source /cvmfs/cms.cern.ch/cmsset_default.sh && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// system include files
#include <memory>
#include <cmath>
#include <iostream>
#include <map>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
// GEM
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/GEMRecHit/interface/GEMSegmentCollection.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartition.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"
#include "Geometry/CommonTopologies/interface/GEMStripTopology.h"
// Muon
#include "DataFormats/PatCandidates/interface/Muon.h"
#include "DataFormats/TrackReco/interface/Track.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/CommonDetUnit/interface/GeomDet.h"

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/Run.h"

#include "TH1D.h"
#include "TH2D.h"
#include "TProfile.h"
#include "TString.h"
#include "TGraphAsymmErrors.h"
#include "TLorentzVector.h"
#include "TTree.h"

using namespace std;

typedef std::tuple<int, int> Key2;
typedef std::tuple<int, int, int> Key3;

class TestBeamTrackAnalyzer : public edm::EDAnalyzer {
public:
  explicit TestBeamTrackAnalyzer(const edm::ParameterSet&);
  ~TestBeamTrackAnalyzer();

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void beginJob() override;
  virtual void endJob() override;

  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------
  edm::Service<TFileService> fs;
  edm::EDGetTokenT<reco::TrackCollection> tracks_;
  edm::EDGetTokenT<TrajectorySeed> seeds_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;

  MuonServiceProxy* theService_;

  TH1D* trackChi2_;

  std::map<Key2, TH2D*> track_occ_;
  std::map<Key2, TH2D*> rechit_occ_;
  std::map<Key2, TH1D*> residual_x_;
  std::map<Key2, TH2D*> residual_x_cls_;
  std::map<Key2, TH1D*> residual_y_;
  std::map<Key2, TH1D*> trackingError_x_;
  std::map<Key2, TH1D*> trackingError_y_;
  std::map<Key2, TH2D*> track_rechit_;

  std::map<Key3, TH2D*> track_occ_detail_;
  std::map<Key3, TH2D*> rechit_occ_detail_;
  std::map<Key3, TH1D*> residual_x_detail_;
  std::map<Key3, TH2D*> residual_x_cls_detail_;
  std::map<Key3, TH1D*> residual_y_detail_;
  std::map<Key3, TH1D*> trackingError_x_detail_;
  std::map<Key3, TH1D*> trackingError_y_detail_;
  std::map<Key3, TH2D*> track_rechit_detail_;

};

TestBeamTrackAnalyzer::TestBeamTrackAnalyzer(const edm::ParameterSet& iConfig)
{ 
  tracks_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"));
  //seeds_ = consumes<TrajectorySeed>(iConfig.getParameter<edm::InputTag>("seeds"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));
  edm::ParameterSet serviceParameters = iConfig.getParameter<edm::ParameterSet>("ServiceParameters");
  theService_ = new MuonServiceProxy(serviceParameters, consumesCollector(), MuonServiceProxy::UseEventSetupIn::RunAndEvent);

  trackChi2_ = fs->make<TH1D>("track_chi2", "Normalized Track Chi2", 100, 0, 10);

  int station = 0;
  int chamber = 1;
  Key2 key2(station, chamber);
  track_occ_[key2]= fs->make<TH2D>("track_occ_GE0", 
                                   "Occupancy from Track : GE0",
                                   20, -60, 60, 
                                   20, 100, 140);
  rechit_occ_[key2] = fs->make<TH2D>("rechit_occ_GE0", 
                                     "Occupancy from Matched RecHit : GE0",
                                     20, -60, 60, 
                                     20, 100, 140);
  track_rechit_[key2] = fs->make<TH2D>("track_rechit_occ_GE0",
                                       "Occpancy from Track vs RecHit : GE0",
                                       200, -10, 10,
                                       200, -10, 10);
  residual_x_[key2] = fs->make<TH1D>("residual_x_GE0",
                                     "residual X : GE0",
                                     1000, -5, 5);
  residual_x_cls_[key2] = fs->make<TH2D>("residual_x_cls_GE0",
                                         "residual X : GE0",
                                         1000, -5, 5,
                                         10, 1, 11);
  residual_y_[key2] = fs->make<TH1D>("residual_y_GE0",
                                     "residual Y : GE0",
                                     10, -20, 20);
  trackingError_x_[key2] = fs->make<TH1D>("trackingError_x_GE0",
                                          "trackingError X : GE0",
                                          20, -1, 1);
  trackingError_y_[key2] = fs->make<TH1D>("trackingError_y_GE0",
                                          "trackingError Y : GE0",
                                          20, -1, 1);
  for (int ieta = 1; ieta < 9; ieta++) {
    Key3 key3(station, chamber, ieta);
    track_occ_detail_[key3]= fs->make<TH2D>(Form("track_occ_GE0_ieta%d",ieta), 
                                            Form("Occupancy from Track : GE0 iEta%d",ieta),
                                            20, -60, 60, 
                                            20, 100, 140);
    rechit_occ_detail_[key3] = fs->make<TH2D>(Form("rechit_occ_GE0_ieta%d",ieta), 
                                              Form("Occupancy from Matched RecHit : GE0 iEta%d",ieta),
                                              20, -60, 60, 
                                              20, 100, 140);
    track_rechit_detail_[key3] = fs->make<TH2D>(Form("track_rechit_occ_GE0_ieta%d",ieta),
                                                Form("Occpancy from Track vs RecHit : GE0 iEta%d",ieta),
                                                200, -10, 10,
                                                200, -10, 10);
    residual_x_detail_[key3] = fs->make<TH1D>(Form("residual_x_GE0_ieta%d",ieta),
                                              Form("residual X : GE0 iEta%d",ieta),
                                              1000, -5, 5);
    residual_x_cls_detail_[key3] = fs->make<TH2D>(Form("residual_x_cls_GE0_ieta%d",ieta),
                                                  Form("residual X : GE0 iEta%d",ieta),
                                                  1000, -5, 5,
                                                  10, 1, 11);
    residual_y_detail_[key3] = fs->make<TH1D>(Form("residual_y_GE0_ieta%d",ieta),
                                              Form("residual Y : GE0 iEta%d",ieta),
                                              10, -20, 20);
    trackingError_x_detail_[key3] = fs->make<TH1D>(Form("trackingError_x_GE0_ieta%d", ieta),
                                                   Form("trackingError X : GE0_ieta%d", ieta),
                                                   20, -1, 1);
    trackingError_y_detail_[key3] = fs->make<TH1D>(Form("trackingError_y_GE0_ieta%d", ieta),
                                                   Form("trackingError Y : GE0_ieta%d", ieta),
                                                   20, -1, 1);
  }

  station = 2;
  chamber = 1;
  key2 = Key2(station, chamber);
  track_occ_[key2]= fs->make<TH2D>("track_occ_GE21", 
                                   "Occupancy from Track : GE21",
                                   20, -60, 60, 
                                   20, 100, 140);
  rechit_occ_[key2] = fs->make<TH2D>("rechit_occ_GE21", 
                                     "Occupancy from Matched RecHit : GE21",
                                     20, -60, 60, 
                                     20, 100, 140);
  track_rechit_[key2] = fs->make<TH2D>("track_rechit_occ_GE21",
                                       "Occpancy from Track vs RecHit : GE21",
                                       200, 0, 40,
                                       200, 0, 40);
  residual_x_[key2] = fs->make<TH1D>("residual_x_GE21",
                                     "residual X : GE21",
                                     1000, -5, 5);
  residual_x_cls_[key2] = fs->make<TH2D>("residual_x_cls_GE21",
                                         "residual X : GE21",
                                         1000, -5, 5,
                                         10, 1, 11);
  residual_y_[key2] = fs->make<TH1D>("residual_y_GE21",
                                     "residual Y : GE21",
                                     10, -20, 20);
  trackingError_x_[key2] = fs->make<TH1D>("trackingError_x_GE21",
                                          "trackingError X : GE21",
                                          20, -1, 1);
  trackingError_y_[key2] = fs->make<TH1D>("trackingError_y_GE21",
                                          "trackingError Y : GE21",
                                          20, -1, 1);
  for (int ieta = 1; ieta < 17; ieta++) {
    Key3 key3(station, chamber, ieta);
    track_occ_detail_[key3]= fs->make<TH2D>(Form("track_occ_GE21_ieta%d",ieta), 
                                            Form("Occupancy from Track : GE21 iEta%d",ieta),
                                            20, -60, 60, 
                                            20, 100, 140);
    rechit_occ_detail_[key3] = fs->make<TH2D>(Form("rechit_occ_GE21_ieta%d",ieta), 
                                              Form("Occupancy from Matched RecHit : GE21 iEta%d",ieta),
                                              20, -60, 60, 
                                              20, 100, 140);
    track_rechit_detail_[key3] = fs->make<TH2D>(Form("track_rechit_occ_GE21_ieta%d",ieta),
                                                Form("Occpancy from Track vs RecHit : GE21 iEta%d",ieta),
                                                200, 0, 40,
                                                200, 0, 40);
    residual_x_detail_[key3] = fs->make<TH1D>(Form("residual_x_GE21_ieta%d",ieta),
                                              Form("residual X : GE21 iEta%d",ieta),
                                              1000, -5, 5);
    residual_x_cls_detail_[key3] = fs->make<TH2D>(Form("residual_x_cls_GE21_ieta%d",ieta),
                                                  Form("residual X : GE21 iEta%d",ieta),
                                                  1000, -5, 5,
                                                  10, 1, 11);
    residual_y_detail_[key3] = fs->make<TH1D>(Form("residual_y_GE21_ieta%d",ieta),
                                              Form("residual Y : GE21 iEta%d",ieta),
                                              10, -20, 20);
    trackingError_x_detail_[key3] = fs->make<TH1D>(Form("trackingError_x_GE21_ieta%d", ieta),
                                                   Form("trackingError X : GE21_ieta%d", ieta),
                                                   20, -1, 1);
    trackingError_y_detail_[key3] = fs->make<TH1D>(Form("trackingError_y_GE21_ieta%d", ieta),
                                                   Form("trackingError Y : GE21_ieta%d", ieta),
                                                   20, -1, 1);
  }

  station = 1;
  for (int ch = 2; ch < 5; ch+=2) {
    Key2 key2(station, ch);
    track_occ_[key2]= fs->make<TH2D>(Form("track_occ_GE11_ch%d", ch), 
                                     Form("Occupancy from Track : GE11 chamber %d", ch),
                                     20, -5, 5, 
                                     20, -5, 5);
    rechit_occ_[key2] = fs->make<TH2D>(Form("rechit_occ_GE11_ch%d", ch), 
                                       Form("Occupancy from Matched RecHit : GE11 chamber %d", ch),
                                       20, -5, 5, 
                                       20, -5, 5);
    track_rechit_[key2] = fs->make<TH2D>(Form("track_rechit_occ_GE11_ch%d", ch),
                                         Form("Occpancy from Track vs RecHit : GE11 chamber %d", ch),
                                         200, -5, 5,
                                         200, -5, 5);
    residual_x_[key2] = fs->make<TH1D>(Form("residual_x_GE11_ch%d", ch),
                                       Form("residual X : GE11 chamber %d", ch),
                                       200, -5, 5);
    residual_x_cls_[key2] = fs->make<TH2D>(Form("residual_x_cls_GE11_ch%d", ch),
                                           Form("residual X : GE11 chamber %d", ch),
                                           200, -5, 5,
                                           10, 1, 11);
    residual_y_[key2] = fs->make<TH1D>(Form("residual_y_GE11_ch%d", ch),
                                       Form("residual Y : GE11 chamber %d", ch),
                                       10, -20, 20);
    trackingError_x_[key2] = fs->make<TH1D>(Form("trackingError_x_GE11_ch%d", ch),
                                            Form("trackingError X : GE11_ch%d", ch),
                                            20, -1, 1);
    trackingError_y_[key2] = fs->make<TH1D>(Form("trackingError_y_GE11_ch%d", ch),
                                            Form("trackingError Y : GE11_ch%d", ch),
                                            20, -1, 1);
    for (int ieta = 1; ieta < 6; ieta++) {
      Key3 key3(station, ch, ieta);
      track_occ_detail_[key3]= fs->make<TH2D>(Form("track_occ_GE11_ch%d_ieta%d", ch, ieta), 
                                              Form("Occupancy from Track : GE11 chamber %d iEta%d", ch, ieta),
                                              20, -5, 5, 
                                              20, -5, 5);
      rechit_occ_detail_[key3] = fs->make<TH2D>(Form("rechit_occ_GE11_ch%d_ieta%d", ch, ieta), 
                                                Form("Occupancy from Matched RecHit : GE11 chamber %d iEta%d", ch, ieta),
                                                20, -5, 5, 
                                                20, -5, 5);
      track_rechit_detail_[key3] = fs->make<TH2D>(Form("track_rechit_occ_GE11_ch%d_ieta%d", ch, ieta),
                                                  Form("Occpancy from Track vs RecHit : GE11 chamber %d iEta%d", ch, ieta),
                                                  200, -5, 5,
                                                  200, -5, 5);
      residual_x_detail_[key3] = fs->make<TH1D>(Form("residual_x_GE11_ch%d_ieta%d", ch, ieta),
                                                Form("residual X : GE11 chamber %d iEta%d", ch, ieta),
                                                1000, -5, 5);
      residual_x_cls_detail_[key3] = fs->make<TH2D>(Form("residual_x_cls_GE11_ch%d_ieta%d", ch, ieta),
                                                    Form("residual X : GE11 chamber %d iEta%d", ch, ieta),
                                                    1000, -5, 5,
                                                    10, 1, 11);
      residual_y_detail_[key3] = fs->make<TH1D>(Form("residual_y_GE11_ch%d_ieta%d", ch, ieta),
                                                Form("residual Y : GE11 chamber %d iEta%d", ch, ieta),
                                                10, -20, 20);
      trackingError_x_detail_[key3] = fs->make<TH1D>(Form("trackingError_x_GE11_ch%d_ieta%d", ch, ieta),
                                                     Form("trackingError X : GE11_ch%d_ieta%d", ch, ieta),
                                                     20, -1, 1);
      trackingError_y_detail_[key3] = fs->make<TH1D>(Form("trackingError_y_GE11_ch%d_ieta%d", ch, ieta),
                                                     Form("trackingError Y : GE11_ch%d_ieta%d", ch, ieta),
                                                     20, -1, 1);
    }
  }
}

TestBeamTrackAnalyzer::~TestBeamTrackAnalyzer(){}

void
TestBeamTrackAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom;
  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<vector<reco::Track>> tracks;
  iEvent.getByToken(tracks_, tracks);

  //edm::Handle<vector<TrajectorySeed>> seeds;
  //iEvent.getByToken(seeds_, seeds);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_,gemRecHits);

  for (std::vector<reco::Track>::const_iterator track = tracks->begin(); track != tracks->end(); ++track)
  {
    trackChi2_->Fill(track->normalizedChi2());
    //auto seed = track->seedRef();
    //auto seed = seeds->begin();
    //auto ptsd1 = seed->startingState();
    //DetId did(ptsd1.detId());
    //const BoundPlane& bp = theService_->trackingGeometry()->idToDet(did)->surface();
    //TrajectoryStateOnSurface tsos = trajectoryStateTransform::transientState(ptsd1,&bp,&*theService_->magneticField());

    for (auto hit : track->recHits()) {
      auto etaPart = GEMGeometry_->etaPartition(hit->geographicalId());
      auto etaPartId = etaPart->id();

      //tsos = theService_->propagator("StraightLinePropagator")->propagate(tsos,etaPart->surface());
      //if (!tsos.isValid()) continue;
      //auto tsos_error = tsos.localError().positionError();

      auto lp_track = hit->localPosition();
      auto gp_track = etaPart->toGlobal(lp_track);

      auto range = gemRecHits->get(etaPartId);

      bool hasHit = false;
      float maxR = 500000;
      float lp_x = 0;
      int cls_size=0;
      float residualX = 0;
      float residualY = 0;

      int station = etaPartId.station();
      int chamber = etaPartId.chamber();
      int ieta = etaPartId.ieta();

      Key2 key2(station, chamber);
      Key3 key3(station, chamber, ieta);
    
      track_occ_[key2]->Fill(gp_track.x(), gp_track.y());
      track_occ_detail_[key3]->Fill(lp_track.x(), lp_track.y());
      //trackingError_x_[key2]->Fill(tsos_error.xx());
      //trackingError_y_[key2]->Fill(tsos_error.yy());
      //trackingError_x_detail_[key3]->Fill(tsos_error.xx());
      //trackingError_y_detail_[key3]->Fill(tsos_error.yy());

      for (auto rechit = range.first; rechit != range.second; ++rechit) {
        auto lp_rechit = rechit->localPosition();

        auto deltaR = (lp_rechit - lp_track).mag();
        if (deltaR < maxR) {
          hasHit = true;
          maxR = deltaR; 
          residualX = (lp_rechit.x() - lp_track.x())*10.;
          residualY = lp_rechit.y() - lp_track.y();
          cls_size = rechit->clusterSize();
          lp_x = lp_rechit.x();
        }
      }
      if (hasHit) {
        rechit_occ_[key2]->Fill(gp_track.x(), gp_track.y());
        track_rechit_[key2]->Fill(lp_track.x(), lp_x);
        residual_x_[key2]->Fill(residualX);
        residual_x_cls_[key2]->Fill(residualX, cls_size);
        residual_y_[key2]->Fill(residualY);

        rechit_occ_detail_[key3]->Fill(lp_track.x(), lp_track.y());
        track_rechit_detail_[key3]->Fill(lp_track.x(), lp_x);
        residual_x_detail_[key3]->Fill(residualX);
        residual_x_cls_detail_[key3]->Fill(residualX, cls_size);
        residual_y_detail_[key3]->Fill(residualY);
      }
    }
  }
}

void TestBeamTrackAnalyzer::beginJob(){}
void TestBeamTrackAnalyzer::endJob(){}

void TestBeamTrackAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& iSetup){}
void TestBeamTrackAnalyzer::endRun(edm::Run const&, edm::EventSetup const&){}
                   
//define this as a plug-in
DEFINE_FWK_MODULE(TestBeamTrackAnalyzer);

#ifndef TestBeamTrackAnalyzer_H
#define TestBeamTrackAnalyzer_H
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
#include "TrackingTools/PatternTools/interface/Trajectory.h"
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
#include "DataFormats/TrajectorySeed/interface/TrajectorySeedCollection.h"
#include "DataFormats/TrackingRecHit/interface/KfComponentsHolder.h"

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
  edm::EDGetTokenT<vector<Trajectory>> trajs_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;

  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeom_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeomBeginRun_;

  TH1D* trackChi2_;

  std::map<int, TH2D*> track_occ_;
  std::map<int, TH2D*> rechit_occ_;
  std::map<int, TH2D*> rechit_pos_;
  std::map<int, TH1D*> residual_x_;
  std::map<int, TH1D*> residual_strip_;
  std::map<int, TH1D*> residual_eta_;
  std::map<int, TH2D*> residual_x_cls_;
  std::map<int, TH1D*> residual_y_;
  std::map<int, TH2D*> residual_;
  std::map<int, TH1D*> trackingError_x_;
  std::map<int, TH1D*> trackingError_y_;
  std::map<int, TH2D*> track_rechit_x_;
  std::map<int, TH2D*> track_rechit_y_;

  std::map<Key3, TH2D*> track_occ_detail_;
  std::map<Key3, TH2D*> rechit_occ_detail_;
  std::map<Key3, TH2D*> rechit_pos_detail_;
  std::map<Key3, TH1D*> residual_x_detail_;
  std::map<Key3, TH2D*> residual_detail_;
  std::map<Key3, TH2D*> residual_x_cls_detail_;
  std::map<Key3, TH1D*> residual_y_detail_;
  std::map<Key3, TH1D*> trackingError_x_detail_;
  std::map<Key3, TH1D*> trackingError_y_detail_;
  std::map<Key3, TH2D*> track_rechit_detail_;

};

#endif

TestBeamTrackAnalyzer::TestBeamTrackAnalyzer(const edm::ParameterSet& iConfig)
 : hGEMGeom_(esConsumes()),
   hGEMGeomBeginRun_(esConsumes<edm::Transition::BeginRun>())
{ 
  tracks_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"));
  trajs_ = consumes<vector<Trajectory>>(iConfig.getParameter<edm::InputTag>("trajs"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));
}

TestBeamTrackAnalyzer::~TestBeamTrackAnalyzer(){}

void
TestBeamTrackAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeom_);
//  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<vector<reco::Track>> tracks;
  iEvent.getByToken(tracks_, tracks);

  edm::Handle<vector<Trajectory>> trajs;
  iEvent.getByToken(trajs_, trajs);
  
  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_,gemRecHits);

  std::map<GEMDetId, TrajectoryStateOnSurface> tsosMap;
  for (std::vector<reco::Track>::const_iterator track = tracks->begin(); track != tracks->end(); ++track)
  {
    trackChi2_->Fill(track->normalizedChi2());
    auto traj = trajs->begin();
    for (auto trajMeas : traj->measurements()) {
      auto tsos = trajMeas.predictedState();
      auto tsos_error = tsos.cartesianError().position();
      auto rechit = trajMeas.recHit();
      auto gemId = GEMDetId(rechit->geographicalId());
      tsosMap[gemId] = tsos;

      int station = gemId.station();
      int chamber = gemId.chamber();
      int ieta = gemId.ieta();

      Key3 key3(station, chamber, ieta);

      trackingError_x_[station]->Fill(tsos_error.cxx()*1e+3);
      trackingError_y_[station]->Fill(tsos_error.czz()*1e+3);
      trackingError_x_detail_[key3]->Fill(tsos_error.cxx()*1e+3);
      trackingError_y_detail_[key3]->Fill(tsos_error.czz()*1e+3);
    }

    for (auto hit : track->recHits()) {
      auto etaPart = GEMGeometry_->etaPartition(hit->geographicalId());
      auto etaPartId = etaPart->id();

      if (tsosMap.find(etaPartId) == tsosMap.end()) continue;
      auto tsos = tsosMap[etaPartId];

      auto lp_track = tsos.localPosition();
      auto gp_track = tsos.globalPosition();

      int station = etaPartId.station();
      int chamber = etaPartId.chamber();
      int ieta = etaPartId.ieta();

      Key3 key3(station, chamber, ieta);
    
      track_occ_[station]->Fill(gp_track.x(), gp_track.z());
      track_occ_detail_[key3]->Fill(lp_track.x(), lp_track.y());

      bool hasHit = false;
      float maxR = 5000;
      int cls_size=0;
      auto lp_rechit = hit->localPosition();
      auto strip = etaPart->strip(lp_rechit);
      auto lp_error = hit->localPositionError();
      auto gp_rechit = etaPart->toGlobal(lp_rechit);
      auto strip_rechit = etaPart->strip(lp_rechit);
      auto residual_eta = ieta;
      for (auto testEta : GEMGeometry_->etaPartitions()) {
        if (abs(testEta->position().y() - gp_track.y()) > 0.0001) continue;
        else if (testEta->id().ieta() - ieta > 1) continue;

        auto range = gemRecHits->get(testEta->id());

        for (auto rechit = range.first; rechit != range.second; ++rechit) {
          if (rechit->clusterSize() > 6) continue;
          auto lp_rechit_ = rechit->localPosition();
          auto gp_rechit_ = testEta->toGlobal(lp_rechit);

          auto deltaR = (gp_rechit_ - gp_track).mag();
          if (deltaR < maxR) {
            hasHit = true;
            maxR = deltaR; 
            cls_size = rechit->clusterSize();
            lp_rechit = lp_rechit_;
            lp_error = rechit->localPositionError();
            gp_rechit = etaPart->toGlobal(lp_rechit);
            strip_rechit = testEta->strip(lp_rechit);
            residual_eta = ieta- testEta->id().ieta();
          }
        }
      }

      if (hasHit) {
        float residualX = (gp_rechit.x() - gp_track.x()) * 10.;
        float lp_residualX = (lp_rechit.x() - lp_track.x()) * 10;
        float residualStrip = strip_rechit - strip;
        float residualY = (gp_rechit.z() - gp_track.z()) * 10.;

        rechit_occ_[station]->Fill(gp_track.x(), gp_track.z());
        rechit_pos_[station]->Fill(gp_rechit.x(), gp_rechit.z());

        if (station != 0) track_rechit_x_[station]->Fill(gp_track.x(), gp_rechit.x());
        else track_rechit_x_[station]->Fill(gp_track.z(), gp_rechit.z());
        if (station != 0) track_rechit_y_[station]->Fill(gp_track.z(), gp_rechit.z());
        else track_rechit_x_[station]->Fill(gp_track.x(), gp_rechit.x());

        residual_x_[station]->Fill(residualX);
        residual_y_[station]->Fill(residualY);
        residual_strip_[station]->Fill(residualStrip);
        residual_eta_[station]->Fill(residual_eta);
        residual_[station]->Fill(residualX, residualY);

        rechit_occ_detail_[key3]->Fill(lp_track.x(), lp_track.y());
        rechit_pos_detail_[key3]->Fill(lp_rechit.x(), lp_rechit.y());
        track_rechit_detail_[key3]->Fill(gp_track.x(), gp_rechit.x());
        residual_x_detail_[key3]->Fill(lp_residualX);
        residual_detail_[key3]->Fill(residualX, residualY);
        residual_y_detail_[key3]->Fill(residualY);
        residual_x_cls_detail_[key3]->Fill(lp_residualX, cls_size);
      }
      else continue;
    }
  }
}

void TestBeamTrackAnalyzer::beginJob(){}
void TestBeamTrackAnalyzer::endJob(){}

void TestBeamTrackAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& iSetup) { 
  trackChi2_ = fs->make<TH1D>("track_chi2", "Normalized Track Chi2", 100, 0, 200);
  
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeomBeginRun_);
//  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  for (auto station : GEMGeometry_->stations()) {
    int st = station->station();
    track_occ_[st] = fs->make<TH2D>(Form("track_occ_GE%d", st),
                                    Form("Occupancy from Track GE%d", st),
                                    800, -10, 10,
                                    800, -10, 10);
    rechit_occ_[st] = fs->make<TH2D>(Form("rechit_occ_GE%d", st),
                                     Form("Occupancy from Matched RecHit : GE%d", st),
                                     800, -10, 10,
                                     800, -10, 10);
    rechit_pos_[st] = fs->make<TH2D>(Form("rechit_pos_GE%d", st),
                                     Form("Position from Matched RecHit : GE%d", st),
                                     800, -10, 10,
                                     800, -10, 10);
    track_rechit_x_[st] = fs->make<TH2D>(Form("track_rechit_occ_x_GE%d", st),
                                         Form("Occpancy from Track vs RecHit : GE%d", st),
                                         200, -10, 10,
                                         200, -10, 10);
    track_rechit_y_[st] = fs->make<TH2D>(Form("track_rechit_occ_y_GE%d", st),
                                         Form("Occpancy from Track vs RecHit : GE%d", st),
                                         200, -10, 10,
                                         200, -10, 10);
    residual_x_[st] = fs->make<TH1D>(Form("residual_x_GE%d", st),
                                     Form("residual X : GE%d", st),
                                     400, -20, 20);
    residual_strip_[st] = fs->make<TH1D>(Form("residual_strip_GE%d", st),
                                         Form("residual strip : GE%d", st),
                                         400, -200, 200);
    residual_eta_[st] = fs->make<TH1D>(Form("residual_eta_GE%d", st),
                                       Form("residual eta : GE%d", st),
                                       3, -1, 2);
    residual_x_cls_[st] = fs->make<TH2D>(Form("residual_x_cls_GE%d", st),
                                         Form("residual X : GE%d", st),
                                         400, -1, 1,
                                         10, 1, 11);
    residual_y_[st] = fs->make<TH1D>(Form("residual_y_GE%d", st),
                                     Form("residual Y : GE%d", st),
                                     10, -20, 20);
    residual_[st] = fs->make<TH2D>(Form("residual_GE%d", st),
                                   Form("residual X vs Y : GE%d", st),
                                   400, -20, 20,
                                   400, -100, 100);
    trackingError_x_[st] = fs->make<TH1D>(Form("trackingError_x_GE%d", st),
                                          Form("trackingError X : GE%d", st),
                                          1000, 0, 10);
    trackingError_y_[st] = fs->make<TH1D>(Form("trackingError_y_GE%d", st),
                                          Form("trackingError Y : GE%d", st),
                                          1000, 0, 10);
            
    for (auto superChamber : station->superChambers()) {
      for (auto chamber : superChamber->chambers()) {
        int ch = chamber->id().chamber();
        for (auto etaPart : chamber->etaPartitions()) {
          int ieta = etaPart->id().ieta();
          Key3 key3(st, ch, ieta);
          track_occ_detail_[key3]= fs->make<TH2D>(Form("track_occ_GE%d1_ch%d_ieta%d", st, ch, ieta), 
                                                  Form("Occupancy from Track : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                  800, -10, 10, 
                                                  800, -10, 10);
          rechit_occ_detail_[key3] = fs->make<TH2D>(Form("rechit_occ_GE%d1_ch%d_ieta%d", st, ch, ieta), 
                                                    Form("Occupancy from Matched RecHit : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                    800, -10, 10, 
                                                    800, -10, 10);
          rechit_pos_detail_[key3] = fs->make<TH2D>(Form("rechit_pos_GE%d1_ch%d_ieta%d", st, ch, ieta), 
                                                    Form("Position from Matched RecHit : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                    800, -10, 10, 
                                                    800, -10, 10);
          track_rechit_detail_[key3] = fs->make<TH2D>(Form("track_rechit_occ_GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                      Form("Occpancy from Track vs RecHit : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                      200, -10, 10,
                                                      200, -10, 10);
          residual_x_detail_[key3] = fs->make<TH1D>(Form("residual_x_GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                    Form("residual X : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                    400, -20, 20);
          residual_x_cls_detail_[key3] = fs->make<TH2D>(Form("residual_x_cls_GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                        Form("residual X : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                        400, -1, 1,
                                                        10, 1, 11);
          residual_detail_[key3] = fs->make<TH2D>(Form("residual_GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                  Form("residual X vs Y : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                  400, -20, 20,
                                                  400, -100, 100);
          residual_y_detail_[key3] = fs->make<TH1D>(Form("residual_y_GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                    Form("residual Y : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                    10, -20, 20);
          trackingError_x_detail_[key3] = fs->make<TH1D>(Form("trackingError_x_GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                         Form("trackingError X : GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                         1000, 0, 10);
          trackingError_y_detail_[key3] = fs->make<TH1D>(Form("trackingError_y_GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                         Form("trackingError Y : GE%d1_ch%d_ieta%d", st, ch, ieta),
                                                         1000, 0, 10);
        }
      }
    }
  }
}
void TestBeamTrackAnalyzer::endRun(edm::Run const&, edm::EventSetup const&){}
                   
//define this as a plug-in
DEFINE_FWK_MODULE(TestBeamTrackAnalyzer);

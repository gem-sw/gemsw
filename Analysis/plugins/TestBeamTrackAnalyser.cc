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

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
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
using namespace edm;

class TestBeamTrackAnalyzer : public edm::EDAnalyzer {
public:
  explicit TestBeamTrackAnalyzer(const edm::ParameterSet&);
  ~TestBeamTrackAnalyzer();

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void beginJob() override;
  virtual void endJob() override;

  virtual void beginRun(Run const&, EventSetup const&) override;
  virtual void endRun(Run const&, EventSetup const&) override;

  // ----------member data ---------------------------
  edm::Service<TFileService> fs;
  edm::EDGetTokenT<reco::TrackCollection> tracks_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;

  TH1D* trackChi2_;
  std::map<int, TH2D*> track_occ_;
  std::map<int, TH2D*> recHit_occ_;
  std::map<int, TH1D*> residualX_;
  std::map<int, TH1D*> residualY_;

  std::map<int, TH2D*> tracker_occ_;
  std::map<int, TH1D*> tracker_residual_x_;
  std::map<int, TH1D*> tracker_residual_y_;
};

TestBeamTrackAnalyzer::TestBeamTrackAnalyzer(const edm::ParameterSet& iConfig)
{ 
  tracks_ = consumes<reco::TrackCollection>(iConfig.getParameter<InputTag>("tracks"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));

  trackChi2_ = fs->make<TH1D>("track_chi2", "Normalized Track Chi2", 100, 0, 10);

  residualX_[0] = fs->make<TH1D>("residual_X_GE0",  "residual X : GE0", 100, -5, 5);
  residualY_[0] = fs->make<TH1D>("residual_Y_GE0",  "residual Y : GE0", 100, -10, 10);
  track_occ_[0] = fs->make<TH2D>("track_occ_GE0", "Occupancy from Track : GE0", 20, -10, 10, 20, -10, 10);
  recHit_occ_[0] = fs->make<TH2D>("recHit_occ_GE0", "Occupancy from Matched RecHit : GE0", 20, -10, 10, 20, -10, 10);

  residualX_[2] = fs->make<TH1D>("residual_X_GE21", "residual X : GE21", 100, -5, 5);
  residualY_[2] = fs->make<TH1D>("residual_Y_GE21", "residual Y : GE21", 100, -10, 10);
  track_occ_[2] = fs->make<TH2D>("track_occ_GE21", "Occupancy from Track : GE21", 20, -10, 10, 20, -10, 10);
  recHit_occ_[2] = fs->make<TH2D>("recHit_occ_GE21", "Occupancy from Matched RecHit : GE21", 20, -10, 10, 20, -10, 10);

  for (int i = 0; i < 4; i++) {
    tracker_occ_[i] = fs->make<TH2D>(Form("tracker_occ_ch_%d", i), Form("Occupancy from tracker chamber %d", i), 20, -10, 10, 20, -10, 10);
    tracker_residual_x_[i] = fs->make<TH1D>(Form("residual_X_tracker_%d",i), Form("residual X : tracking chamber %d", i), 100, -5, 5);
    tracker_residual_y_[i] = fs->make<TH1D>(Form("residual_Y_tracker_%d",i), Form("residual Y : tracking chamber %d", i), 100, -5, 5);
  }
}

TestBeamTrackAnalyzer::~TestBeamTrackAnalyzer()
{
}

void
TestBeamTrackAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom;
  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<vector<reco::Track>> tracks;
  iEvent.getByToken(tracks_, tracks);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_,gemRecHits);

  for (std::vector<reco::Track>::const_iterator track = tracks->begin(); track != tracks->end(); ++track)
  {
    trackChi2_->Fill(track->normalizedChi2());
    for (auto hit : track->recHits()) {
      auto etaPart = GEMGeometry_->etaPartition(hit->geographicalId());
      auto etaPartId = etaPart->id();

      auto lp_track = hit->localPosition();
      auto gp_track = etaPart->toGlobal(lp_track);

      auto range = gemRecHits->get(etaPartId);

      bool hasHit = false;
      float maxR = 500000;
      float residualX = 0;
      float residualY = 0;

      int station = etaPartId.station();
      int chamber = etaPartId.chamber();
      int ieta = etaPartId.ieta();

      int chamber_nu = (chamber / 2 - 1)*2 + (ieta-1)/2;
      if (station == 1) {
        if (ieta % 2 != 0) {
          for (auto hit2 : track->recHits()) {
            auto etaPart2 = GEMGeometry_->etaPartition(hit2->geographicalId());
            auto etaPartId2 = etaPart2->id();

            int station2 = etaPartId2.station();
            int chamber2 = etaPartId2.chamber();
            int ieta2 = etaPartId2.ieta();

            if (station2 != station or chamber2 != chamber) continue;
            if (ieta+1 != ieta2) continue;

            auto lp_track2 = hit2->localPosition();
            auto gp_track2 = etaPart2->toGlobal(lp_track2);
            tracker_occ_[chamber_nu]->Fill(gp_track.x(), gp_track2.z());
          }
        }
      } else {
        track_occ_[station]->Fill(gp_track.x(), gp_track.z());
      }

      const GEMStripTopology* top_(dynamic_cast<const GEMStripTopology*>(&(etaPart->topology())));
      const float stripLength(top_->stripLength());
      const float stripPitch(etaPart->pitch());


      for (auto rechit = range.first; rechit != range.second; ++rechit) {
        auto lp_rechit = rechit->localPosition();
        auto gp_rechit = etaPart->toGlobal(lp_rechit);

        //if (abs(lp_rechit.x() - lp_track.x()) > stripPitch*30) continue;
        //if (abs(lp_rechit.y() - lp_track.y()) > stripLength*2) continue;
        auto deltaR = (lp_rechit - lp_track).mag();
        if (deltaR < maxR) {
          hasHit = true;
          maxR = deltaR; 
          residualX = gp_rechit.x() - gp_track.x();
          residualY = gp_rechit.z() - gp_track.z();
        }
      }
      if (not hasHit) continue;
      if (station != 1) { 
        residualX_[station]->Fill(residualX);
        residualY_[station]->Fill(residualY);
        recHit_occ_[station]->Fill(gp_track.x(), gp_track.z());
      } else if (ieta % 2 == 1) {
        tracker_residual_x_[chamber_nu]->Fill(residualX);
      } else if (ieta % 2 == 0) {
        tracker_residual_y_[chamber_nu]->Fill(residualY);
      }
    }
  }
}

void TestBeamTrackAnalyzer::beginJob(){}
void TestBeamTrackAnalyzer::endJob(){}

void TestBeamTrackAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& iSetup){}
void TestBeamTrackAnalyzer::endRun(Run const&, EventSetup const&){}
                   
//define this as a plug-in
DEFINE_FWK_MODULE(TestBeamTrackAnalyzer);

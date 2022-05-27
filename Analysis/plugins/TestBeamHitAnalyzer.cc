#ifndef TestBeamHitAnalyzer_H
#define TestBeamHitAnalyzer_H
// cd /cms/ldap_home/iawatson/scratch/GEM/CMSSW_10_1_5/src/ && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// cd ../../.. && source /cvmfs/cms.cern.ch/cmsset_default.sh && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// system include files
#include <memory>
#include <cmath>
#include <iostream>
#include <map>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

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

class TestBeamHitAnalyzer : public edm::one::EDAnalyzer<edm::one::WatchRuns> {  
public:
  explicit TestBeamHitAnalyzer(const edm::ParameterSet&);
  ~TestBeamHitAnalyzer();

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void beginJob() override;
  virtual void endJob() override;

  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;

  // ----------member data ---------------------------
  edm::Service<TFileService> fs;
  edm::EDGetTokenT<GEMDigiCollection> gemDigis_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeom_; 
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeomBeginRun_; 

  std::map<int, TH2D*> digi_occ_;
  std::map<int, TH2D*> rechit_occ_;

  std::map<Key3, TH1D*> digi_occ_detail_;
  std::map<Key3, TH2D*> rechit_occ_detail_;
};

TestBeamHitAnalyzer::TestBeamHitAnalyzer(const edm::ParameterSet& iConfig)
  : hGEMGeom_(esConsumes()),
    hGEMGeomBeginRun_(esConsumes<edm::Transition::BeginRun>())
{ 
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));
  gemDigis_ = consumes<GEMDigiCollection>(iConfig.getParameter<edm::InputTag>("gemDigiLabel"));
//  hGEMGeomBegin_ = esConsumes<GEMGeometry, MuonGeometryRecord>(); 
//  hGEMGeom_ = esConsumes<GEMGeometry, MuonGeometryRecord>(); 
}

#endif

TestBeamHitAnalyzer::~TestBeamHitAnalyzer(){}

void
TestBeamHitAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom;
  hGEMGeom = iSetup.getHandle(hGEMGeom_);
//  iSetup.getByToken(hGEMGeom_, hGEMGeom);

//  edm::ESHandle<GEMGeometry> hGEMGeom;
//  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_,gemRecHits);

  edm::Handle<GEMDigiCollection> gemDigis;
  iEvent.getByToken(gemDigis_,gemDigis);
  
  for (auto etaPart : GEMGeometry_->etaPartitions()) {
    auto etaPartId = etaPart->id();

    int station = etaPartId.station();
    int chamber = etaPartId.chamber();
    int ieta = etaPartId.ieta();

    Key3 key3(station, chamber, ieta);

    auto range = gemRecHits->get(etaPartId);
    for (auto rechit = range.first; rechit != range.second; ++rechit) {
      auto lp_rechit = rechit->localPosition();
      auto gp_rechit = etaPart->toGlobal(lp_rechit);

      rechit_occ_[station]->Fill(gp_rechit.x(), gp_rechit.z());
      rechit_occ_detail_[key3]->Fill(gp_rechit.x(), gp_rechit.z());
    }
    auto digiRange = gemDigis->get(etaPartId); 
    for (auto digi = digiRange.first; digi != digiRange.second; ++digi) {
      auto strip = digi->strip();
      
      digi_occ_[station]->Fill(strip, ieta);
      digi_occ_detail_[key3]->Fill(strip);
    }
  }
}

void TestBeamHitAnalyzer::beginJob(){}
void TestBeamHitAnalyzer::endJob(){}

void TestBeamHitAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& iSetup) { 
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom;
  hGEMGeom = iSetup.getHandle(hGEMGeomBeginRun_);

//  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  for (auto station : GEMGeometry_->stations()) {
    int st = station->station();
    int nEta = st==2?16:8;
    digi_occ_[st] = fs->make<TH2D>(Form("digi_occ_GE%d", st),
                                   Form("Occupancy from Track GE%d", st),
                                   384, -0.5, 383.5,
                                   nEta, 0.5, nEta+0.5);
    rechit_occ_[st] = fs->make<TH2D>(Form("rechit_occ_GE%d", st),
                                     Form("Occupancy from Matched RecHit : GE%d", st),
                                     800, -10, 10,
                                     800, -10, 10);
    for (auto superChamber : station->superChambers()) {
      for (auto chamber : superChamber->chambers()) {
        int ch = chamber->id().chamber();
        for (auto etaPart : chamber->etaPartitions()) {
          int ieta = etaPart->id().ieta();
          Key3 key3(st, ch, ieta);
          digi_occ_detail_[key3]= fs->make<TH1D>(Form("digi_occ_GE%d1_ch%d_ieta%d", st, ch, ieta), 
                                                 Form("Occupancy from Track : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                 384, -0.5, 383.5);
          rechit_occ_detail_[key3] = fs->make<TH2D>(Form("rechit_occ_GE%d1_ch%d_ieta%d", st, ch, ieta), 
                                                    Form("Occupancy from Matched RecHit : GE%d1 chamber %d iEta%d", st, ch, ieta),
                                                    800, -10, 10, 
                                                    800, -10, 10);
        }
      }
    }
  }
}
void TestBeamHitAnalyzer::endRun(edm::Run const&, edm::EventSetup const&){}
                   
//define this as a plug-in
DEFINE_FWK_MODULE(TestBeamHitAnalyzer);

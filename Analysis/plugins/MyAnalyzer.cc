#ifndef MyAnalyzer_H
#define MyAnalyzer_H
// cd /cms/ldap_home/iawatson/scratch/GEM/CMSSW_10_1_5/src/ && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// cd ../../.. && source /cvmfs/cms.cern.ch/cmsset_default.sh && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// system include files
#include <memory>
#include <cmath>
#include <iostream>
#include <map>
#include <unistd.h>

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

typedef std::tuple<int, int> Key2;
typedef std::tuple<int, int, int> Key3;

struct MuonData {
  void init();
  TTree* book(TTree *t);
  //Muon Info//////////////////////////////////////////////////////
  //int recHitsSize;
  int strip;
  int ieta;
  int chamber;
  int bx;
  };

void MuonData::init() {
  //recHitsSize = 9999;
  strip = 9999;
  ieta = 0;
  chamber = 99999;
  bx = 99999;
};

TTree* MuonData::book(TTree *t){
  edm::Service< TFileService > fs;
  t = fs->make<TTree>("MyTree", "MyTree");
  //Muon Info//////////////////////////////////////////////////////
  //t->Branch("recHitsSize", &recHitsSize);
  t->Branch("strip", &strip);
  t->Branch("ieta", &ieta);
  t->Branch("chamber", &chamber);
  t->Branch("bx", &bx);
  return t;
};

class MyAnalyzer : public edm::one::EDAnalyzer<edm::one::WatchRuns> { 
public:
  explicit MyAnalyzer(const edm::ParameterSet&);
  ~MyAnalyzer();

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void beginJob() override;
  virtual void endJob() override;

  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  std::pair<int, int> getModuleVfat(int ieta, int strip);

  MuonData data_;
  TTree* MyTree;

  // ----------member data ---------------------------
  edm::Service<TFileService> fs;
  edm::EDGetTokenT<reco::TrackCollection> tracks_;
  edm::EDGetTokenT<vector<Trajectory>> trajs_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeom_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeomBeginRun_;

  TH1D* trackChi2_;

  std::map<int, TH2D*> track_ch_occ_;
  std::map<int, TH2D*> rechit_ch_occ_;
  std::map<Key2, TH2D*> track_occ_;
  std::map<Key2, TH2D*> rechit_occ_;
};

MyAnalyzer::MyAnalyzer(const edm::ParameterSet& iConfig)
  : hGEMGeom_(esConsumes()),
    hGEMGeomBeginRun_(esConsumes<edm::Transition::BeginRun>())
{ 
  tracks_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"));
  trajs_ = consumes<vector<Trajectory>>(iConfig.getParameter<edm::InputTag>("trajs"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));
  edm::ParameterSet serviceParameters = iConfig.getParameter<edm::ParameterSet>("ServiceParameters");
  MyTree = data_.book(MyTree);
}

#endif

MyAnalyzer::~MyAnalyzer(){}

void MyAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& iSetup) {
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeomBeginRun_);
//  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  trackChi2_ = fs->make<TH1D>("track_chi2", "Normalized Track Chi2", 100, 0, 10);

  for (auto chamber : GEMGeometry_->chambers()) {
    auto chamberId = chamber->id();
    auto ch = chamberId.chamber();
    auto ly = chamberId.layer();

    track_ch_occ_[ch] = fs->make<TH2D>(Form("track_occ_ch%d", ch),
                                       Form("track_occ_ch%d", ch),
                                       6, 0, 6,
                                       8, 1, 17);
    rechit_ch_occ_[ch] = fs->make<TH2D>(Form("rechit_occ_ch%d", ch),
                                        Form("rechit_occ_ch%d", ch),
                                        6, 0, 6,
                                        8, 1, 17);
    for (int idx_mod = 0; idx_mod < 4; idx_mod++) {
      int module = idx_mod + (2 -ly) * 4 + 1;
      Key2 key(ch, module);
      track_occ_[key] = fs->make<TH2D>(Form("track_occ_ch%d_module%d", ch, module),
                                       Form("track_occ_ch%d_module%d", ch, module),
                                       6, 0, 6,
                                       2, 0, 2);
      rechit_occ_[key] = fs->make<TH2D>(Form("rechit_occ_ch%d_module%d", ch, module),
                                        Form("rechit_occ_ch%d_module%d", ch, module),
                                        6, 0, 6,
                                        2, 0, 2);

    }
  }
}

void MyAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
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
  std::cout << "****** Event Loop ******" << std::endl;
  int counter = 0;
  int zero_strip = 0;
  for (std::vector<reco::Track>::const_iterator track = tracks->begin(); track != tracks->end(); ++track)
  {
    std::cout << "RecHitsSize: " << track->recHitsSize() << std::endl;
    std::cout << "track->innerPosition() = " << track->innerPosition() << std::endl;
    counter++;
    trackChi2_->Fill(track->normalizedChi2());
    auto traj = trajs->begin();
    for (auto trajMeas : traj->measurements()) {
      auto tsos = trajMeas.predictedState();
      auto rechit = trajMeas.recHit();
      auto gemId = GEMDetId(rechit->geographicalId());
      tsosMap[gemId] = tsos;
    }

    for (auto hit : track->recHits()) {
      auto etaPart = GEMGeometry_->etaPartition(hit->geographicalId());
      auto etaPartId = etaPart->id();

      if (tsosMap.find(etaPartId) == tsosMap.end()) continue;
      auto tsos = tsosMap[etaPartId];

      auto lp_track = tsos.localPosition();

      auto ch = etaPartId.chamber();
      auto ly = etaPartId.layer();
      auto ieta = etaPartId.ieta();

      auto strip = int(etaPart->strip(lp_track));
      int module = (16-ieta)/4 + 1 + (2-ly)*4;
      int sector = (16-ieta)%4 / 2;
     
      Key2 key(ch, module);

      track_occ_[key]->Fill(strip / 64, sector);
      track_ch_occ_[ch]->Fill(strip / 64, ieta);

      std::cout << "Before assigning values to strip: " << data_.strip << std::endl;

      data_.strip = strip;
      data_.ieta = ieta; 
      data_.chamber = ch;
      std::cout << "After assigning values to strip: " << data_.strip << std::endl;
      if (data_.strip == 0){ 
        zero_strip++;
        std::cout << "Another zero strip hit" << std::endl;
      }
      //MyTree->Fill();

      if (!hit->isValid()) continue;

      rechit_occ_[key]->Fill(strip / 64, sector);
      rechit_ch_occ_[ch]->Fill(strip / 64, ieta);
    }
    //MyTree->Fill();
    //edm::EventBase* Event_Base = new EventBase;
    int BX = iEvent.eventAuxiliary().bunchCrossing();
    data_.bx = BX;
    MyTree->Fill();
  }
  std::cout << "The number of times strip zero was hit is:" << zero_strip << std::endl;
  std::cout << "The number of tracks: " << counter << std::endl; 
}

std::pair<int, int> MyAnalyzer::getModuleVfat(int ieta, int strip) {
  ieta = 16 - ieta;
  int module = ieta / 4 + 1;
  int sector = (ieta % 4) / 2;
  int iphi = strip / 64;
  int vfat = iphi * 2 + sector;
  return std::make_pair(module, vfat);
}

void MyAnalyzer::beginJob(){}
void MyAnalyzer::endJob(){}

void MyAnalyzer::endRun(edm::Run const&, edm::EventSetup const&){}




//define this as a plug-in
DEFINE_FWK_MODULE(MyAnalyzer);

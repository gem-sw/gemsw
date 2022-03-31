#ifndef QC8TrackAnalyzer_H
#define QC8TrackAnalyzer_H
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

typedef std::tuple<int, int> Key2;
typedef std::tuple<int, int, int> Key3;

class QC8TrackAnalyzer : public edm::EDAnalyzer {
public:
  explicit QC8TrackAnalyzer(const edm::ParameterSet&);
  ~QC8TrackAnalyzer();

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void beginJob() override;
  virtual void endJob() override;

  virtual void beginRun(edm::Run const&, edm::EventSetup const&) override;
  virtual void endRun(edm::Run const&, edm::EventSetup const&) override;
  std::pair<int, int> getModuleVfat(int ieta, int strip);

  // ----------member data ---------------------------
  edm::Service<TFileService> fs;
  edm::EDGetTokenT<reco::TrackCollection> tracks_;
  edm::EDGetTokenT<vector<Trajectory>> trajs_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;

  TH1D* trackChi2_;

  std::map<Key2, TH2D*> track_occ_;
  std::map<Key2, TH2D*> rechit_occ_;
};

QC8TrackAnalyzer::QC8TrackAnalyzer(const edm::ParameterSet& iConfig)
{ 
  tracks_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"));
  trajs_ = consumes<vector<Trajectory>>(iConfig.getParameter<edm::InputTag>("trajs"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));
  edm::ParameterSet serviceParameters = iConfig.getParameter<edm::ParameterSet>("ServiceParameters");
}

#endif

QC8TrackAnalyzer::~QC8TrackAnalyzer(){}

void QC8TrackAnalyzer::beginRun(const edm::Run& run, const edm::EventSetup& iSetup) {
  edm::ESHandle<GEMGeometry> hGEMGeom;
  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  trackChi2_ = fs->make<TH1D>("track_chi2", "Normalized Track Chi2", 100, 0, 10);

  for (auto chamber : GEMGeometry_->chambers()) {
    auto chamberId = chamber->id();
    auto ch = chamberId.chamber();
    auto ly = chamberId.layer();

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

void QC8TrackAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom;
  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
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

      if (!hit->isValid()) continue;

      rechit_occ_[key]->Fill(strip / 64, sector);
    }
  }
}

std::pair<int, int> QC8TrackAnalyzer::getModuleVfat(int ieta, int strip) {
  ieta = 16 - ieta;
  int module = ieta / 4 + 1;
  int sector = (ieta % 4) / 2;
  int iphi = strip / 64;
  int vfat = iphi * 2 + sector;
  return std::make_pair(module, vfat);
}

void QC8TrackAnalyzer::beginJob(){}
void QC8TrackAnalyzer::endJob(){}

void QC8TrackAnalyzer::endRun(edm::Run const&, edm::EventSetup const&){}
                   
//define this as a plug-in
DEFINE_FWK_MODULE(QC8TrackAnalyzer);

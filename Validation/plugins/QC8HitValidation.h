#ifndef QC8HitValidation_H
#define QC8HitValidation_H
// cd /cms/ldap_home/iawatson/scratch/GEM/CMSSW_10_1_5/src/ && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// cd ../../.. && source /cvmfs/cms.cern.ch/cmsset_default.sh && eval `scramv1 runtime -sh` && eval `scramv1 runtime -sh` && scram b -j 10
// system include files
#include <memory>
#include <cmath>
#include <iostream>
#include <map>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"
// GEM
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/GEMRecHit/interface/GEMSegmentCollection.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartition.h"
#include "Geometry/GEMGeometry/interface/GEMEtaPartitionSpecs.h"
#include "Geometry/CommonTopologies/interface/GEMStripTopology.h"

#include "SimDataFormats/TrackingHit/interface/PSimHitContainer.h"

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

class QC8HitValidation : public DQMEDAnalyzer {
public:
  explicit QC8HitValidation(const edm::ParameterSet&);
  ~QC8HitValidation();

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);
  virtual void bookHistograms(DQMStore::IBooker&, edm::Run const&, edm::EventSetup const&) override;

  void setBinLabelAndTitle(TH2F* hist, int module = 0);
  bool matchRecHitAgainstSimHit(GEMRecHitCollection::const_iterator rechit, int simhit_strip);

  std::pair<int, int> getModuleVfat(int ieta, int strip);

  // ----------member data ---------------------------
  edm::EDGetTokenT<edm::PSimHitContainer> gemSimHits_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeom_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeomBeginRun_;

  dqm::impl::MonitorElement* trackChi2_;

  std::map<int, dqm::impl::MonitorElement*> simhit_ch_occ_;
  std::map<int, dqm::impl::MonitorElement*> rechit_ch_occ_;
  std::map<Key2, dqm::impl::MonitorElement*> simhit_occ_;
  std::map<Key2, dqm::impl::MonitorElement*> rechit_occ_;
};

#endif

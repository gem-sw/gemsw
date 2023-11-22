#ifndef DQM_GEM_INTERFACE_QC8RecHitSource_h
#define DQM_GEM_INTERFACE_QC8RecHitSource_h

#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "DQMServices/Core/interface/DQMEDAnalyzer.h"
#include "DQMServices/Core/interface/DQMStore.h"
#include "DQMServices/Core/interface/MonitorElement.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"

#include <string>

//----------------------------------------------------------------------------------------------------

class QC8RecHitSource : public DQMEDAnalyzer {
public:
  explicit QC8RecHitSource(const edm::ParameterSet &cfg);
  ~QC8RecHitSource() override{};
  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

protected:
  void dqmBeginRun(edm::Run const &, edm::EventSetup const &) override{};
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(edm::Event const &e, edm::EventSetup const &eSetup) override;

private:
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;
  edm::EDGetTokenT<GEMDigiCollection> gemDigis_;

  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeom_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeomBeginRun_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiOcc_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapRecHitOcc_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapRecHitClsSize_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc1hrEta1_4_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc1hrEta5_8_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc1hrEta9_12_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc1hrEta13_16_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc5hrEta1_4_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc5hrEta5_8_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc5hrEta9_12_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc5hrEta13_16_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc10hrEta1_4_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc10hrEta5_8_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc10hrEta9_12_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc10hrEta13_16_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc24hrEta1_4_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc24hrEta5_8_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc24hrEta9_12_;
  std::map<unsigned int, dqm::impl::MonitorElement*> mapDigiStripOcc24hrEta13_16_;
  int maxClsSizeToShow_ = 10;
};

#endif  // DQM_GEM_INTERFACE_QC8RecHitSource_h

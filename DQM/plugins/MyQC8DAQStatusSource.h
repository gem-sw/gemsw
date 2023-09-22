#ifndef DQM_GEM_INTERFACE_QC8DAQStatusSource_h
#define DQM_GEM_INTERFACE_QC8DAQStatusSource_h

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

#include "CondFormats/DataRecord/interface/GEMChMapRcd.h"
#include "CondFormats/GEMObjects/interface/GEMChMap.h"
#include "DataFormats/GEMDigi/interface/GEMDigiCollection.h"
#include "DataFormats/GEMDigi/interface/GEMVFATStatusCollection.h"
#include "DataFormats/GEMDigi/interface/GEMOHStatusCollection.h"
#include "DataFormats/GEMDigi/interface/GEMAMCStatusCollection.h"

#include <string>

//----------------------------------------------------------------------------------------------------

typedef std::tuple<int, int> Key2;
typedef std::tuple<int, int, int> Key3;

class MyQC8DAQStatusSource : public DQMEDAnalyzer {
public:
  enum DAQStatus { Good = 3, Warning = 2, Error = 1, None = 0 };
  explicit MyQC8DAQStatusSource(const edm::ParameterSet &cfg);
  ~MyQC8DAQStatusSource() override{};
  static void fillDescriptions(edm::ConfigurationDescriptions &descriptions);

protected:
  void dqmBeginRun(edm::Run const &, edm::EventSetup const &) override{};
  void bookHistograms(DQMStore::IBooker &, edm::Run const &, edm::EventSetup const &) override;
  void analyze(edm::Event const &e, edm::EventSetup const &eSetup) override;

private:
  void SetLabelAMCStatus(dqm::impl::MonitorElement *h2Status);
  void SetLabelOHStatus(dqm::impl::MonitorElement *h2Status);
  void SetLabelVFATStatus(dqm::impl::MonitorElement *h2Status);

  void fillSummary(int,
                   dqm::impl::MonitorElement*, 
                   dqm::impl::MonitorElement*, 
                   dqm::impl::MonitorElement*, 
                   dqm::impl::MonitorElement*); 

  const edm::ESGetToken<GEMChMap, GEMChMapRcd> gemChMapToken_;

  edm::EDGetToken tagVFAT_;
  edm::EDGetToken tagOH_;
  edm::EDGetToken tagAMC_;

  std::map<GEMChMap::chamDC, GEMChMap::chamEC> mapDCtoEC_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapAMCStatus_;
  std::map<Key2, dqm::impl::MonitorElement*> mapOHStatus_;
  std::map<Key3, dqm::impl::MonitorElement*> mapVFATStatus_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapAMCExists_;
  std::map<Key2, dqm::impl::MonitorElement*> mapOHExists_;
  std::map<Key3, dqm::impl::MonitorElement*> mapVFATExists_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapAMCErrors_;
  std::map<Key2, dqm::impl::MonitorElement*> mapOHErrors_;
  std::map<Key3, dqm::impl::MonitorElement*> mapVFATErrors_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapAMCWarnings_;
  std::map<Key2, dqm::impl::MonitorElement*> mapOHWarnings_;
  std::map<Key3, dqm::impl::MonitorElement*> mapVFATWarnings_;

  std::map<unsigned int, dqm::impl::MonitorElement*> mapAMCSummary_;
  std::map<Key2, dqm::impl::MonitorElement*> mapOHSummary_;
  std::map<Key3, dqm::impl::MonitorElement*> mapVFATSummary_;
  
  int nMAXEvents_ = 6000000;
  int nEventsPerBin_ = 100;
  int nAMCSlots_;
  int nOHSlots_;
  int nVFATSlots_;

  int nBitAMC_ = 12;
  int nBitOH_ = 17;
  int nBitVFAT_ = 7;
};

#endif  // DQM_GEM_INTERFACE_QC8DAQStatusSource_h

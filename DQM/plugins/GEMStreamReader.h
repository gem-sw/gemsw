#ifndef gemsw_DQM_GEMStreamReader_h
#define gemsw_DQM_GEMStreamReader_h

// Based
// DQMServices_StreamerIO_DQMStreamerReader
// and
// gemsw_EventFilter_GEMStreamSource

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/InputSourceMacros.h"

#include "FWCore/Sources/interface/ProducerSourceBase.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/Provenance/interface/Timestamp.h"

#include "RawEntryIterator.h"

#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>

class GEMStreamReader : public edm::ProducerSourceBase {
public:
  // construction/destruction
  GEMStreamReader(edm::ParameterSet const& pset, edm::InputSourceDescription const& desc);
  ~GEMStreamReader() override{};

  static void fillDescription(edm::ParameterSetDescription& desc);

private:
  // member functions
  bool setRunAndEventInfo(edm::EventID& id,
                          edm::TimeValue_t& theTime,
                          edm::EventAuxiliary::ExperimentType& eType) override;
  void produce(edm::Event& e) override;

  bool openFile(const std::string& fileName, std::ifstream& fin);
  std::unique_ptr<FRDEventMsgView> getEventMsg(std::ifstream& fin);
  std::vector<uint64_t>* makeFEDRAW(FRDEventMsgView* frdEventMsg, uint16_t fedId);

  void init();
  bool prepareNextFile();
  void openNextFile();
  void closeFile();
  std::unique_ptr<FRDEventMsgView> prepareNextEvent();

  // member data
  RawEntryIterator fIterator_;
  bool hasSecFile_;
  std::ifstream fin_;
  std::ifstream fin2_;
  GEMRawToDigi gemR2D;
  std::unique_ptr<FEDRawDataCollection> rawData_;
  std::vector<char> buffer_;
  int fedId_;
  int fedId2_;
  uint16_t detectedFRDversion_ = 0;
  uint16_t flags_ = 0;
  
  bool flagEndOfRunKills_;
  unsigned int procEventsFile_ = 0;
  unsigned int minEventsFile_;
};

#endif  // gemsw_DQM_GEMStreamReader_h

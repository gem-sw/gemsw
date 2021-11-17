#ifndef gemsw_EventFilter_GEMStreamSource_h
#define gemsw_EventFilter_GEMStreamSource_h

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/LuminosityBlock.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/InputSourceMacros.h"

#include "FWCore/Sources/interface/ProducerSourceFromFiles.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/Provenance/interface/Timestamp.h"

#include <unistd.h>
#include <string>
#include <vector>
#include <fstream>

class GEMStreamSource : public edm::ProducerSourceFromFiles {
public:
  // construction/destruction
  GEMStreamSource(edm::ParameterSet const& pset, edm::InputSourceDescription const& desc);
  ~GEMStreamSource() override{};

private:
  // member functions
  bool setRunAndEventInfo(edm::EventID& id,
                          edm::TimeValue_t& theTime,
                          edm::EventAuxiliary::ExperimentType& eType) override;
  void produce(edm::Event& e) override;

  bool openFile(const std::string& fileName);

private:
  // member data
  std::vector<std::string>::const_iterator itFileName_;
  std::vector<std::string>::const_iterator endFileName_;
  std::ifstream fin_;
  std::unique_ptr<FEDRawDataCollection> rawData_;
  std::vector<char> buffer_;
  const bool verifyAdler32_;
  const bool verifyChecksum_;
  const bool useL1EventID_;
  uint16_t detectedFRDversion_ = 0;
  uint16_t flags_ = 0;
};

#endif  // EventFilter_Utilities_GEMStreamSource_h

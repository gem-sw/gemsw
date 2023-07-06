/** \class GEMMappingTestor
 *  \unpacker for gem
 *  \based on CSCDigiToRawModule
 *  \author J. Lee - UoS
 */

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"
#include "EventFilter/GEMRawToDigi/interface/GEMRawToDigi.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/EventSetup.h"
#include "FWCore/Framework/interface/Run.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"
#include "FWCore/ParameterSet/interface/ConfigurationDescriptions.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ParameterSet/interface/ParameterSetDescription.h"
#include "FWCore/Utilities/interface/ESGetToken.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/Utilities/interface/Transition.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include <iostream>

class GEMMappingTestor : public edm::one::EDAnalyzer<> {
public:
  /// Constructor
  GEMMappingTestor(const edm::ParameterSet& pset);

  // Fill parameters descriptions
  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void analyze(const edm::Event&, const edm::EventSetup&);

  edm::EDGetTokenT<FEDRawDataCollection> fed_token;
  unsigned int fedIdStart_, fedIdEnd_;
  std::unique_ptr<GEMRawToDigi> gemRawToDigi_;
};

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GEMMappingTestor);

GEMMappingTestor::GEMMappingTestor(const edm::ParameterSet& pset)
    : fed_token(consumes<FEDRawDataCollection>(pset.getParameter<edm::InputTag>("InputLabel"))),
      fedIdStart_(pset.getParameter<unsigned int>("fedIdStart")),
      fedIdEnd_(pset.getParameter<unsigned int>("fedIdEnd")),
      gemRawToDigi_(std::make_unique<GEMRawToDigi>()) {
}

void GEMMappingTestor::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("InputLabel", edm::InputTag("rawDataCollector"));
  desc.add<unsigned int>("fedIdStart", FEDNumbering::MINGEMFEDID);
  desc.add<unsigned int>("fedIdEnd", FEDNumbering::MAXGEMFEDID);
}

void GEMMappingTestor::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  // Take raw from the event
  edm::Handle<FEDRawDataCollection> fed_buffers;
  iEvent.getByToken(fed_token, fed_buffers);

  for (unsigned int fedId = fedIdStart_; fedId <= fedIdEnd_; ++fedId) {
    const FEDRawData& fedData = fed_buffers->FEDData(fedId);

    int nWords = fedData.size() / sizeof(uint64_t);
    if (nWords == 0) continue;

    const uint64_t* word = reinterpret_cast<const uint64_t*>(fedData.data());
    auto amc13 = gemRawToDigi_->convertWordToGEMAMC13(word);

    // Read AMC data
    for (const auto& amc : *(amc13->getAMCpayloads())) {
      uint8_t amcNum = amc.amcNum();

      // Read GEB data
      for (const auto& optoHybrid : *amc.gebs()) {
        uint8_t gebId = optoHybrid.inputID();
        std::cout << Form("fedId %d amcNum %d oh %d", fedId, amcNum, gebId) << std::endl;

      }  // end of optohybrid loop

    }  // end of amc loop

  }  // end of amc13
}

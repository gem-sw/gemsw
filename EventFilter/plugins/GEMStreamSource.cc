#include <iostream>
#include <memory>
#include <zlib.h>

#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "IOPool/Streamer/interface/FRDEventMessage.h"
#include "IOPool/Streamer/interface/FRDFileHeader.h"

#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "DataFormats/FEDRawData/interface/FEDNumbering.h"
#include "DataFormats/FEDRawData/interface/FEDHeader.h"
#include "DataFormats/FEDRawData/interface/FEDTrailer.h"
#include "DataFormats/GEMDigi/interface/GEMAMC13.h"
#include "DataFormats/GEMDigi/interface/GEMAMC.h"

#include "DataFormats/TCDS/interface/TCDSRaw.h"

#include "EventFilter/Utilities/interface/GlobalEventNumber.h"
#include "EventFilter/GEMRawToDigi/interface/GEMRawToDigi.h"

#include "GEMStreamSource.h"
#include "EventFilter/Utilities/interface/crc32c.h"

using namespace std;

GEMStreamSource::GEMStreamSource(edm::ParameterSet const& pset, edm::InputSourceDescription const& desc)
    : ProducerSourceFromFiles(pset, desc, true),
      fedId_(pset.getUntrackedParameter<int>("fedId", 1477)),
      fedId2_(pset.getUntrackedParameter<int>("fedId2", 1478)) {
  openFile(fileNames(0)[0], fin_);
  hasSecFile = false;
  if (fileNames(0).size() == 2) {
    hasSecFile = true;
    openFile(fileNames(0)[1], fin2_);
  }
  produces<FEDRawDataCollection>();
}

bool GEMStreamSource::setRunAndEventInfo(edm::EventID& id,
                                         edm::TimeValue_t& theTime,
                                         edm::EventAuxiliary::ExperimentType& eType) {
  rawData_ = std::make_unique<FEDRawDataCollection>();

  std::unique_ptr<FRDEventMsgView> frdEventMsg = getEventMsg(fin_);
  if (!frdEventMsg) return false;

  //id = edm::EventID(frdEventMsg->run(), frdEventMsg->lumi(), frdEventMsg->event());
  // should be set up frdEventMsg, but run is wrong and lumi is not set for DB emap
  id = edm::EventID(id.run(), id.luminosityBlock(), frdEventMsg->event());

  std::vector<uint64_t>* words = makeFEDRAW(frdEventMsg.get(), fedId_);
  size_t fedSize = words->size() * sizeof(uint64_t);
  GEMAMC13::CDFHeader cdfh;
  cdfh.word = uint64_t(words->at(0));
  FEDRawData& fedData = rawData_->FEDData(GEMAMC13::CDFHeader{cdfh}.sourceId);
  fedData.resize(fedSize);
  memcpy(fedData.data(), words->data(), fedSize);

  if (hasSecFile) {
    std::unique_ptr<FRDEventMsgView> frdEventMsg2 = getEventMsg(fin2_);
    if (!frdEventMsg2) return false;

    if (frdEventMsg->event() != frdEventMsg2->event()) {
      cout << " event number does not match between files " << frdEventMsg->event() << " " << frdEventMsg2->event()
           << endl;
      return false;
    }

    std::vector<uint64_t>* words2 = makeFEDRAW(frdEventMsg2.get(), fedId2_);
    size_t fedSize2 = words2->size() * sizeof(uint64_t);
    GEMAMC13::CDFHeader cdfh2;
    cdfh2.word = uint64_t(words2->at(0));
    FEDRawData& fedData2 = rawData_->FEDData(GEMAMC13::CDFHeader{cdfh2}.sourceId);
    fedData2.resize(fedSize2);
    memcpy(fedData2.data(), words2->data(), fedSize2);
  }
  return true;
}

void GEMStreamSource::produce(edm::Event& e) { e.put(std::move(rawData_)); }

bool GEMStreamSource::openFile(const std::string& fileName, std::ifstream& fin) {
  std::cout << " open file.. " << fileName << std::endl;
  fin.close();
  fin.clear();
  size_t pos = fileName.find(':');
  if (pos != std::string::npos) {
    std::string prefix = fileName.substr(0, pos);
    if (prefix != "file")
      return false;
    pos++;
  } else
    pos = 0;

  fin.open(fileName.substr(pos).c_str(), std::ios::in | std::ios::binary);
  return fin.is_open();
}

std::unique_ptr<FRDEventMsgView> GEMStreamSource::getEventMsg(std::ifstream& fin) {
  if (fin.peek() == EOF) return NULL;
  
  //look for FRD header at beginning of the file and skip it
  if (fin.tellg() == 0) {
    constexpr size_t buf_sz = sizeof(FRDFileHeader_v1);  //try to read v1 FRD header size
    FRDFileHeader_v1 fileHead;
    fin.read((char*)&fileHead, buf_sz);

    if (fin.gcount() == 0)
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo")
          << "Unable to read file or empty file";
    else if (fin.gcount() < (ssize_t)buf_sz) {
      fin.seekg(0);
    } else {
      uint16_t frd_version = getFRDFileHeaderVersion(fileHead.id_, fileHead.version_);
      if (frd_version >= 1) {
        if (fileHead.headerSize_ < buf_sz)
          throw cms::Exception("GEMStreamSource::setRunAndEventInfo")
              << "Invalid FRD file header (size mismatch) in file ";
        else if (fileHead.headerSize_ > buf_sz)
          fin.seekg(fileHead.headerSize_, fin.beg);
      } else
        fin.seekg(0, fin.beg);
    }
  }

  if (detectedFRDversion_ == 0) {
    fin.read((char*)&detectedFRDversion_, sizeof(uint16_t));
    fin.read((char*)&flags_, sizeof(uint16_t));
    assert(detectedFRDversion_ > 0 && detectedFRDversion_ <= FRDHeaderMaxVersion);
    if (buffer_.size() < FRDHeaderVersionSize[detectedFRDversion_])
      buffer_.resize(FRDHeaderVersionSize[detectedFRDversion_]);
    *((uint32_t*)(&buffer_[0])) = detectedFRDversion_;
    fin.read(&buffer_[0] + sizeof(uint32_t), FRDHeaderVersionSize[detectedFRDversion_] - sizeof(uint32_t));
    assert(fin.gcount() == FRDHeaderVersionSize[detectedFRDversion_] - (unsigned int)(sizeof(uint32_t)));
  } else {
    if (buffer_.size() < FRDHeaderVersionSize[detectedFRDversion_])
      buffer_.resize(FRDHeaderVersionSize[detectedFRDversion_]);
    fin.read(&buffer_[0], FRDHeaderVersionSize[detectedFRDversion_]);
    assert(fin.gcount() == FRDHeaderVersionSize[detectedFRDversion_]);
  }

  std::unique_ptr<FRDEventMsgView> frdEventMsg(new FRDEventMsgView(&buffer_[0]));

  const uint32_t totalSize = frdEventMsg->size();
  if (totalSize > buffer_.size()) {
    buffer_.resize(totalSize);
  }
  if (totalSize > FRDHeaderVersionSize[detectedFRDversion_]) {
    fin.read(&buffer_[0] + FRDHeaderVersionSize[detectedFRDversion_],
             totalSize - FRDHeaderVersionSize[detectedFRDversion_]);
    if (fin.gcount() != totalSize - FRDHeaderVersionSize[detectedFRDversion_]) {
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo") << "premature end of file ";
    }
    frdEventMsg = std::make_unique<FRDEventMsgView>(&buffer_[0]);
  }

  LogDebug("GEMStreamSource") << "frdEventMsg run=" << frdEventMsg->run() << " lumi=" << frdEventMsg->lumi()
                              << " event=" << frdEventMsg->event() << " size=" << int(frdEventMsg->size())
                              << " eventSize=" << int(frdEventMsg->eventSize());

  return frdEventMsg;
}

std::vector<uint64_t>* GEMStreamSource::makeFEDRAW(FRDEventMsgView* frdEventMsg, uint16_t fedId) {
  uint64_t* event = (uint64_t*)frdEventMsg->payload();
  GEMAMC amc;
  amc.setAMCheader1(*(event));
  amc.setAMCheader2(*(event + 1));
  uint32_t eventSize = frdEventMsg->eventSize()/8;
  uint16_t BX_id = amc.bunchCrossing();
  uint32_t LV1_id = amc.lv1Id();
  uint32_t OrN = amc.orbitNumber();
  GEMAMC13 amc13;
  if (amc.formatVer() != 0) amc13.setCDFHeader(0x1, LV1_id, BX_id, amc.softSrcId());
  else amc13.setCDFHeader(0x1, LV1_id, BX_id, fedId);
  amc13.setAMC13Header(0, 1, OrN);
  amc13.setAMC13Trailer(BX_id, LV1_id, BX_id);
  uint32_t EvtLength = eventSize + 5;  // 2 header + 2 trailer + 1 AMC header
  amc13.setCDFTrailer(EvtLength);

  vector<uint64_t>* words = new vector<uint64_t>();
  words->push_back(amc13.getCDFHeader());
  words->push_back(amc13.getAMC13Header());
  words->push_back(amc13.getAMC13Header());// this is for AMCheader
  for (uint32_t i = 0; i < eventSize; i++) {
    words->push_back(*(event+i));
  }
  words->push_back(amc13.getAMC13Trailer());
  words->push_back(amc13.getCDFTrailer());

  LogDebug("GEMStreamSource") << "amc lv1Id=" << amc.lv1Id() << " orbitNumber=" << amc.orbitNumber()
                              << " bx=" << amc.bunchCrossing() << " amcNum=" << int(amc.amcNum())
                              << " words=" << words->size();

  return words;
}
DEFINE_FWK_INPUT_SOURCE(GEMStreamSource);

#include <iostream>
#include <memory>
#include <zlib.h>

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
      verifyAdler32_(pset.getUntrackedParameter<bool>("verifyAdler32", true)),
      verifyChecksum_(pset.getUntrackedParameter<bool>("verifyChecksum", true)),
      useL1EventID_(pset.getUntrackedParameter<bool>("useL1EventID", false)),
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

  if (fin_.peek() == EOF) return false;

  rawData_ = std::make_unique<FEDRawDataCollection>();

  std::unique_ptr<FRDEventMsgView> frdEventMsg = getEventMsg(fin_);
  if (useL1EventID_)
    id = edm::EventID(frdEventMsg->run(), frdEventMsg->lumi(), frdEventMsg->event());

  std::vector<uint64_t> words = makeFEDRAW(frdEventMsg.get(), fedId_);
  size_t fedSize = words.size() * sizeof(uint64_t);
  FEDRawData& fedData = rawData_->FEDData(fedId_);
  fedData.resize(fedSize);

  //cout << "words.size() " << words.size() << " fedData.size() " << fedData.size() << endl;

  memcpy(fedData.data(), words.data(), fedSize);

  // uint32_t fedSize = words.size() * sizeof(uint64_t);
  // fedData.resize(fedSize * 8);
  // memcpy(fedData.data(), words.data(), fedSize);

  if (hasSecFile) {
    if (fin2_.peek() == EOF) return false;
    std::unique_ptr<FRDEventMsgView> frdEventMsg2 = getEventMsg(fin2_);

    if (frdEventMsg->event() != frdEventMsg2->event()) {
      cout << " event number does not match between files " << frdEventMsg->event() << " " << frdEventMsg2->event()
           << endl;
      return false;
    }

    std::vector<uint64_t> words2 = makeFEDRAW(frdEventMsg2.get(), fedId2_);
    size_t fedSize2 = words2.size() * sizeof(uint64_t);
    FEDRawData& fedData2 = rawData_->FEDData(fedId2_);
    fedData2.resize(fedSize2);
    //cout << "words2.size() " << words2.size() << endl;
    memcpy(fedData2.data(), words2.data(), fedSize2);
  }
  return true;
}

void GEMStreamSource::produce(edm::Event& e) { e.put(std::move(rawData_)); }

std::ifstream GEMStreamSource::openFile(const std::string& fileName) {
  std::cout << " open file.. " << fileName << std::endl;
  std::ifstream fin;
  fin.close();
  fin.clear();
  size_t pos = fileName.find(':');
  if (pos != std::string::npos) {
    std::string prefix = fileName.substr(0, pos);
    if (prefix != "file")
      return fin;
    pos++;
  } else
    pos = 0;

  fin.open(fileName.substr(pos).c_str(), std::ios::in | std::ios::binary);
  return fin;
}

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
  if (fin.peek() == EOF) {
    if (++itFileName_ == endFileName_) {
      fin.close();
      return NULL;
    }
    if (!openFile(*itFileName_)) {
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo") << "could not open file " << *itFileName_;
    }
  }
  //look for FRD header at beginning of the file and skip it
  if (fin.tellg() == 0) {
    constexpr size_t buf_sz = sizeof(FRDFileHeader_v1);  //try to read v1 FRD header size
    FRDFileHeader_v1 fileHead;
    fin.read((char*)&fileHead, buf_sz);

    if (fin.gcount() == 0)
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo")
          << "Unable to read file or empty file" << *itFileName_;
    else if (fin.gcount() < (ssize_t)buf_sz) {
      fin.seekg(0);
    } else {
      uint16_t frd_version = getFRDFileHeaderVersion(fileHead.id_, fileHead.version_);
      if (frd_version >= 1) {
        if (fileHead.headerSize_ < buf_sz)
          throw cms::Exception("GEMStreamSource::setRunAndEventInfo")
              << "Invalid FRD file header (size mismatch) in file " << *itFileName_;
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
  // more debugging
  // cout << "frdEventMsg->run() " << frdEventMsg->run() << endl;
  // cout << "frdEventMsg->lumi() " << frdEventMsg->lumi() << endl;
  // cout << "frdEventMsg->event() " << frdEventMsg->event() << endl;
  // cout << "frdEventMsg->size() " << frdEventMsg->size() << endl;
  // cout << "frdEventMsg->flags() " << frdEventMsg->flags() << endl;
  // cout << "frdEventMsg->eventSize() " << frdEventMsg->eventSize() << endl;
  // cout << "frdEventMsg->paddingSize() " << frdEventMsg->paddingSize() << endl;
  // cout << "buffer_.size() " << buffer_.size() << endl;
  // cout << "FRDHeaderVersionSize[detectedFRDversion_] " << FRDHeaderVersionSize[detectedFRDversion_] << endl;

  const uint32_t totalSize = frdEventMsg->size();
  if (totalSize > buffer_.size()) {
    buffer_.resize(totalSize);
  }
  if (totalSize > FRDHeaderVersionSize[detectedFRDversion_]) {
    fin.read(&buffer_[0] + FRDHeaderVersionSize[detectedFRDversion_],
             totalSize - FRDHeaderVersionSize[detectedFRDversion_]);
    if (fin.gcount() != totalSize - FRDHeaderVersionSize[detectedFRDversion_]) {
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo") << "premature end of file " << *itFileName_;
    }
    frdEventMsg = std::make_unique<FRDEventMsgView>(&buffer_[0]);
  }

  if (verifyChecksum_ && frdEventMsg->version() >= 5) {
    uint32_t crc = 0;
    crc = crc32c(crc, (const unsigned char*)frdEventMsg->payload(), frdEventMsg->eventSize());
    if (crc != frdEventMsg->crc32c()) {
      throw cms::Exception("GEMStreamSource::getNextEvent") << "Found a wrong crc32c checksum: expected 0x" << std::hex
                                                            << frdEventMsg->crc32c() << " but calculated 0x" << crc;
    }
  } else if (verifyAdler32_ && frdEventMsg->version() >= 3) {
    uint32_t adler = adler32(0L, Z_NULL, 0);
    adler = adler32(adler, (Bytef*)frdEventMsg->payload(), frdEventMsg->eventSize());

    if (adler != frdEventMsg->adler32()) {
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo")
          << "Found a wrong Adler32 checksum: expected 0x" << std::hex << frdEventMsg->adler32() << " but calculated 0x"
          << adler;
    }
  }
  return frdEventMsg;
}

std::vector<uint64_t> GEMStreamSource::makeFEDRAW(FRDEventMsgView* frdEventMsg, uint16_t fedId) {
  uint32_t eventSize = frdEventMsg->eventSize();
  uint16_t BX_id = 0;
  uint32_t LV1_id = frdEventMsg->event();
  uint32_t OrN = frdEventMsg->event();
  GEMAMC13 amc13;
  amc13.setCDFHeader(0, LV1_id, BX_id, fedId);
  amc13.setAMC13Header(0, 1, OrN);
  amc13.setAMC13Trailer(BX_id, LV1_id, BX_id);
  uint32_t EvtLength = eventSize / 8 + 5;  // 2 header + 2 trailer + 1 AMC header
  amc13.setCDFTrailer(EvtLength);

  std::vector<uint64_t> words(EvtLength);
  words[0] = amc13.getCDFHeader();
  words[1] = amc13.getAMC13Header();
  words[2] = amc13.getAMC13Header();  // this is for AMCheader
  uint64_t* event = (uint64_t*)frdEventMsg->payload();
  for (uint32_t i = 0; i < eventSize; i++) {
    words[i + 3] = *(event++);
  }
  words[EvtLength - 2] = amc13.getAMC13Trailer();
  words[EvtLength - 1] = amc13.getCDFTrailer();

  // // debugging fragmentLength
  //   cout << "amc13.fragmentLength()         " << amc13.fragmentLength() << endl;
  //   GEMAMC amctest;
  //   cout << "amc13.getCDFTrailer() " << std::bitset<64>(amc13.getCDFTrailer()) << endl;
  //   for (uint32_t i = 0; i < EvtLength; i++) {
  //     cout << i << " " << std::bitset<64>(words[i]) << endl;
  //   }

  //   uint64_t* newevent = (uint64_t*)frdEventMsg->payload();
  //   amctest.setAMCheader1(*(newevent));
  //   amctest.setAMCheader2(*(newevent + 1));
  //   amctest.setGEMeventHeader(*(newevent + 2));
  //   cout << "amctest.lv1Id()         " << amctest.lv1Id() << endl;
  //   cout << "amctest.orbitNumber()   " << amctest.orbitNumber() << endl;
  //   cout << "amctest.bunchCrossing() " << amctest.bunchCrossing() << endl;
  //   cout << "amctest.amcNum()        " << int(amctest.amcNum()) << endl;
  //   cout << "amctest.boardId()       " << amctest.boardId() << endl;
  //   cout << "amctest.davCnt()       " << int(amctest.davCnt()) << endl;

  //   cout << "amc13->fragmentLength() " << amc13.fragmentLength() << endl;
  //   auto amc13n = gemR2D.convertWordToGEMAMC13(words.data());
  //   cout << "amc13n->fragmentLength() " << amc13n->fragmentLength() << endl;
  //   cout << "amc13n->sourceId() " << amc13n->sourceId() << endl;
  //   cout << "amc13n->nAMC() " << int(amc13n->nAMC()) << endl;

  //   for (const auto& amc : *(amc13n->getAMCpayloads())) {
  //     cout << "amc.lv1Id()         " << amc.lv1Id() << endl;
  //     cout << "amc.orbitNumber()   " << amc.orbitNumber() << endl;
  //     cout << "amc.bunchCrossing() " << amc.bunchCrossing() << endl;
  //     cout << "amc.amcNum()        " << int(amc.amcNum()) << endl;
  //     cout << "amc.boardId()       " << amc.boardId() << endl;
  //     cout << "amc.davCnt()       " << int(amc.davCnt()) << endl;
  //     for (const auto& optoHybrid : *amc.gebs()) {
  //       cout << "optoHybrid.inputID() " << int(optoHybrid.inputID()) << endl;
  //       for (auto vfat : *optoHybrid.vFATs()) {
  //         cout << "vfat.vfatId() " << int(vfat.vfatId()) << endl;
  //       }
  //     }
  //   }

  return words;
}
DEFINE_FWK_INPUT_SOURCE(GEMStreamSource);

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

#include "GEMStreamSource.h"
#include "EventFilter/Utilities/interface/crc32c.h"

using namespace std;

GEMStreamSource::GEMStreamSource(edm::ParameterSet const& pset, edm::InputSourceDescription const& desc)
    : ProducerSourceFromFiles(pset, desc, true),
      verifyAdler32_(pset.getUntrackedParameter<bool>("verifyAdler32", true)),
      verifyChecksum_(pset.getUntrackedParameter<bool>("verifyChecksum", true)),
      useL1EventID_(pset.getUntrackedParameter<bool>("useL1EventID", false)) {
  itFileName_ = fileNames(0).begin();
  endFileName_ = fileNames(0).end();
  openFile(*itFileName_);
  produces<FEDRawDataCollection>();
}

bool GEMStreamSource::setRunAndEventInfo(edm::EventID& id,
                                         edm::TimeValue_t& theTime,
                                         edm::EventAuxiliary::ExperimentType& eType) {
  if (fin_.peek() == EOF) {
    if (++itFileName_ == endFileName_) {
      fin_.close();
      return false;
    }
    if (!openFile(*itFileName_)) {
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo") << "could not open file " << *itFileName_;
    }
  }
  //look for FRD header at beginning of the file and skip it
  if (fin_.tellg() == 0) {
    constexpr size_t buf_sz = sizeof(FRDFileHeader_v1);  //try to read v1 FRD header size
    // cout << "FRD header size "<<buf_sz<<endl;
    FRDFileHeader_v1 fileHead;
    fin_.read((char*)&fileHead, buf_sz);

    if (fin_.gcount() == 0)
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo")
          << "Unable to read file or empty file" << *itFileName_;
    else if (fin_.gcount() < (ssize_t)buf_sz)
      fin_.seekg(0);
    else {
      uint16_t frd_version = getFRDFileHeaderVersion(fileHead.id_, fileHead.version_);
      // cout << "frd_version "<<frd_version <<endl;
      if (frd_version >= 1) {
        if (fileHead.headerSize_ < buf_sz)
          throw cms::Exception("GEMStreamSource::setRunAndEventInfo")
              << "Invalid FRD file header (size mismatch) in file " << *itFileName_;
        else if (fileHead.headerSize_ > buf_sz)
          fin_.seekg(fileHead.headerSize_, fin_.beg);
      } else
        fin_.seekg(0, fin_.beg);
    }
  }

  if (detectedFRDversion_ == 0) {
    fin_.read((char*)&detectedFRDversion_, sizeof(uint16_t));
    fin_.read((char*)&flags_, sizeof(uint16_t));
    assert(detectedFRDversion_ > 0 && detectedFRDversion_ <= FRDHeaderMaxVersion);
    if (buffer_.size() < FRDHeaderVersionSize[detectedFRDversion_])
      buffer_.resize(FRDHeaderVersionSize[detectedFRDversion_]);
    *((uint32_t*)(&buffer_[0])) = detectedFRDversion_;
    fin_.read(&buffer_[0] + sizeof(uint32_t), FRDHeaderVersionSize[detectedFRDversion_] - sizeof(uint32_t));
    assert(fin_.gcount() == FRDHeaderVersionSize[detectedFRDversion_] - (unsigned int)(sizeof(uint32_t)));
  } else {
    if (buffer_.size() < FRDHeaderVersionSize[detectedFRDversion_])
      buffer_.resize(FRDHeaderVersionSize[detectedFRDversion_]);
    fin_.read(&buffer_[0], FRDHeaderVersionSize[detectedFRDversion_]);
    assert(fin_.gcount() == FRDHeaderVersionSize[detectedFRDversion_]);
  }
      //cout << "detectedFRDversion_ "<<detectedFRDversion_ <<endl;

  std::unique_ptr<FRDEventMsgView> frdEventMsg(new FRDEventMsgView(&buffer_[0]));
  if (useL1EventID_)
    id = edm::EventID(frdEventMsg->run(), frdEventMsg->lumi(), frdEventMsg->event());
  // cout << "frdEventMsg->run() "<<frdEventMsg->run() <<endl;
  // cout << "frdEventMsg->lumi() "<<frdEventMsg->lumi() <<endl;
  // cout << "frdEventMsg->event() "<<frdEventMsg->event() <<endl;
  // cout << "frdEventMsg->size() "<<frdEventMsg->size() <<endl;
  // cout << "frdEventMsg->flags() "<<frdEventMsg->flags() <<endl;
  // cout << "frdEventMsg->eventSize() "<<frdEventMsg->eventSize() <<endl;
  // cout << "frdEventMsg->paddingSize() "<<frdEventMsg->paddingSize() <<endl;

  const uint32_t totalSize = frdEventMsg->size();
  if (totalSize > buffer_.size()) {
    buffer_.resize(totalSize);
  }
  if (totalSize > FRDHeaderVersionSize[detectedFRDversion_]) {
    fin_.read(&buffer_[0] + FRDHeaderVersionSize[detectedFRDversion_],
              totalSize - FRDHeaderVersionSize[detectedFRDversion_]);
    if (fin_.gcount() != totalSize - FRDHeaderVersionSize[detectedFRDversion_]) {
      throw cms::Exception("GEMStreamSource::setRunAndEventInfo") << "premature end of file " << *itFileName_;
    }
    frdEventMsg = std::make_unique<FRDEventMsgView>(&buffer_[0]);
  }
  // cout << "totalSize "<<totalSize <<endl;

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

  rawData_ = std::make_unique<FEDRawDataCollection>();

  uint32_t eventSize = frdEventMsg->eventSize();
  // cout << "eventSize "<<eventSize <<endl;

  const uint16_t fedId = 1477; //fedHeader.sourceID();
  uint16_t BX_id = 0;
  uint32_t LV1_id = frdEventMsg->event();
  uint32_t OrN = frdEventMsg->event();
  GEMAMC13 amc13;
  amc13.setCDFHeader(0, LV1_id, BX_id, fedId);
  amc13.setAMC13Header(0, 1, OrN);
  amc13.setAMC13Trailer(BX_id, LV1_id, BX_id);
  uint32_t EvtLength = eventSize + 4;  // 2 header and 2 trailer
  amc13.setCDFTrailer(EvtLength);
  const uint32_t fedSize = amc13.fragmentLength() << 3;  //trailer length counts in 8 bytes

  FEDRawData& fedData = rawData_->FEDData(fedId);
  fedData.resize(fedSize);

  uint64_t* event = (uint64_t*)frdEventMsg->payload();
  uint64_t* newFedData = reinterpret_cast<uint64_t*>(fedData.data());
  *(newFedData++) = amc13.getCDFHeader();
  *(newFedData++) = amc13.getAMC13Header();
  for (uint32_t i = 0; i < eventSize; i++) {
    *(newFedData++) = *(event++);
  }
  *(newFedData++) = amc13.getAMC13Trailer();
  *(newFedData++) = amc13.getCDFTrailer();

  //memcpy(fedData.data(), event, fedSize);

  // std::vector<long unsigned int> vdata(event, fedSize);
  //  // cout << "vdata.size() "<<vdata.size() <<endl;
 
  // //while (eventSize > 0) {
  //   assert(eventSize >= FEDTrailer::length);
  //   //eventSize -= FEDTrailer::length;
  //   const FEDTrailer fedTrailer(event + eventSize);
  //   // cout << "fedSize "<<fedSize <<endl;
  //   // cout << "FEDHeader::length "<<FEDHeader::length <<endl;
  //   //assert(eventSize >= fedSize - FEDHeader::length);
  //   //eventSize -= (fedSize - FEDHeader::length);
  //   const FEDHeader fedHeader(event + eventSize);
 
  //}
  //assert(eventSize == 0);

  return true;
}

void GEMStreamSource::produce(edm::Event& e) { e.put(std::move(rawData_)); }

bool GEMStreamSource::openFile(const std::string& fileName) {
  std::// cout << " open file.. " << fileName << std::endl;
  fin_.close();
  fin_.clear();
  size_t pos = fileName.find(':');
  if (pos != std::string::npos) {
    std::string prefix = fileName.substr(0, pos);
    if (prefix != "file")
      return false;
    pos++;
  } else
    pos = 0;

  fin_.open(fileName.substr(pos).c_str(), std::ios::in | std::ios::binary);
  return fin_.is_open();
}

DEFINE_FWK_INPUT_SOURCE(GEMStreamSource);

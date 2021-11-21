#include "DataFormats/GEMDigi/interface/GEMAMC13.h"
#include "DataFormats/GEMDigi/interface/GEMVFAT.h"
#include "gemsw/EventFilter/plugins/GEMLocalModeDataSource.h"
#include "DataFormats/FEDRawData/interface/FEDRawDataCollection.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/Utilities/interface/Exception.h"

#include <fstream>
#include <algorithm>

GEMLocalModeDataSource::GEMLocalModeDataSource(const edm::ParameterSet & pset, edm::InputSourceDescription const &desc) :
edm::ProducerSourceFromFiles(pset,desc,true), // true - RealData
m_hasFerolHeader( pset.getUntrackedParameter<bool>("hasFerolHeader",false)),
m_fedid( pset.getUntrackedParameter<int>("fedId", 12345)),
m_filenames( pset.getUntrackedParameter<std::vector<std::string> >("fileNames" )),
m_fileindex(0),
m_runnumber( pset.getUntrackedParameter<int>("runNumber",-1)),
m_currenteventnumber(1),
m_processEvents(),
m_nProcessedEvents(0),
m_nGoodEvents(0),
m_goodEvents()
{
	produces<FEDRawDataCollection>("gemLocalModeDataSource");
	
	if (pset.exists("processEvents")) {
		m_processEvents= pset.getUntrackedParameter<std::vector<unsigned int> >("processEvents",std::vector<unsigned int>());
	}
	
	int containsListFile=0;
	std::cout << "there are " << m_filenames.size() << " files: ";
	for (unsigned int i=0; i<m_filenames.size(); i++) {
		std::cout << " " << m_filenames.at(i);
		if (m_filenames.at(i).find(".lst")!=std::string::npos)
		containsListFile=1;
	}
	std::cout << "\n";
	
	if (containsListFile) {
		std::cout << "input file definitions contains list file\n";
		std::vector<std::string> inpfnames (m_filenames);
		m_filenames.clear();
		for (unsigned int i=0; i<inpfnames.size(); i++) {
			std::string fn= inpfnames[i];
			if (fn.find(".lst")==std::string::npos) {
				m_filenames.push_back(fn);
				std::cout << " -- keeping " << fn << "\n";
			}
			else {
				std::cout << "loading list from file <" << fn << ">\n";
				std::ifstream fin(fn.c_str());
				std::string line;
				while (!fin.eof() && getline(fin,line)) {
					m_filenames.push_back(line);
					std::cout << " -- adding " << line << "\n";
				}
				fin.close();
			}
		}
		std::cout << std::endl;
	}
	
	edm::storage::IOOffset size = -1;
	edm::storage::StorageFactory::getToModify()->enableAccounting(true);
	
	for (unsigned int i=0; i<m_filenames.size(); i++) {
		std::string fname = m_filenames.at(i);
		bool exists = edm::storage::StorageFactory::get()->check(fname, &size);
		std::cout << "file " << fname << " size " << size << std::endl;
		if (!exists) {
			std::stringstream ss;
			ss << "file not found (" << fname << ")";
			throw cms::Exception(ss.str());
		}
	}
	
	std::string currentfilename = m_filenames[m_fileindex];
	std::cout << "examining " << currentfilename << std::endl;
	m_fileindex++;
	
	// open file stream
	storage = edm::storage::StorageFactory::get()->open(currentfilename);
	// (throw if storage is 0)
	if (!storage) {
		std::stringstream ss;
		ss << "GEMLocalModeDataSource: failed to open the file";
		throw cms::Exception(ss.str());
	}
	
	m_runnumber = m_fileindex; // dummy run number
}


GEMLocalModeDataSource::~GEMLocalModeDataSource()
{
	std::cout << "GEMLocalModeDataSource::~GEMLocalModeDataSource nGoodEvents=" << m_nGoodEvents << ", nProcessedEvents=" << m_nProcessedEvents << std::endl;
	std::cout << " their numbers (may be limited to 100)\n";
	for (unsigned int i=0; i<m_goodEvents.size(); i++) {
		std::cout << " , " << m_goodEvents[i];
		if (i>99) break;
	}
	std::cout << "\n";
}


void GEMLocalModeDataSource::debug_studyFile()
{
	std::cout << "\ndebug_studyFile" << std::endl;
	std::cout << "m_fileindex=" << m_fileindex << ", m_filenames.size=" << m_filenames.size() << std::endl;
	std::string currentfilename= m_filenames[m_fileindex-1];
	std::cout << "checkpoint A" << std::endl;
	std::string fname= currentfilename.substr(5,currentfilename.size());
	std::cout << "trying to open the file " << fname << std::endl;
	std::ifstream fin(fname.c_str(),std::ios::binary);
	if (!fin.is_open()) {
		std::cout << "failed" << std::endl;
		return;
	}
	
	const int bufsize=150;
	uint64_t buf[bufsize];
	if (m_hasFerolHeader && 1) {
		fin.read((char*)&buf,3*sizeof(uint64_t));
		
		for (uint64_t i=0; i<3*sizeof(uint64_t); i++) {
			char c= ((char*)&buf)[i];
			std::cout << "i=" << i << ", code=0x" << std::hex << buf[i] << std::dec << ", char=" << c << std::endl;
		}
	}
	fin.read((char*)&buf,bufsize*sizeof(uint64_t));
	
	if (0) {
		uint64_t n= fin.gcount();
		std::cout << "gcount=" << n << "\n";
		if (n>205) n=205;
		for (uint64_t i=0; i<n; i++) {
			// for some reason this if does not work
			if (i>200) { std::cout << "limiting to " << i << std::endl; break; }
			char c= ((char*)&buf)[i];
			std::cout << "i=" << i << ", code=0x" << std::hex << buf[i] << std::dec << ", char=" << c << std::endl;
		}
	}
	
	std::cout << "got words\n";
	GEMAMC13 amc13Event;
	for (int i=0; i<bufsize-1; i++) {
		amc13Event.setAMC13Header(0, 1, 0);
		
		int ok=0;
		//if (buf[i] > ((uint64_t)(1)<<60)) std::cout << "b ";
		//else { std::cout << "g "; ok=1; }
		//if (amc13Event.get_cb5()==5) ok=1;
		std::cout << ((ok) ? "g " : "b ");
		std::cout << "0x" << std::hex << buf[i] << std::dec << "  ";
		/*
		 if (ok) {
		 std::cout << "cb5=" << amc13Event.get_cb5() << " cb0=" << amc13Event.get_cb0()
		 << " nAMC=" << amc13Event.get_nAMC();
		 }
		 */
		std::cout << std::endl;
	}
	
	fin.close();
	std::cout << "debug_studyFile done" << std::endl;
}

void GEMLocalModeDataSource::fillDescriptions(edm::ConfigurationDescriptions & descriptions)
{
	edm::ParameterSetDescription desc;
	desc.setComment("Reads GEM data saved in 'local mode' to produce FEDRawDataCollection.");
	desc.addUntracked<std::vector<std::string> >("fileNames")
	->setComment("Names of files to be processed.");
	desc.addUntracked<unsigned int>("skipEvents")
	->setComment("Number of events to skip before next processing.");
	desc.addUntracked<bool>("hasFerolHeader", false)
	->setComment("Whether additional header is present.");
	desc.addUntracked<int>("fedId", 12345)
	->setComment("FedID value to embed into events.");
	desc.addUntracked<int>("runNumber", -1)
	->setComment("Which runNumber to embed:\n -1 - get from filename,\n other - use this value.");
	desc.addUntracked<std::vector<unsigned int> >("processEvents", std::vector<unsigned int>());
	desc.addUntracked<std::vector<edm::LuminosityBlockID>>("firstLuminosityBlockForEachRun", {});
	//ProductSelectorRules::fillDescription(desc, "inputCommands");
	
	descriptions.addDefault(desc);
}


bool GEMLocalModeDataSource::setRunAndEventInfo(edm::EventID &id, edm::TimeValue_t &time, edm::EventAuxiliary::ExperimentType &)
{
	if (0 && (m_currenteventnumber==1)) {
		debug_studyFile();
		return false;
	}
	
	if (storage->eof()) {
		storage->close();
		
		if (m_fileindex >= m_filenames.size()) {
			std::cout << "end of last file" << std::endl;
			return false;
		}
		std::string currentfilename = m_filenames[m_fileindex];
		std::cout << "processing " << currentfilename << std::endl;
		m_fileindex++;
		storage = edm::storage::StorageFactory::get()->open(currentfilename);
		if (!storage) {
			std::cout << "failed to open next file" << std::endl;
			return false;
		}
	}
	
	edm::storage::Storage & inpFile = *storage;
	
	// create product (raw data)
	buffers.reset( new FEDRawDataCollection );
	
	// assume 1 record is 1 event
	std::vector<uint64_t> buf;
	const int tmpBufSize=40;
	uint64_t tmpBuf[tmpBufSize];
	do {
		
		buf.clear();
		
		int prn=0;
		
		if (1 && m_processEvents.size() &&
				(std::find(m_processEvents.begin(),m_processEvents.end(),m_currenteventnumber-1)!=m_processEvents.end())) {
			prn=1;
		}
		if (prn ==1)std::cout << "dd"<<std::endl;
		
		if (m_hasFerolHeader) {
			if (inpFile.read((char*)tmpBuf,3*sizeof(uint64_t))!=3*sizeof(uint64_t)) {
				std::cout << "failed to read next FerolHeader" << std::endl;
				storage->close();
				return false;
			}
			if (0) {
				std::cout << "headerSkipped" << std::endl;
				for (int i=0; i<3; i++) std::cout << "h" << (i+1) << " 0x" << std::hex << tmpBuf[i] << std::dec << "\n";
			}
		}
		
		GEMAMC13 amc13Event;
		amc13Event.setCDFHeader(1, 0, 0, m_fedid);
		amc13Event.setAMC13Header(0, 1, 0);
        uint32_t amc13EvtLength = 0;
		
		for (uint8_t ii=0; ii<amc13Event.nAMC(); ii++) {
			amc13Event.addAMCheader(tmpBuf[ii]);
			buf.push_back(tmpBuf[ii]);
		}
		
        uint64_t lv1Id = 0;
        uint64_t orbitNum = 0;
        uint64_t bx = 0;
		// read AMC payloads
		for (uint8_t iamc = 0; iamc<amc13Event.nAMC(); ++iamc) {
            uint32_t amcSize = 0;
			GEMAMC amcData;
			inpFile.read((char*)tmpBuf, 3*sizeof(uint64_t));
			for (int ii=0; ii<3; ii++) buf.push_back(tmpBuf[ii]);
			amcData.setAMCheader1(tmpBuf[0]);
			amcData.setAMCheader2(tmpBuf[1]);
			amcData.setGEMeventHeader(tmpBuf[2]);

            amcSize += 3;

            lv1Id = amcData.lv1Id();
            bx = amcData.bunchCrossing();
            orbitNum = amcData.orbitNumber();
			
			// read GEB
			for (uint8_t igeb=0; igeb<amcData.davCnt(); igeb++) {
				GEMOptoHybrid gebData;
				inpFile.read((char*)tmpBuf, sizeof(uint64_t));
				buf.push_back(tmpBuf[0]);
				gebData.setChamberHeader(tmpBuf[0]);
                //std::cout << "OH : " << int(gebData.inputID()) << std::endl;

                amcSize += 1;

				if (gebData.vfatWordCnt()%3!=0) {
					throw cms::Exception("gebData.vfatWordCnt()%3!=0");
				}
                amcSize += gebData.vfatWordCnt();
				
				// check if one buffer accommodates information
				if (gebData.vfatWordCnt() <= tmpBufSize) {
					// one buffer is enough
					inpFile.read((char*)tmpBuf, gebData.vfatWordCnt() * sizeof(uint64_t));
					for (int ii=0; ii < gebData.vfatWordCnt(); ii++) {
						buf.push_back(tmpBuf[ii]);
					}
					
				}
				else {
					// use buffer several times
					const int allowNumBufs= 32000/tmpBufSize; // 32k is firmware limit
					if (allowNumBufs * tmpBufSize<gebData.vfatWordCnt()) {
						std::cout << "update code: tmpBufSize=" << tmpBufSize << ", allowNumBufs=" << allowNumBufs << ", tmpBufSize*allowNumBufs=" << (tmpBufSize*allowNumBufs) << " (firmware limit), gebData.vfatWordCnt=" << gebData.vfatWordCnt() << std::endl;
						std::cout << "current file name is " << m_filenames[m_fileindex-1] << std::endl;
						return false;
					}
					
					int neededBufs= gebData.vfatWordCnt()/tmpBufSize;
					if (gebData.vfatWordCnt()%tmpBufSize>0) neededBufs++;
					if (neededBufs>allowNumBufs) {
						std::cout << "code error (neededBufs>allowNumBufs)\n";
						return false;
					}
					for (int iUse=0; iUse<neededBufs; iUse++) {
						uint32_t chunkSize= tmpBufSize;
						if (iUse*tmpBufSize+chunkSize > gebData.vfatWordCnt()) {
							chunkSize= gebData.vfatWordCnt() - iUse*tmpBufSize;
						}

						inpFile.read((char*)tmpBuf, chunkSize * sizeof(uint64_t));
						for (unsigned int ii=0; ii < chunkSize; ii++) {
							buf.push_back(tmpBuf[ii]);
						}
					}
				}
				
				// read gebData trailer
				inpFile.read((char*)tmpBuf, sizeof(uint64_t));
				buf.push_back(tmpBuf[0]);
                amcSize += 1;
				
				// check
				gebData.setChamberTrailer(tmpBuf[0]);
				//if (prn) std::cout << " -- " << gebData.getChamberTrailer_str() << std::endl;
				if (gebData.vfatWordCntT() != gebData.vfatWordCnt()) {
					std::cout << "corrupt data? gebData vfatWordCnt does not match" << std::endl;
				} 
			} // end of geb loop
			
			// read GEMeventTrailer and AMCTrailer
			inpFile.read((char*)tmpBuf, 2*sizeof(uint64_t));
			buf.push_back(tmpBuf[0]);
			buf.push_back(tmpBuf[1]);
            amcSize += 2;
            amc13EvtLength += amcSize + 1;  // AMC data size + AMC header size
		} // end of amc loop
		
        uint32_t EvtLength = amc13EvtLength + 4;  // 2 header and 2 trailer
        amc13Event.setCDFTrailer(EvtLength);
        amc13Event.setAMC13Trailer(0, 0, 0);
        buf.push_back(amc13Event.getAMC13Trailer());
        buf.push_back(amc13Event.getCDFTrailer());

		amc13Event.setCDFHeader(0x1, lv1Id, bx, m_fedid);
		amc13Event.setAMC13Header(0, 1, orbitNum);

        buf.insert(buf.begin(), amc13Event.getAMC13Header());
        buf.insert(buf.begin(), amc13Event.getCDFHeader());

		if (buf.size()>12) {
			m_nGoodEvents++;
			if (m_goodEvents.size()<100)
			m_goodEvents.push_back(m_currenteventnumber-1);
		}
		
		m_currenteventnumber++;
		
		if (m_processEvents.size() &&
				(std::find(m_processEvents.begin(),m_processEvents.end(),m_currenteventnumber-1)!=m_processEvents.end())) {
			std::cout << "got it" << std::endl;
			break;
		}
	}
	while (m_processEvents.size());
	
	//
	// create FEDRawData
	
	auto rawData = std::make_unique<FEDRawData>(sizeof(uint64_t)*buf.size());
	unsigned char *dataptr = rawData->data();
	
	for (uint16_t i=0; i<buf.size(); i++) {
		((uint64_t*)dataptr)[i] = buf[i];
	}
	
	FEDRawData & fedRawData = buffers->FEDData( m_fedid );
	fedRawData = *rawData;
	
	// get real event number
	uint32_t realeventno = synchronizeEvents();
	int set_runnumber= (m_runnumber!=0) ? m_runnumber : id.run();
	id = edm::EventID(set_runnumber, id.luminosityBlock(), realeventno);
	return true;
}


void GEMLocalModeDataSource::produce(edm::Event &event) {
	//std::cout << "GEMLocalModeDataSource::produce" << std::endl;
	event.put(std::move(buffers), "gemLocalModeDataSource");
	buffers.reset();
}

uint32_t GEMLocalModeDataSource::synchronizeEvents() {
	//std::cout << "GEMLocalModeDataSource::synchronizeEvents" << std::endl;
	int32_t result= m_currenteventnumber -1;
	return(uint32_t) result;
}


#include "FWCore/Framework/interface/InputSourceMacros.h"
DEFINE_FWK_INPUT_SOURCE(GEMLocalModeDataSource);


#include "gemsw/DQM/plugins/QC8DAQStatusSource.h"

using namespace std;
using namespace edm;

QC8DAQStatusSource::QC8DAQStatusSource(const edm::ParameterSet &cfg)
    : gemChMapToken_(esConsumes<GEMChMap, GEMChMapRcd, edm::Transition::BeginRun>()) {
  tagVFAT_ = consumes<GEMVFATStatusCollection>(cfg.getParameter<edm::InputTag>("VFATInputLabel"));
  tagOH_ = consumes<GEMOHStatusCollection>(cfg.getParameter<edm::InputTag>("OHInputLabel"));
  tagAMC_ = consumes<GEMAMCStatusCollection>(cfg.getParameter<edm::InputTag>("AMCInputLabel"));

  nAMCSlots_ = cfg.getParameter<int>("AMCSlots");
  nOHSlots_ = cfg.getParameter<int>("OHSlots");
  nVFATSlots_ = cfg.getParameter<int>("VFATSlots");
}

void QC8DAQStatusSource::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("VFATInputLabel", edm::InputTag("muonGEMDigis", "VFATStatus"));
  desc.add<edm::InputTag>("OHInputLabel", edm::InputTag("muonGEMDigis", "OHStatus"));
  desc.add<edm::InputTag>("AMCInputLabel", edm::InputTag("muonGEMDigis", "AMCStatus"));

  desc.add<int>("AMCSlots", 4);
  desc.add<int>("OHSlots", 12);
  desc.add<int>("VFATSlots", 12);

  descriptions.add("QC8DAQStatusSource", desc);
}

void QC8DAQStatusSource::SetLabelAMCStatus(MonitorElement *h2Status) {
  if (h2Status == nullptr) {
    return;
  }

  unsigned int unBinPos = 1;
  h2Status->setBinLabel(unBinPos++, "Good", 2);
  h2Status->setBinLabel(unBinPos++, "Invalid OH", 2);
  h2Status->setBinLabel(unBinPos++, "Back pressure", 2);
  h2Status->setBinLabel(unBinPos++, "Bad EC", 2);
  h2Status->setBinLabel(unBinPos++, "Bad BC", 2);
  h2Status->setBinLabel(unBinPos++, "Bad OC", 2);
  h2Status->setBinLabel(unBinPos++, "Bad run type", 2);
  h2Status->setBinLabel(unBinPos++, "Bad CRC", 2);
  h2Status->setBinLabel(unBinPos++, "MMCM locked", 2);
  h2Status->setBinLabel(unBinPos++, "DAQ clock locked", 2);
  h2Status->setBinLabel(unBinPos++, "DAQ not ready", 2);
  h2Status->setBinLabel(unBinPos++, "BC0 not locked", 2);
}

void QC8DAQStatusSource::SetLabelOHStatus(MonitorElement *h2Status) {
  if (h2Status == nullptr) {
    return;
  }

  unsigned int unBinPos = 1;
  h2Status->setBinLabel(unBinPos++, "Good", 2);
  h2Status->setBinLabel(unBinPos++, "Event FIFO near full", 2);
  h2Status->setBinLabel(unBinPos++, "Input FIFO near full", 2);
  h2Status->setBinLabel(unBinPos++, "L1A FIFO near full", 2);
  h2Status->setBinLabel(unBinPos++, "Event size warn", 2);
  h2Status->setBinLabel(unBinPos++, "Invalid VFAT", 2);
  h2Status->setBinLabel(unBinPos++, "Event FIFO full", 2);
  h2Status->setBinLabel(unBinPos++, "Input FIFO full", 2);
  h2Status->setBinLabel(unBinPos++, "L1A FIFO full", 2);
  h2Status->setBinLabel(unBinPos++, "Event size overflow", 2);
  h2Status->setBinLabel(unBinPos++, "Invalid event", 2);
  h2Status->setBinLabel(unBinPos++, "Out of Sync AMC vs VFAT", 2);
  h2Status->setBinLabel(unBinPos++, "Out of Sync VFAT vs VFAT", 2);
  h2Status->setBinLabel(unBinPos++, "BX mismatch AMC vs VFAT", 2);
  h2Status->setBinLabel(unBinPos++, "BX mismatch VFAT vs VFAT", 2);
  h2Status->setBinLabel(unBinPos++, "Input FIFO underflow", 2);
  h2Status->setBinLabel(unBinPos++, "Bad VFAT count", 2);
}

void QC8DAQStatusSource::SetLabelVFATStatus(MonitorElement *h2Status) {
  if (h2Status == nullptr) {
    return;
  }

  unsigned int unBinPos = 1;
  h2Status->setBinLabel(unBinPos++, "Good", 2);
  h2Status->setBinLabel(unBinPos++, "Basic overflow", 2);
  h2Status->setBinLabel(unBinPos++, "Zero-sup overflow", 2);
  h2Status->setBinLabel(unBinPos++, "VFAT CRC error", 2);
  h2Status->setBinLabel(unBinPos++, "Invalid header", 2);
  h2Status->setBinLabel(unBinPos++, "AMC EC mismatch", 2);
  h2Status->setBinLabel(unBinPos++, "AMC BC mismatch", 2);
}

void QC8DAQStatusSource::bookHistograms(DQMStore::IBooker &ibooker, edm::Run const &, edm::EventSetup const &iSetup) {
  const auto &chMap = iSetup.getData(gemChMapToken_);
  auto gemChMap = std::make_unique<GEMChMap>(chMap);

  std::string daqFolder = "GEM/DAQStatus";
  ibooker.cd();
  ibooker.setCurrentFolder(daqFolder);
  for (auto const &[ec, dc] : gemChMap->chamberMap()) {
    mapDCtoEC_[dc] = ec;
    unsigned int fedId = ec.fedId;
    unsigned int amcNum = ec.amcNum;
    unsigned int ohN = ec.gebId;
    GEMDetId gemChId(dc.detId);
    Key2 key2(fedId, amcNum);
    Key3 key3(fedId, amcNum, ohN);

    if (mapAMCStatus_.find(fedId) == mapAMCStatus_.end()) {
      auto strName = Form("fedId%d", int(fedId));
      auto strTitle = Form("FED %d;AMC slot;", int(fedId));
      auto eventBasedTitle = Form("FED %d;Event Id;AMC slot", int(fedId));

      ibooker.setCurrentFolder(daqFolder+"/AMCStatus");
      mapAMCStatus_[fedId] = ibooker.book2D(strName, strTitle, nAMCSlots_, -0.5, nAMCSlots_ - 0.5, nBitAMC_, 0.5, nBitAMC_ + 0.5);
      SetLabelAMCStatus(mapAMCStatus_[fedId]);

      auto nBinsX = nMAXEvents_/nEventsPerBin_;

      ibooker.setCurrentFolder(daqFolder+"/AMCStatus/EventBased/Exists");
      mapAMCExists_[fedId] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nAMCSlots_, -0.5, nAMCSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/AMCStatus/EventBased/Errors");
      mapAMCErrors_[fedId] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nAMCSlots_, -0.5, nAMCSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/AMCStatus/EventBased/Warnings");
      mapAMCWarnings_[fedId] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nAMCSlots_, -0.5, nAMCSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/AMCStatus/EventBased/Summary");
      mapAMCSummary_[fedId] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nAMCSlots_, -0.5, nAMCSlots_ - 0.5);
    }
    if (mapOHStatus_.find(key2) == mapOHStatus_.end()) {
      auto strName = Form("fedId%d_amc%d", fedId, amcNum);
      auto strTitle = Form("FED %d AMC %d;OH no.;", fedId, amcNum);
      auto eventBasedTitle = Form("FED %d AMC %d;Event Id;OH no", fedId, amcNum);
      
      ibooker.setCurrentFolder(daqFolder+"/OHStatus");
      mapOHStatus_[key2] = ibooker.book2D(strName, strTitle, nOHSlots_, -0.5, nOHSlots_ - 0.5, nBitOH_, 0.5, nBitOH_ + 0.5);
      SetLabelOHStatus(mapOHStatus_[key2]);

      auto nBinsX = nMAXEvents_/nEventsPerBin_;
      
      ibooker.setCurrentFolder(daqFolder+"/OHStatus/Exists");
      mapOHExists_[key2] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nOHSlots_, -0.5, nOHSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/OHStatus/Errors");
      mapOHErrors_[key2] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nOHSlots_, -0.5, nOHSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/OHStatus/Warnings");
      mapOHWarnings_[key2] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nOHSlots_, -0.5, nOHSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/OHStatus/Summary");
      mapOHSummary_[key2] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nOHSlots_, -0.5, nOHSlots_ - 0.5);
    }
    if (mapVFATStatus_.find(key3) == mapVFATStatus_.end()) {
      auto strName = Form("fedId%d_amc%d_oh%d", fedId, amcNum, ohN);
      auto strTitle = Form("FED %d AMC %d OH %d;VFAT ID;", fedId, amcNum, ohN);
      auto eventBasedTitle = Form("FED %d AMC %d OH %d;Event Id;VFAT ID", fedId, amcNum, ohN);

      ibooker.setCurrentFolder(daqFolder+"/VFATStatus");
      mapVFATStatus_[key3] = ibooker.book2D(strName, strTitle, nVFATSlots_, -0.5, nVFATSlots_ - 0.5, nBitVFAT_, 0.5, nBitVFAT_ + 0.5);
      SetLabelVFATStatus(mapVFATStatus_[key3]);

      auto nBinsX = nMAXEvents_/nEventsPerBin_;

      ibooker.setCurrentFolder(daqFolder+"/VFATStatus/Exists");
      mapVFATExists_[key3] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nVFATSlots_, -0.5, nVFATSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/VFATStatus/Errors");
      mapVFATErrors_[key3] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nVFATSlots_, -0.5, nVFATSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/VFATStatus/Warnings");
      mapVFATWarnings_[key3] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nVFATSlots_, -0.5, nVFATSlots_ - 0.5);

      ibooker.setCurrentFolder(daqFolder+"/VFATStatus/Summary");
      mapVFATSummary_[key3] = ibooker.book2D(strName, eventBasedTitle, nBinsX, 0, nMAXEvents_, nVFATSlots_, -0.5, nVFATSlots_ - 0.5);
    }
  }
}

void QC8DAQStatusSource::fillSummary(int xBin,
                                     dqm::impl::MonitorElement* hSummary,
                                     dqm::impl::MonitorElement* hExists,
                                     dqm::impl::MonitorElement* hError,
                                     dqm::impl::MonitorElement* hWarning) {
  auto nBinsY = hExists->getNbinsY();
  for (auto yBin = 1; yBin <= nBinsY; yBin++) {
    auto nExists = hExists->getBinContent(xBin, yBin);
    auto nError = hError->getBinContent(xBin, yBin);
    auto nWarning = hWarning->getBinContent(xBin, yBin);

    if (nError > 0) hSummary->setBinContent(xBin, yBin, DAQStatus::Error);
    else if (nWarning > 0) hSummary->setBinContent(xBin, yBin, DAQStatus::Warning);
    else if (nExists > 0) hSummary->setBinContent(xBin, yBin, DAQStatus::Good);
  }
}

void QC8DAQStatusSource::analyze(edm::Event const &event, edm::EventSetup const &eventSetup) {
  edm::Handle<GEMVFATStatusCollection> gemVFAT;
  edm::Handle<GEMOHStatusCollection> gemOH;
  edm::Handle<GEMAMCStatusCollection> gemAMC;

  event.getByToken(tagVFAT_, gemVFAT);
  event.getByToken(tagOH_, gemOH);
  event.getByToken(tagAMC_, gemAMC);

  if (!(gemVFAT.isValid() && gemOH.isValid() && gemAMC.isValid())) {
    edm::LogWarning("QC8DAQStatusSource") << "DAQ sources from muonGEMDigis are not found";
    return;
  }

  int evtId = event.id().event();

  for (auto amcIt = gemAMC->begin(); amcIt != gemAMC->end(); ++amcIt) {
    int fedId = (*amcIt).first;
    if (mapAMCStatus_.find(fedId) == mapAMCStatus_.end()) {
      continue;
    }
    auto h2AMCStatus = mapAMCStatus_[fedId];

    const GEMAMCStatusCollection::Range &range = (*amcIt).second;
    for (auto amc = range.first; amc != range.second; ++amc) {
      int amcNum = amc->amcNumber();

      GEMAMCStatus::Warnings warnings{amc->warnings()};
      GEMAMCStatus::Errors errors{amc->errors()};

      bool bWarn = warnings.wcodes != 0;
      bool bErr = errors.ecodes != 0;
      mapAMCExists_[fedId]->Fill(evtId, amcNum);
      if (!bWarn && !bErr) {
        h2AMCStatus->Fill(amcNum, 1);
        continue;
      } else if (bErr) {
        mapAMCErrors_[fedId]->Fill(evtId, amcNum);
      } else if (bWarn) {
        mapAMCWarnings_[fedId]->Fill(evtId, amcNum);
      }

      if (warnings.InValidOH)
        h2AMCStatus->Fill(amcNum, 2);
      if (warnings.backPressure)
        h2AMCStatus->Fill(amcNum, 3);
      if (errors.badEC)
        h2AMCStatus->Fill(amcNum, 4);
      if (errors.badBC)
        h2AMCStatus->Fill(amcNum, 5);
      if (errors.badOC)
        h2AMCStatus->Fill(amcNum, 6);
      if (errors.badRunType)
        h2AMCStatus->Fill(amcNum, 7);
      if (errors.badCRC)
        h2AMCStatus->Fill(amcNum, 8);
      if (errors.MMCMlocked)
        h2AMCStatus->Fill(amcNum, 9);
      if (errors.DAQclocklocked)
        h2AMCStatus->Fill(amcNum, 10);
      if (errors.DAQnotReday)
        h2AMCStatus->Fill(amcNum, 11);
      if (errors.BC0locked)
        h2AMCStatus->Fill(amcNum, 12);
    }
  }

  for (auto ohIt = gemOH->begin(); ohIt != gemOH->end(); ++ohIt) {
    GEMDetId gid = (*ohIt).first;

    const GEMOHStatusCollection::Range &range = (*ohIt).second;
    for (auto OHStatus = range.first; OHStatus != range.second; ++OHStatus) {
      int chamberType = OHStatus->chamberType();
      GEMChMap::chamDC dc;
      dc.detId = gid.rawId() ;
      dc.chamberType = chamberType;
      auto ec = mapDCtoEC_[dc];
      unsigned int fedId = ec.fedId;
      unsigned int amcNum = ec.amcNum;
      unsigned int ohN = ec.gebId;
      Key2 key2(fedId, amcNum);

      auto h2OHStatus = mapOHStatus_[key2];

      GEMOHStatus::Warnings warnings{OHStatus->warnings()};
      GEMOHStatus::Errors errors{OHStatus->errors()};

      bool bWarn = warnings.wcodes != 0;
      bool bErr = errors.codes != 0;

      mapOHExists_[key2]->Fill(evtId, ohN);

      if (!bErr) {
        for (int vfatId = 0; vfatId < nVFATSlots_; vfatId++) {
          bool zsed = ((OHStatus->zsMask() >> vfatId) & 0x1);// == 0x1;
          bool exist = ((OHStatus->existVFATs() >> vfatId) & 0x1);// == 0x1;
          Key3 key3(fedId, amcNum, ohN);
          if (exist or zsed) {
            mapVFATExists_[key3]->Fill(evtId, vfatId);
          }
        }
      }

      if (!bWarn && !bErr) {
        h2OHStatus->Fill(ohN, 1);
        continue;
      } else if (bErr) {
        mapOHErrors_[key2]->Fill(evtId, ohN);
      } else if (bWarn) {
        mapOHWarnings_[key2]->Fill(evtId, ohN);
      }

      if (warnings.EvtNF)
        h2OHStatus->Fill(ohN, 2);
      if (warnings.InNF)
        h2OHStatus->Fill(ohN, 3);
      if (warnings.L1aNF)
        h2OHStatus->Fill(ohN, 4);
      if (warnings.EvtSzW)
        h2OHStatus->Fill(ohN, 5);
      if (warnings.InValidVFAT)
        h2OHStatus->Fill(ohN, 6);

      if (errors.EvtF)
        h2OHStatus->Fill(ohN, 7);
      if (errors.InF)
        h2OHStatus->Fill(ohN, 8);
      if (errors.L1aF)
        h2OHStatus->Fill(ohN, 9);
      if (errors.EvtSzOFW)
        h2OHStatus->Fill(ohN, 10);
      if (errors.Inv)
        h2OHStatus->Fill(ohN, 11);
      if (errors.OOScAvV)
        h2OHStatus->Fill(ohN, 12);
      if (errors.OOScVvV)
        h2OHStatus->Fill(ohN, 13);
      if (errors.BxmAvV)
        h2OHStatus->Fill(ohN, 14);
      if (errors.BxmVvV)
        h2OHStatus->Fill(ohN, 15);
      if (errors.InUfw)
        h2OHStatus->Fill(ohN, 16);
      if (errors.badVFatCount)
        h2OHStatus->Fill(ohN, 17);
    }
  }

  for (auto vfatIt = gemVFAT->begin(); vfatIt != gemVFAT->end(); ++vfatIt) {
    GEMDetId gid = (*vfatIt).first;

    const GEMVFATStatusCollection::Range &range = (*vfatIt).second;
    for (auto vfatStat = range.first; vfatStat != range.second; ++vfatStat) {
      int chamberType = vfatStat->chamberType();
      GEMChMap::chamDC dc;
      dc.detId = gid.rawId();
      dc.chamberType = chamberType;
      auto ec = mapDCtoEC_[dc];
      unsigned int fedId = ec.fedId;
      unsigned int amcNum = ec.amcNum;
      unsigned int ohN = ec.gebId;
      Key3 key3(fedId, amcNum, ohN);

      auto h2VFATStatus = mapVFATStatus_[key3];
        
      auto vfatPos = int(vfatStat->vfatPosition());

      GEMVFATStatus::Warnings warnings{vfatStat->warnings()};
      GEMVFATStatus::Errors errors{(uint8_t)vfatStat->errors()};

      bool bWarn = warnings.wcodes != 0;
      bool bErr = errors.codes != 0;
      if (!bWarn && !bErr) {
        h2VFATStatus->Fill(vfatPos, 1);
        continue;
      } else if (bErr) {
        mapVFATErrors_[key3]->Fill(evtId, vfatPos);
      } else if (bWarn) {
        mapVFATWarnings_[key3]->Fill(evtId, vfatPos);
      }

      if (warnings.basicOFW)
        h2VFATStatus->Fill(vfatPos, 2);
      if (warnings.zeroSupOFW)
        h2VFATStatus->Fill(vfatPos, 3);

      if (errors.vc)
        h2VFATStatus->Fill(vfatPos, 4);
      if (errors.InValidHeader)
        h2VFATStatus->Fill(vfatPos, 5);
      if (errors.EC)
        h2VFATStatus->Fill(vfatPos, 6);
      if (errors.BC)
        h2VFATStatus->Fill(vfatPos, 7);
    }
  }

  if ((evtId % nEventsPerBin_) == 0) {
    // Fill summary
    int xBin = evtId / nEventsPerBin_;
    for (auto &[key, h] : mapAMCStatus_) {
      fillSummary(xBin, mapAMCSummary_[key], mapAMCExists_[key], mapAMCErrors_[key], mapAMCWarnings_[key]);
    }
    for (auto &[key, h] : mapOHStatus_) {
      fillSummary(xBin, mapOHSummary_[key], mapOHExists_[key], mapOHErrors_[key], mapOHWarnings_[key]);
    }
    for (auto &[key, h] : mapVFATStatus_) {
      fillSummary(xBin, mapVFATSummary_[key], mapVFATExists_[key], mapVFATErrors_[key], mapVFATWarnings_[key]);
    }
  }
}

DEFINE_FWK_MODULE(QC8DAQStatusSource);

#include "gemsw/DQM/plugins/QC8RecHitSource.h"

using namespace std;
using namespace edm;

int nEvent = 0;
QC8RecHitSource::QC8RecHitSource(const edm::ParameterSet &cfg)
  : hGEMGeom_(esConsumes()),
    hGEMGeomBeginRun_(esConsumes<edm::Transition::BeginRun>()) {
  gemRecHits_ = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("rechitInputLabel"));
  gemDigis_ = consumes<GEMDigiCollection>(cfg.getParameter<edm::InputTag>("digiInputLabel"));
}

void QC8RecHitSource::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("digiInputLabel", edm::InputTag("muonGEMDigis"));
  desc.add<edm::InputTag>("rechitInputLabel", edm::InputTag("gemRecHits"));
  descriptions.add("QC8RecHitSource", desc);
}

void QC8RecHitSource::bookHistograms(DQMStore::IBooker &booker, edm::Run const &, edm::EventSetup const &iSetup) {
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeomBeginRun_);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  for (auto chamber : GEMGeometry_->chambers()) {
    auto chamberId = chamber->id();
    auto ch = chamberId.chamber();
    auto etaPart = GEMGeometry_->etaPartitions();

    // Uploaded
    booker.setCurrentFolder("GE21QC8/Digi");
    mapDigiOcc_[ch] = booker.book2D(Form("digi_occ_ch%d", ch),
                                    Form("Digi Occupancy Chamber %d;strip;i#eta", ch),
                                    384, -0.5, 383.5,
                                    16, 0.5, 16.5);
    //Uploaded 
    booker.setCurrentFolder("GE21QC8/RecHit");
    mapRecHitOcc_[ch] = booker.book2D(Form("rechit_occ_ch%d", ch),
                                      Form("Rechit Occupancy Chamber %d;x [cm]; y [cm]", ch),
                                      600, -60, 60,
                                      30, -70, 130);
    mapRecHitClsSize_[ch] = booker.book2D(Form("rechit_cls_size_ch%d", ch),
                                          Form("Rechit Cluster Size Chamber %d;Cluster Size;i#eta", ch),
                                          maxClsSizeToShow_, 0.5, maxClsSizeToShow_ + 0.5,
                                          16, 0.5, 16.5);
    
    //Not Uploaded 
    // 1 hour
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc1hrEta1_4_[ch] = booker.book2D(Form("digiStrip_occ_1hr_ch%d_eta1_4", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 1-4: 1 Hour;events;strip", ch),
                                    6000, 0, 375000,
                                    1536, 0, 1535);
    // Not Uploaded
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc1hrEta5_8_[ch] = booker.book2D(Form("digiStrip_occ_1hr_ch%d_eta5_8", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 5-8: 1 Hour;events;strip", ch),
                                    6000, 0, 375000,
                                    1536, 0, 1535);
    //Not Uploaded
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc1hrEta9_12_[ch] = booker.book2D(Form("digiStrip_occ_1hr_ch%d_eta9_12", ch),
                                     Form("DigiStrip Occupancy Chamber %d Eta 9-12: 1 Hour;events;strip", ch),
                                     6000, 0, 375000,
                                     1536, 0, 1535);                                                         // Not Uploaded 
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc1hrEta13_16_[ch] = booker.book2D(Form("digiStrip_occ_1hr_ch%d_eta13_16", ch),
                                      Form("DigiStrip Occupancy Chamber %d Eta 13-16: 1 Hour;events;strip", ch),
                                      6000, 0, 375000,
                                      1536, 0, 1535);

    // Not Uploaded
    // 5 Hour
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc5hrEta1_4_[ch] = booker.book2D(Form("digiStrip_occ_5hr_ch%d_eta1_4", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 1-4: 5 Hours;events;strip", ch),
                                    6000, 0, 2000000,
                                    //60000, 0, 2000000,
                                    1536, 0, 1535);
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc5hrEta5_8_[ch] = booker.book2D(Form("digiStrip_occ_5hr_ch%d_eta5_8", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 5-8;events: 5 Hours;strip", ch),
                                    6000, 0, 2000000,
                                    //60000, 0, 2000000,
                                    1536, 0, 1535);
    // Not Uploaded
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc5hrEta9_12_[ch] = booker.book2D(Form("digiStrip_occ_5hr_ch%d_eta9_12", ch),
                                     Form("DigiStrip Occupancy Chamber %d Eta 9-12: 5 Hours;events;strip", ch),
                                     6000, 0, 2000000,
                                     //60000, 0, 2000000,
                                     1536, 0, 1535);
    // Not Uploaded 
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc5hrEta13_16_[ch] = booker.book2D(Form("digiStrip_occ_5hr_ch%d_eta13_16", ch),
                                      Form("DigiStrip Occupancy Chamber %d Eta 13-16: 5 Hours;events;strip", ch),
                                      6000, 0, 2000000,
                                      //60000, 0, 2000000,
                                      1536, 0, 1535);
    // Not Uploaded 
    // 10 hour 
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc10hrEta1_4_[ch] = booker.book2D(Form("digiStrip_occ_10hr_ch%d_eta1_4", ch),
                                     Form("DigiStrip Occupancy Chamber %d Eta 1-4: 10 Hours;events;strip", ch),
                                     6000, 0, 3750000,
                                     //60000, 0, 3750000,
                                     1536, 0, 1535);

    // Not Uploaded
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc10hrEta5_8_[ch] = booker.book2D(Form("digiStrip_occ_10hr_ch%d_eta5_8", ch),
                                     Form("DigiStrip Occupancy Chamber %d Eta 5-8: 10 Hours;events;strip", ch),
                                     6000, 0, 3750000,
                                     //60000, 0, 3750000,
                                     1536, 0, 1535);
    // Not Uploaded
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc10hrEta9_12_[ch] = booker.book2D(Form("digiStrip_occ_10hr_ch%d_eta9_12", ch),
                                      Form("DigiStrip Occupancy Chamber %d Eta 9-12: 10 Hours;events;strip", ch),
                                      6000, 0, 3750000,
                                      //60000, 0, 3750000,
                                      1536, 0, 1535);
    // Not Uploaded 
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc10hrEta13_16_[ch] = booker.book2D(Form("digiStrip_occ_10hr_ch%d_eta13_16", ch),
                                       Form("DigiStrip Occupancy Chamber %d Eta 13-16: 10 Hours;events;strip", ch),
                                       6000, 0, 3750000,
                                       //60000, 0, 3750000,
                                       1536, 0, 1535);
    // Not Uploaded 
    // 24 hours 
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc24hrEta1_4_[ch] = booker.book2D(Form("digiStrip_occ_24hr_ch%d_eta1_4", ch),
                                     Form("DigiStrip Occupancy Chamber %d Eta 1-4: 24 Hours;events;strip", ch),
                                     6000, 0, 8750000,
                                     //90000, 0, 8750000,
                                     1536, 0, 1535);

    // Not Uploaded
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc24hrEta5_8_[ch] = booker.book2D(Form("digiStrip_occ_24hr_ch%d_eta5_8", ch),
                                     Form("DigiStrip Occupancy Chamber %d Eta 5-8: 24 Hours;events;strip", ch),
                                     6000, 0, 8750000,
                                     //90000, 0, 8750000,
                                     1536, 0, 1535);
    // Not Uploaded 
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc24hrEta9_12_[ch] = booker.book2D(Form("digiStrip_occ_24hr_ch%d_eta9_12", ch),
                                      Form("DigiStrip Occupancy Chamber %d Eta 9-12: 24 Hours;events;strip", ch),
                                      6000, 0, 8750000,
                                      //90000, 0, 8750000,
                                      1536, 0, 1535);
    // Not Uploaded 
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOcc24hrEta13_16_[ch] = booker.book2D(Form("digiStrip_occ_24hr_ch%d_eta13_16", ch),
                                       Form("DigiStrip Occupancy Chamber %d Eta 13-16: 24 Hours;events;strip", ch),
                                       6000, 0, 8750000,
                                       //90000, 0, 8750000,
                                       1536, 0, 1535);
    
  }
}

void QC8RecHitSource::analyze(edm::Event const &event, edm::EventSetup const &iSetup) {
  nEvent++;
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeom_);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<GEMDigiCollection> gemDigi;
  edm::Handle<GEMRecHitCollection> gemRecHit;

  event.getByToken(gemDigis_, gemDigi);
  event.getByToken(gemRecHits_, gemRecHit);

  if (!(gemDigi.isValid() && gemDigi.isValid())) {
    edm::LogWarning("QC8RecHitSource") << "Rechit sources from muonGEMDigis or gemRecHits are not found";
    return;
  }

  for (auto etaPart : GEMGeometry_->etaPartitions()) {
    auto gId = etaPart->id();
    auto ch = gId.chamber();
    auto ieta = gId.ieta();
    
    auto digiRange = gemDigi->get(gId);
    for (auto digi = digiRange.first; digi != digiRange.second; ++digi) {
      auto strip = digi->strip();
      mapDigiOcc_[ch]->Fill(strip, ieta);
     if (ieta >= 1 && ieta <= 4) {
        if (ieta == 2) {
          strip = strip + 384;
        }
        if (ieta == 3) {
          strip = strip + 768;
        }
        if (ieta == 4) {
          strip = strip + 1152;
        }
        mapDigiStripOcc1hrEta1_4_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc5hrEta1_4_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc10hrEta1_4_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc24hrEta1_4_[ch]->Fill(nEvent,strip);
      }
     if (ieta >= 5 && ieta <= 8) {
        if (ieta == 6) {
          strip = strip + 384;
        }
        if (ieta == 7) {
          strip = strip + 768;
        }
        if (ieta == 8) {
          strip = strip + 1152;
        }
        mapDigiStripOcc1hrEta5_8_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc5hrEta5_8_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc10hrEta5_8_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc24hrEta5_8_[ch]->Fill(nEvent,strip);
      }
     if (ieta >= 9 && ieta <= 12) {
        if (ieta == 10) {
          strip = strip + 384;
        }
        if (ieta == 11) {
          strip = strip + 768;
        }
        if (ieta == 12) {
          strip = strip + 1152;
        }
        mapDigiStripOcc1hrEta9_12_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc5hrEta9_12_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc10hrEta9_12_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc24hrEta9_12_[ch]->Fill(nEvent,strip);
      }
     if (ieta >= 13 && ieta <= 16) {
        if (ieta == 14) {
          strip = strip + 384;
        }
        if (ieta == 15) {
          strip = strip + 768;
        }
        if (ieta == 16) {
          strip = strip + 1152;
        }
        mapDigiStripOcc1hrEta13_16_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc5hrEta13_16_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc10hrEta13_16_[ch]->Fill(nEvent,strip);
        mapDigiStripOcc24hrEta13_16_[ch]->Fill(nEvent,strip);
      }
    } 
   auto rechitRange = gemRecHit->get(gId);
    for (auto rechit = rechitRange.first; rechit != rechitRange.second; ++rechit) {
      auto cls_size = rechit->clusterSize();
      auto gPos = etaPart->surface().toGlobal(rechit->localPosition());
      auto gX = gPos.x();
      auto gY = gPos.y();
      mapRecHitOcc_[ch]->Fill(gX, gY);
      if (cls_size > maxClsSizeToShow_) cls_size = maxClsSizeToShow_;
      mapRecHitClsSize_[ch]->Fill(cls_size, ieta);
    }
  }
}

DEFINE_FWK_MODULE(QC8RecHitSource);

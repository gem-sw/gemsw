#include "gemsw/DQM/plugins/MyQC8RecHitSource.h"
#include <ctime>

using namespace std;
using namespace edm;

int nEvent = 0;
MyQC8RecHitSource::MyQC8RecHitSource(const edm::ParameterSet &cfg)
  : hGEMGeom_(esConsumes()),
    hGEMGeomBeginRun_(esConsumes<edm::Transition::BeginRun>()) {
  gemRecHits_ = consumes<GEMRecHitCollection>(cfg.getParameter<edm::InputTag>("rechitInputLabel"));
  gemDigis_ = consumes<GEMDigiCollection>(cfg.getParameter<edm::InputTag>("digiInputLabel"));
}

void MyQC8RecHitSource::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
  edm::ParameterSetDescription desc;
  desc.add<edm::InputTag>("digiInputLabel", edm::InputTag("muonGEMDigis"));
  desc.add<edm::InputTag>("rechitInputLabel", edm::InputTag("gemRecHits"));
  descriptions.add("MyQC8RecHitSource", desc);
}

void MyQC8RecHitSource::bookHistograms(DQMStore::IBooker &booker, edm::Run const &, edm::EventSetup const &iSetup) {
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeomBeginRun_);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  for (auto chamber : GEMGeometry_->chambers()) {
    auto chamberId = chamber->id();
    auto ch = chamberId.chamber();
    auto etaPart = GEMGeometry_->etaPartitions();

    booker.setCurrentFolder("GE21QC8/Digi");
    mapDigiOcc_[ch] = booker.book2D(Form("digi_occ_ch%d", ch),
                                    Form("Digi Occupancy Chamber %d;strip;i#eta", ch),
                                    384, -0.5, 383.5,
                                    16, 0.5, 16.5);
    
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOccEta1_4_[ch] = booker.book2D(Form("digiStrip_occ_ch%d_eta1_4", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 1-4;events;strip", ch),
                                    100, 0, 8900,
                                    1536, 0, 1535);
        
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOccEta5_8_[ch] = booker.book2D(Form("digiStrip_occ_ch%d_eta5_8", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 5-8;events;strip", ch),
                                    100, 0, 8900,
                                    1536, 0, 1535);
    
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOccEta9_12_[ch] = booker.book2D(Form("digiStrip_occ_ch%d_eta9_12", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 9-12;events;strip", ch),
                                    100, 0, 8900,
                                    1536, 0, 1535);
        
    booker.setCurrentFolder("GE21QC8/DigiStrip");
    mapDigiStripOccEta13_16_[ch] = booker.book2D(Form("digiStrip_occ_ch%d_eta13_16", ch),
                                    Form("DigiStrip Occupancy Chamber %d Eta 13-16;events;strip", ch),
                                    100, 0, 8900,
                                    1536, 0, 1535);
    
    booker.setCurrentFolder("GE21QC8/RecHit");
    mapRecHitOcc_[ch] = booker.book2D(Form("rechit_occ_ch%d", ch),
                                      Form("Rechit Occupancy Chamber %d;x [cm]; y [cm]", ch),
                                      600, -60, 60,
                                      30, -70, 130);
    mapRecHitClsSize_[ch] = booker.book2D(Form("rechit_cls_size_ch%d", ch),
                                          Form("Rechit Cluster Size Chamber %d;Cluster Size;i#eta", ch),
                                          maxClsSizeToShow_, 0.5, maxClsSizeToShow_ + 0.5,
                                          16, 0.5, 16.5);
    booker.setCurrentFolder("GE21QC8/ChamberStatus");
    ChamberStatus_Ch[ch] = booker.book2D(Form("Ch%d_Status", ch),
                                    Form("Chamber %d Status;time;Status", ch),
                                    100, 0, 8900,
                                    2, 0, 2);
    
  }
}

void MyQC8RecHitSource::analyze(edm::Event const &event, edm::EventSetup const &iSetup) {
  nEvent++;
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeom_);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<GEMDigiCollection> gemDigi;
  edm::Handle<GEMRecHitCollection> gemRecHit;
  GEMDigi MyGemDigi;

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
        mapDigiStripOccEta1_4_[ch]->Fill(nEvent,strip);
      }
      else if (ieta >= 5 && ieta <= 8) {
        if (ieta == 6) {
          strip = strip + 384;
        }
        if (ieta == 7) {
          strip = strip + 768;
        }
        if (ieta == 8) {
          strip = strip + 1152;
        }
        mapDigiStripOccEta5_8_[ch]->Fill(nEvent,strip);
      }
      else if (ieta >= 9 && ieta <= 12) {
        if (ieta == 10) {
          strip = strip + 384;
        }
        if (ieta == 11) {
          strip = strip + 768;
        }
        if (ieta == 12) {
          strip = strip + 1152;
        }
        mapDigiStripOccEta9_12_[ch]->Fill(nEvent,strip);
      }
      else if (ieta >= 13 && ieta <= 16) {
        if (ieta == 14) {
          strip = strip + 384;
        }
        if (ieta == 15) {
          strip = strip + 768;
        }
        if (ieta == 16) {
          strip = strip + 1152;
        }
        mapDigiStripOccEta13_16_[ch]->Fill(nEvent,strip);
      }
      /*
      mapDigiStripOccEta13_16_[ch]->cd();
      int VFAT_Finder = 0*128;
      line = ROOT->TLine(0, VFAT_Finder + 128, 8900, VFAT_Finder + 128) // THIS IS HARDCODED - CHANGE
      VFAT_Finder++; 
      */
   }
    auto rechitRange = gemRecHit->get(gId);
    for (auto rechit = rechitRange.first; rechit != rechitRange.second; ++rechit) {
      auto cls_size = rechit->clusterSize();
      auto gPos = etaPart->surface().toGlobal(rechit->localPosition());
      auto gX = gPos.x();
      auto gY = gPos.y();
      mapRecHitOcc_[ch]->Fill(gX, gY);
      if (cls_size > maxClsSizeToShow_) cls_size = maxClsSizeToShow_;
    }
  }
  //std::cout << "Last event: " << nEvent << std::endl;
}

DEFINE_FWK_MODULE(MyQC8RecHitSource);

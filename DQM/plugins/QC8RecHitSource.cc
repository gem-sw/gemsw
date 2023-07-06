#include "gemsw/DQM/plugins/QC8RecHitSource.h"

using namespace std;
using namespace edm;

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

    booker.setCurrentFolder("GEM/Digi");
    mapDigiOcc_[ch] = booker.book2D(Form("digi_occ_ch%d", ch),
                                    Form("Digi Occupancy Chamber %d;strip;i#eta", ch),
                                    384, -0.5, 383.5,
                                    16, 0.5, 16.5);
    booker.setCurrentFolder("GEM/RecHit");
    mapRecHitOcc_[ch] = booker.book2D(Form("rechit_occ_ch%d", ch),
                                      Form("Rechit Occupancy Chamber %d;x [cm]; y [cm]", ch),
                                      600, -60, 60,
                                      30, -70, 130);
    mapRecHitClsSize_[ch] = booker.book2D(Form("rechit_cls_size_ch%d", ch),
                                          Form("Rechit Cluster Size Chamber %d;Cluster Size;i#eta", ch),
                                          maxClsSizeToShow_, 0.5, maxClsSizeToShow_ + 0.5,
                                          16, 0.5, 16.5);
  }
}

void QC8RecHitSource::analyze(edm::Event const &event, edm::EventSetup const &iSetup) {
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

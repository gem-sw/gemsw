#include "gemsw/Validation/plugins/QC8HitValidation.h"

QC8HitValidation::QC8HitValidation(const edm::ParameterSet& iConfig)
  : hGEMGeom_(esConsumes()),
    hGEMGeomBeginRun_(esConsumes<edm::Transition::BeginRun>())
{
  gemSimHits_ = consumes<edm::PSimHitContainer>(iConfig.getParameter<edm::InputTag>("gemSimHitLabel"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));
}

QC8HitValidation::~QC8HitValidation(){}

void QC8HitValidation::bookHistograms(DQMStore::IBooker& booker,
                                   const edm::Run& run,
                                   const edm::EventSetup& iSetup) {

  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeomBeginRun_);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  for (auto chamber : GEMGeometry_->chambers()) {
    auto chamberId = chamber->id();
    auto ch = chamberId.chamber();
    auto ly = chamberId.layer();

    booker.setCurrentFolder("GEM/QC8Hit/simhit");
    simhit_ch_occ_[ch] = booker.book2D(Form("simhit_occ_ch%d", ch),
                                       Form("simhit_occ_ch%d", ch),
                                       6, 0, 6,
                                       8, 1, 17);
    booker.setCurrentFolder("GEM/QC8Hit/rechit");
    rechit_ch_occ_[ch] = booker.book2D(Form("rechit_occ_ch%d", ch),
                                       Form("rechit_occ_ch%d", ch),
                                       6, 0, 6,
                                       8, 1, 17);
    setBinLabelAndTitle(simhit_ch_occ_[ch]->getTH2F());
    setBinLabelAndTitle(rechit_ch_occ_[ch]->getTH2F());
    for (int idx_mod = 0; idx_mod < 4; idx_mod++) {
      int module = idx_mod + (2 -ly) * 4 + 1;
      Key2 key(ch, module);
      booker.setCurrentFolder("GEM/QC8Hit/simhit");
      simhit_occ_[key] = booker.book2D(Form("simhit_occ_ch%d_module%d", ch, module),
                                       Form("simhit_occ_ch%d_module%d", ch, module),
                                       6, 0, 6,
                                       2, 0, 2);
      booker.setCurrentFolder("GEM/QC8Hit/rechit");
      rechit_occ_[key] = booker.book2D(Form("rechit_occ_ch%d_module%d", ch, module),
                                       Form("rechit_occ_ch%d_module%d", ch, module),
                                       6, 0, 6,
                                       2, 0, 2);
      setBinLabelAndTitle(simhit_occ_[key]->getTH2F(), module);
      setBinLabelAndTitle(rechit_occ_[key]->getTH2F(), module);
    }
  }
}

void QC8HitValidation::setBinLabelAndTitle(TH2F* hist, int module) {
  hist->GetXaxis()->SetTitle("ix");
  hist->GetYaxis()->SetTitle("i#eta");

  int nBinsx = hist->GetXaxis()->GetNbins();
  for (int i = 1; i <= nBinsx; i++) {
    hist->GetXaxis()->SetBinLabel(i, Form("%d", i));
  }
  int nBinsy = hist->GetYaxis()->GetNbins();
  for (int i = 1; i <= nBinsy; i++) {
    if (module == 0) hist->GetYaxis()->SetBinLabel(i, Form("%d & %d", i*2-1, i*2));
    else {
      int ieta_offset = (8 - module) % 4 * 4;
      int ieta_start = ieta_offset + (i-1) * 2 + 1;
      int ieta_end = ieta_start + 1;
      hist->GetYaxis()->SetBinLabel(i, Form("%d & %d", ieta_start, ieta_end));
    }
  }
}

void QC8HitValidation::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeom_);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<edm::PSimHitContainer> gemSimHits;
  iEvent.getByToken(gemSimHits_, gemSimHits);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_,gemRecHits);

  for (const auto& simhit : *gemSimHits.product()) {
    const GEMDetId gemId(simhit.detUnitId());
    auto etaPart = GEMGeometry_->etaPartition(gemId);

    auto ch = gemId.chamber();
    auto ly = gemId.layer();
    auto ieta = gemId.ieta();

    auto strip = int(etaPart->strip(simhit.localPosition()));
    int module = (16-ieta)/4 + 1 + (2-ly)*4;
    int sector = 1 - ((16-ieta) % 4 / 2);

    Key2 key(ch, module);

    simhit_occ_[key]->Fill(strip / 64, sector);
    simhit_ch_occ_[ch]->Fill(strip / 64, ieta);

    auto range = gemRecHits->get(gemId);
    for (auto rechit = range.first; rechit != range.second; ++rechit) {
      if (matchRecHitAgainstSimHit(rechit, strip)) {
        rechit_occ_[key]->Fill(strip / 64, sector);
        rechit_ch_occ_[ch]->Fill(strip / 64, ieta);
        break;
      }
    }
  }
}

bool QC8HitValidation::matchRecHitAgainstSimHit(GEMRecHitCollection::const_iterator rechit, int simhit_strip) {
  int cls = rechit->clusterSize();
  int rechit_first_strip = rechit->firstClusterStrip();

  if (cls == 1) {
    return simhit_strip == rechit_first_strip;
  } else {
    int rechit_last_strip = rechit_first_strip + cls - 1;
    return (simhit_strip >= rechit_first_strip) and (simhit_strip <= rechit_last_strip);
  }
}

//define this as a plug-in
DEFINE_FWK_MODULE(QC8HitValidation);

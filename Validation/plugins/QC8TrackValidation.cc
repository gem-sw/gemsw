#include "gemsw/Validation/plugins/QC8TrackValidation.h"

QC8TrackValidation::QC8TrackValidation(const edm::ParameterSet& iConfig)
  : hGEMGeom_(esConsumes()),
    hGEMGeomBeginRun_(esConsumes<edm::Transition::BeginRun>())
{
  tracks_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("tracks"));
  trajs_ = consumes<vector<Trajectory>>(iConfig.getParameter<edm::InputTag>("trajs"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));
  edm::ParameterSet serviceParameters = iConfig.getParameter<edm::ParameterSet>("ServiceParameters");
}

QC8TrackValidation::~QC8TrackValidation(){}

void QC8TrackValidation::bookHistograms(DQMStore::IBooker& booker,
                                   const edm::Run& run,
                                   const edm::EventSetup& iSetup) {

  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeomBeginRun_);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  booker.setCurrentFolder("GEM/QC8Track");
  trackChi2_ = booker.book1D("track_chi2", "Normalized Track Chi2", 100, 0, 10);

  for (auto chamber : GEMGeometry_->chambers()) {
    auto chamberId = chamber->id();
    auto ch = chamberId.chamber();
    auto ly = chamberId.layer();

    booker.setCurrentFolder("GEM/QC8Track/track");
    track_ch_occ_[ch] = booker.book2D(Form("track_occ_ch%d", ch),
                                      Form("track_occ_ch%d", ch),
                                      6, 0, 6,
                                      8, 1, 17);
    booker.setCurrentFolder("GEM/QC8Track/rechit");
    rechit_ch_occ_[ch] = booker.book2D(Form("rechit_occ_ch%d", ch),
                                       Form("rechit_occ_ch%d", ch),
                                       6, 0, 6,
                                       8, 1, 17);
    setBinLabelAndTitle(track_ch_occ_[ch]->getTH2F());
    setBinLabelAndTitle(rechit_ch_occ_[ch]->getTH2F());
    for (int idx_mod = 0; idx_mod < 4; idx_mod++) {
      int module = idx_mod + (2 -ly) * 4 + 1;
      Key2 key(ch, module);
      booker.setCurrentFolder("GEM/QC8Track/track");
      track_occ_[key] = booker.book2D(Form("track_occ_ch%d_module%d", ch, module),
                                      Form("track_occ_ch%d_module%d", ch, module),
                                      6, 0, 6,
                                      2, 0, 2);
      booker.setCurrentFolder("GEM/QC8Track/rechit");
      rechit_occ_[key] = booker.book2D(Form("rechit_occ_ch%d_module%d", ch, module),
                                       Form("rechit_occ_ch%d_module%d", ch, module),
                                       6, 0, 6,
                                       2, 0, 2);
      setBinLabelAndTitle(track_occ_[key]->getTH2F(), module);
      setBinLabelAndTitle(rechit_occ_[key]->getTH2F(), module);
    }
    for (int ieta = 1; ieta < 17; ieta++) {
      booker.setCurrentFolder("GEM/QC8Track/residual");
      Key2 key(ch, ieta);
      residual_[key] = booker.book1D(Form("track_residual_ch%d_ieta%d", ch, ieta),
                                     Form("track_residual_ch%d_ieta%d", ch, ieta),
                                     100, -200, 200);
    }
  }
}

void QC8TrackValidation::setBinLabelAndTitle(TH2F* hist, int module) {
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

void QC8TrackValidation::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  /* GEM Geometry */
  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeom_);
//  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<vector<reco::Track>> tracks;
  iEvent.getByToken(tracks_, tracks);

  edm::Handle<vector<Trajectory>> trajs;
  iEvent.getByToken(trajs_, trajs);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_,gemRecHits);

  std::map<GEMDetId, TrajectoryStateOnSurface> tsosMap;
  for (std::vector<reco::Track>::const_iterator track = tracks->begin(); track != tracks->end(); ++track)
  {
    trackChi2_->Fill(track->normalizedChi2());
    auto traj = trajs->begin();
    for (auto trajMeas : traj->measurements()) {
      auto tsos = trajMeas.predictedState();
      auto rechit = trajMeas.recHit();
      auto gemId = GEMDetId(rechit->geographicalId());
      tsosMap[gemId] = tsos;
    }

    for (auto hit : track->recHits()) {
      auto etaPart = GEMGeometry_->etaPartition(hit->geographicalId());
      auto etaPartId = etaPart->id();

      if (tsosMap.find(etaPartId) == tsosMap.end()) continue;
      auto tsos = tsosMap[etaPartId];

      auto lp_track = tsos.localPosition();

      auto ch = etaPartId.chamber();
      auto ly = etaPartId.layer();
      auto ieta = etaPartId.ieta();

      auto strip = int(etaPart->strip(lp_track));
      int module = (16-ieta)/4 + 1 + (2-ly)*4;
      int sector = 1 - ((16-ieta) % 4 / 2);

      Key2 key(ch, module);
      Key2 etaKey(ch, ieta);

      track_occ_[key]->Fill(strip / 64, sector);
      track_ch_occ_[ch]->Fill(strip / 64, ieta);

      if (!hit->isValid()) continue;

      rechit_occ_[key]->Fill(strip / 64, sector);
      rechit_ch_occ_[ch]->Fill(strip / 64, ieta);

      auto range = gemRecHits->get(etaPartId);
      float residual = FLT_MAX;
      for (auto rechit = range.first; rechit != range.second; ++rechit) {
        auto rechitStrip = etaPart->strip(rechit->localPosition());
        if (abs(residual) > abs(strip - rechitStrip)) {
          residual = (strip - rechitStrip);
        }
      }
      residual_[etaKey]->Fill(residual);
    }
  }
}

std::pair<int, int> QC8TrackValidation::getModuleVfat(int ieta, int strip) {
  ieta = 16 - ieta;
  int module = ieta / 4 + 1;
  int sector = (ieta % 4) / 2;
  int iphi = strip / 64;
  int vfat = iphi * 2 + sector;
  return std::make_pair(module, vfat);
}

//define this as a plug-in
DEFINE_FWK_MODULE(QC8TrackValidation);

#include "gemsw/Validation/plugins/QC8Harvestor.h"
#include "FWCore/Framework/interface/MakerMacros.h"

QC8Harvestor::QC8Harvestor(const edm::ParameterSet& pset)
 : MuonGEMBaseHarvestor(pset, "QC8Harvestor") {
 isMC_ = pset.getParameter<bool>("isMC");
}

QC8Harvestor::~QC8Harvestor() {}

void QC8Harvestor::fillDescriptions(edm::ConfigurationDescriptions &descriptions) {
  edm::ParameterSetDescription desc;

  desc.add<bool>("isMC", false);

  descriptions.add("QC8DQMHarvestor", desc);
}

void QC8Harvestor::dqmEndJob(DQMStore::IBooker& booker, DQMStore::IGetter& getter) {
  {
    TString track_path = "GEM/QC8Track/track/";
    TString rechit_path = "GEM/QC8Track/rechit/";
    TString eff_path = "GEM/QC8Track/efficiency/";
    for (int i = 0; i < 7; i++) {
      int ch_num = i+1;
      TString track_histName_ch = track_path + Form("track_occ_ch%d", ch_num);
      TString rechit_histName_ch = rechit_path + Form("rechit_occ_ch%d", ch_num);

      TString eff_name_ch = Form("efficiency_ch%d", ch_num);
      TString eff_title_ch = Form("efficiency ch%d", ch_num);

      bookEff2D(booker, getter, rechit_histName_ch, track_histName_ch, eff_path, eff_name_ch, eff_title_ch);

      for (int j = 0; j < 4; j++) {
        int module_num = 5+j;
        TString track_histName_module = track_histName_ch + Form("_module%d", module_num);
        TString rechit_histName_module = rechit_histName_ch + Form("_module%d", module_num);

        TString eff_name_module = Form("efficiency_ch%d_module%d", ch_num, module_num);
        TString eff_title_module = Form("efficiency ch%d module%d", ch_num, module_num);

        bookEff2D(booker, getter, rechit_histName_module, track_histName_module, eff_path, eff_name_module, eff_title_module);
      }
    }
  }
  if (isMC_) {
    TString simhit_path = "GEM/QC8Hit/simhit/";
    TString rechit_path = "GEM/QC8Hit/rechit/";
    TString eff_path = "GEM/QC8Hit/efficiency/";
    for (int i = 0; i < 7; i++) {
      int ch_num = i+1;
      TString simhit_histName_ch = simhit_path + Form("simhit_occ_ch%d", ch_num);
      TString rechit_histName_ch = rechit_path + Form("rechit_occ_ch%d", ch_num);

      TString eff_name_ch = Form("efficiency_ch%d", ch_num);
      TString eff_title_ch = Form("efficiency ch%d", ch_num);

      bookEff2D(booker, getter, rechit_histName_ch, simhit_histName_ch, eff_path, eff_name_ch, eff_title_ch);

      for (int j = 0; j < 4; j++) {
        int module_num = 5+j;
        TString simhit_histName_module = simhit_histName_ch + Form("_module%d", module_num);
        TString rechit_histName_module = rechit_histName_ch + Form("_module%d", module_num);

        TString eff_name_module = Form("efficiency_ch%d_module%d", ch_num, module_num);
        TString eff_title_module = Form("efficiency ch%d module%d", ch_num, module_num);

        bookEff2D(booker, getter, rechit_histName_module, simhit_histName_module, eff_path, eff_name_module, eff_title_module);
      }
    }
  }
}
//define this as a plug-in
DEFINE_FWK_MODULE(QC8Harvestor);

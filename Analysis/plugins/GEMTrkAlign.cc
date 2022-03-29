// -*- C++ -*-
//
// Package:    TB2021/GEMTrkAlign
// Class:      GEMTrkAlign
//
/**\class GEMTrkAlign GEMTrkAlign.cc TB2021/GEMTrkAlign/plugins/GEMTrkAlign.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  mmaggi
//         Created:  Fri, 28 Jan 2022 13:31:05 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

// GEM
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"

// Root                                                                                                                                                                                
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH2F.h"
//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

                                                                                                                                                                               

typedef std::pair<int, std::string> TrChView;
typedef std::pair<int, TrChView> ClsTrChView;

class GEMTrkAlign : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit GEMTrkAlign(const edm::ParameterSet&);
  ~GEMTrkAlign();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

private:
  void Fill(const GEMRecHitCollection& rhs, const GEMRecHitCollection& exs);

  // ----------member data ---------------------------

  edm::EDGetTokenT<GEMRecHitCollection> gemrhToken_;  
  edm::EDGetTokenT<GEMRecHitCollection> exge21Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt01Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt02Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt03Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt04Token_; 

private:
  std::map<TrChView, TH1F*> h_res,h_rec,h_pre;
  std::map<TrChView, TH2F*> h_resVspre;
  std::map<TrChView, TH2F*> h_resVspres;
  std::map<ClsTrChView, TH1F*> h_res_cl,h_rec_cl,h_pre_cl;

};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
GEMTrkAlign::GEMTrkAlign(const edm::ParameterSet& iConfig) {

  gemrhToken_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("gemRecHitLabel"));
  exge21Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackge21"));
  exgt01Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt01"));
  exgt02Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt02"));
  exgt03Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt03"));
  exgt04Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt04"));

  edm::Service<TFileService> fs;
  std::vector<std::string> sv;
  sv.push_back("x");
  sv.push_back("y");
  for (unsigned int ch=1; ch<=4; ch++) {
    for (std::vector<std::string>::iterator iv=sv.begin(); iv < sv.end(); iv++){
      TrChView view(ch,*iv);
      std::stringstream title1, shtit1;
      std::stringstream title2, shtit2;
      std::stringstream title3, shtit3;
      title1 << "Residuals for Tracker Ch " << view.first
             << " view " << view.second;
      shtit1 << view.second << "resCh" << view.first;
      h_res[view] = fs->make<TH1F>(shtit1.str().c_str(), title1.str().c_str(), 2000, -1., 1.);
      std::string sh2d = shtit1.str()+"VsExtr";
      std::string ti2d = " Extr Vs "+title1.str();
      h_resVspre[view] = fs->make<TH2F>(sh2d.c_str(), ti2d.c_str(), 2000, -1., 1., 48, -6., 6);
      std::string sh2ds = shtit1.str()+"VsExtrS";
      std::string ti2ds = " ExtrS Vs "+title1.str();
      h_resVspres[view] = fs->make<TH2F>(sh2ds.c_str(), ti2ds.c_str(), 2000, -1., 1., 48, -6., 6);

      title2 << " Rechit position for Tracker Ch " << view.first
             << " view " << view.second;
      shtit2 << view.second << "recCh" << view.first;
      h_rec[view] = fs->make<TH1F>(shtit2.str().c_str(), title2.str().c_str(), 1920, -6., 6.);
      title3 << " Extrapolated position for Tracker Ch " << view.first
             << " view " << view.second;
      shtit3 << view.second << "extCh" << view.first;
      h_pre[view] = fs->make<TH1F>(shtit3.str().c_str(), title3.str().c_str(), 12000, -6., 6.);
      for (unsigned int cl = 1; cl <= 5; cl++) {
        ClsTrChView clv(cl,view);
        std::stringstream titlecl1,shtitcl1;
        std::stringstream titlecl2,shtitcl2;
        std::stringstream titlecl3,shtitcl3;
        titlecl1 << title1.str() << " cl size = " << cl;
        shtitcl1 << shtit1.str() << "cls" << cl;
        h_res_cl[clv] = fs->make<TH1F>(shtitcl1.str().c_str(), titlecl1.str().c_str(), 2000, -1., 1.);
        titlecl2 << title2.str() << " cl size = " << cl;
        shtitcl2 << shtit2.str() << "cls" << cl;
        h_rec_cl[clv] = fs->make<TH1F>(shtitcl2.str().c_str(), titlecl2.str().c_str(), 2000, -6., 6.);
        titlecl3 << title3.str() << " cl size = " << cl;
        shtitcl3 << shtit3.str() << "cls" << cl;
        h_pre_cl[clv] = fs->make<TH1F>(shtitcl3.str().c_str(), titlecl3.str().c_str(), 2000, -6., 6.);
      }
    }
  }
}

GEMTrkAlign::~GEMTrkAlign() {

}
//
// member functions
//

// ------------ method called for each event  ------------
void GEMTrkAlign::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

  //  std::cout <<" GEtting therechits"<<std::endl;
  GEMRecHitCollection rhs = iEvent.get(gemrhToken_);
  //std::cout <<" GEtting the G21 extr rechits"<<std::endl;
  GEMRecHitCollection exge21 = iEvent.get(exge21Token_);
  //std::cout <<" GEtting the BA001 extr rechits"<<std::endl;
  GEMRecHitCollection exgt01 = iEvent.get(exgt01Token_);
  //std::cout <<" GEtting the BA002 extr rechits"<<std::endl;
  GEMRecHitCollection exgt02 = iEvent.get(exgt02Token_);
  // std::cout <<" GEtting the BA003 extr rechits"<<std::endl;
  GEMRecHitCollection exgt03 = iEvent.get(exgt03Token_);
  //std::cout <<" GEtting the BA003 extr rechits"<<std::endl;
  GEMRecHitCollection exgt04 = iEvent.get(exgt04Token_);

//start with G21 which can get the track extrapolated compatible with two eta partitions
//  unsigned int NetaGE21 = exge21.id_size();

  // Track extrapolation to TrackerChamber 
  //BARI01
  this->Fill(rhs,exgt01);

  //BARI02
  this->Fill(rhs,exgt02);

  //BARI03
  this->Fill(rhs,exgt03);

  //BARI04
  this->Fill(rhs,exgt04);
  
}

void GEMTrkAlign::Fill(const GEMRecHitCollection& rhs, const GEMRecHitCollection& exgt) {

  unsigned int nck_ba = exgt.id_size();
  //must be 1 (maybe 0 if completely misaligned)                                                                                                                                      
  if (nck_ba == 1) {
    auto trx_id = *exgt.id_begin();
    GEMDetId try_id(trx_id.region(), trx_id.ring(), trx_id.station(), trx_id.layer(), trx_id.chamber(), trx_id.ieta()+1);
    int TrChNo = (trx_id.chamber()/2-1)*2 + trx_id.ieta()/2 + trx_id.ieta()%2;
    //    std::cout <<" Tracking Chanmber BA00"<<TrChNo<<std::endl;
    auto rhsx = rhs.get(trx_id);
    auto rhsy = rhs.get(try_id);
    unsigned int nclsx = 0;
    unsigned int nclsy = 0;
    for (auto ix = rhsx.first; ix != rhsx.second; ++ix) {
      nclsx++;
    }
    for (auto iy = rhsy.first; iy != rhsy.second; ++iy) {
      nclsy++;
    }
    auto ehs = exgt.get(trx_id);
    unsigned int nexp = 0;
    for (auto ex = ehs.first; ex != ehs.second; ++ex) {
      nexp++;
    }
    if (nclsx == 1 && nexp == 1) {
      TrChView vw(TrChNo, "x");
      auto lp_rh = rhsx.first->localPosition();
      auto lp_ex = ehs.first->localPosition();

      h_res[vw]->Fill(lp_rh.x()-lp_ex.x());
      h_resVspre[vw]->Fill(lp_rh.x()-lp_ex.x(), lp_ex.y());
      h_resVspres[vw]->Fill(lp_rh.x()-lp_ex.x(), lp_ex.x());
      h_rec[vw]->Fill(lp_rh.x());
      h_pre[vw]->Fill(lp_ex.x());

      int cls = rhsx.first->clusterSize();
      ClsTrChView clv(cls,vw);
      
      h_res_cl[clv]->Fill(lp_rh.x()-lp_ex.x());
      h_rec_cl[clv]->Fill(lp_rh.x());
      h_pre_cl[clv]->Fill(lp_ex.x());
    }
    if (nclsy == 1 && nexp == 1) {
      TrChView vw(TrChNo, "y");
      auto lp_rh = rhsy.first->localPosition();
      auto lp_ex = ehs.first->localPosition();
      //      std::cout <<" +++ lp rechit in y "<<lp_rh<<" +++ lp extrapolate "<<lp_ex<<std::endl;

      h_res[vw]->Fill(lp_rh.x()+lp_ex.y());
      h_resVspre[vw]->Fill(lp_rh.x()+lp_ex.y(), lp_ex.x());
      h_resVspres[vw]->Fill(lp_rh.x()+lp_ex.y(), -lp_ex.y());
      h_rec[vw]->Fill(lp_rh.x());
      h_pre[vw]->Fill(-lp_ex.y());

      int cls = rhsy.first->clusterSize();
      ClsTrChView clv(cls,vw);

      h_res_cl[clv]->Fill(lp_rh.x()+lp_ex.y());
      h_rec_cl[clv]->Fill(lp_rh.x());
      h_pre_cl[clv]->Fill(-lp_ex.y());
    }
  }
}
// ------------ method called once each job just before starting event loop  ------------
void GEMTrkAlign::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void GEMTrkAlign::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void GEMTrkAlign::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addWithDefaultLabel(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(GEMTrkAlign);

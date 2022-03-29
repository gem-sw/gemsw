// -*- C++ -*-
//
// Package:    TB2021/GEMRot
// Class:      GEMRot
//
/**\class GEMRot GEMRot.cc TB2021/GEMRot/plugins/GEMRot.cc

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
typedef std::pair<int, TrChView> RotTrChView;

class GEMRot : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit GEMRot(const edm::ParameterSet&);
  ~GEMRot();

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
  std::map<TrChView, TH1F*> h_res;
  std::map<TrChView, TH2F*> h_resVspre;
  std::map<RotTrChView, TH1F*> h_res_rot;
  std::map<RotTrChView, TH2F*> h_resVspre_r;

private:
  double th_c;
  double th_a;
  int nbin;
  float th_init;
  double th_bin;
};

//
// Constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
GEMRot::GEMRot(const edm::ParameterSet& iConfig){

  gemrhToken_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("gemRecHitLabel"));
  exge21Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackge21"));
  exgt01Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt01"));
  exgt02Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt02"));
  exgt03Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt03"));
  exgt04Token_ = consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt04"));

  th_c = iConfig.getUntrackedParameter<double>("thetaCentral");
  th_a = iConfig.getUntrackedParameter<double>("thetaRange");
  nbin = iConfig.getUntrackedParameter<int>("thetaNPoints");
  th_init = th_c-th_a/2.;
  th_bin = th_a/float(nbin-1);
  std::cout << "Rotation Analysis " << std::endl;
  for (int ir = 0; ir < nbin; ir++) {
    float theta = th_init+ir*th_bin;
    std::cout << "Rotation " << std::setw(3) << ir << " is " << theta*1000 << " mrad" << std::endl;
  }


  edm::Service<TFileService> fs;
  std::vector<std::string> sv;
  sv.push_back("x");
  sv.push_back("y");
  for (unsigned int ch = 1; ch <= 4; ch++) {
    for (std::vector<std::string>::iterator iv=sv.begin(); iv < sv.end(); iv++){
      TrChView view(ch, *iv);
      std::stringstream  title1, shtit1;
      title1 << "Residuals for Tracker Ch " << view.first
             << " view " << view.second;
      shtit1 << view.second << "resCh" << view.first;
      h_res[view] = fs->make<TH1F>(shtit1.str().c_str(), title1.str().c_str(), 200, -0.1, 0.1);
      std::string sh2d = shtit1.str()+"VsExtr";
      std::string ti2d = " Extr Vs "+title1.str();
      h_resVspre[view] = fs->make<TH2F>(sh2d.c_str(), ti2d.c_str(), 200, -0.1, 0.1, 48, -6., 6);
      
      for (int ir = 0;ir <nbin; ir++) {
        std::stringstream rtit, rsht;
        RotTrChView rview(ir,view);
        rtit << "Residual Rotation Step " << std::setw(3) << ir<<" Ch " << view.first << " view " << view.second;
        rsht << view.second << "ResRotSte" << std::setw(3) << std::setfill('0') << ir << "_" << view.first;
        h_res_rot[rview] = fs->make<TH1F>(rsht.str().c_str(), rtit.str().c_str(), 200, -0.1, 0.1);
        std::string shr = rsht.str()+"VsExtr";
        std::string rti = " Extr Vs "+rtit.str();
        h_resVspre_r[rview] = fs->make<TH2F>(shr.c_str(), rti.c_str(), 200, -0.1, 0.1, 48, -6., 6);
      }

    }
  }
}

GEMRot::~GEMRot() {
  std::vector<std::string> sv;
  sv.push_back("x");
  sv.push_back("y");

  std::cout << "\n----------------------------------------" << std::endl;
  for (unsigned int ch = 1; ch <= 4; ch++) {
    std::cout << "|* BARI00" << ch << "     X       Y    view" << std::endl;
    TrChView view1(ch, "x");
    TrChView view2(ch, "y");
    for (int ir = 0;ir < nbin; ir++) {
      RotTrChView rview1(ir, view1);
      RotTrChView rview2(ir, view2);
      double theta = th_init+ir*th_bin;
      double rmsx=h_res_rot[rview1]->GetStdDev();
      double rmsy=h_res_rot[rview2]->GetStdDev();
      std::cout << " Theta " << theta*1000 << " mrad   Sigma x = " << rmsx*10000 << " y = " << rmsy*10000 << std::endl;      
    }
  }
}
//
// member functions
//

// ------------ method called for each event  ------------
void GEMRot::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

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

void GEMRot::Fill(const GEMRecHitCollection& rhs, const GEMRecHitCollection& exgt){

  unsigned int nck_ba = exgt.id_size();
  //must be 1 (maybe 0 if completely misaligned)                                                                                                                                      
  if (nck_ba == 1){
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

    // rotate the extraopilated point 
    //  x_n = x*cos(theta) -y*sin(theta)
    //  y_n = x*sin(theta) +y*cos(theta)
    if (nclsx == 1 && nexp == 1) {
      TrChView vw(TrChNo, "x");
      auto lp_rh = rhsx.first->localPosition();
      auto lp_ex = ehs.first->localPosition();

      h_res[vw]->Fill(lp_rh.x()-lp_ex.x());
      h_resVspre[vw]->Fill(lp_rh.x()-lp_ex.x(), lp_ex.y());

      for (int ir = 0; ir < nbin; ir++) {
        RotTrChView rtv(ir,vw);
        double theta = th_init+ir*th_bin;
        double x_n = lp_ex.x()*cos(theta)-lp_ex.y()*sin(theta);
        double y_n = lp_ex.x()*sin(theta)+lp_ex.y()*cos(theta);

        h_res_rot[rtv]->Fill(lp_rh.x()-x_n);
        h_resVspre_r[rtv]->Fill(lp_rh.x()-x_n, y_n);
      }
    }
    if (nclsy == 1 && nexp == 1) {
      TrChView vw(TrChNo, "y");
      auto lp_rh = rhsy.first->localPosition();
      auto lp_ex = ehs.first->localPosition();

      h_res[vw]->Fill(lp_rh.x()+lp_ex.y());
      h_resVspre[vw]->Fill(lp_rh.x()+lp_ex.y(), lp_ex.x());

      for (int ir = 0; ir < nbin; ir++) {
        RotTrChView rtv(ir, vw);
        double theta = th_init+ir*th_bin;
        double x_n = lp_ex.x()*cos(theta)-lp_ex.y()*sin(theta);
        double y_n = lp_ex.x()*sin(theta)+lp_ex.y()*cos(theta);

        h_res_rot[rtv]->Fill(lp_rh.x()+y_n);
        h_resVspre_r[rtv]->Fill(lp_rh.x()+y_n, x_n);
      }
    }
  }
}
// ------------ method called once each job just before starting event loop  ------------
void GEMRot::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void GEMRot::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void GEMRot::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
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
DEFINE_FWK_MODULE(GEMRot);

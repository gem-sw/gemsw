#ifndef GEMRot_H
#define GEMRot_H
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
#include <Geometry/Records/interface/MuonGeometryRecord.h>
#include <Geometry/GEMGeometry/interface/GEMGeometry.h>

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
  void Analyse21(const GEMRecHitCollection& rhs, const GEMRecHitCollection& exs);

  // ----------member data ---------------------------

  edm::EDGetTokenT<GEMRecHitCollection> gemrhToken_;  
  edm::EDGetTokenT<GEMRecHitCollection> exge21Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt01Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt02Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt03Token_; 
  edm::EDGetTokenT<GEMRecHitCollection> exgt04Token_; 

private:
  const GEMGeometry* mgeom;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> geometry_token_;
private:
  std::map<TrChView, TH1F*> h_res;
  std::map<TrChView, TH2F*> h_resVspre;
  TH2F* h21_yInd1;
  TH2F* h21_yInd2;
  float xoff;
  float offset;
  float offset14;
  float offset15;
  TH2F* h21_yInd; 
  TH1F* h21_yInd_den;
  TH1F* h21_yInd_eff;
  TH1F* h21_yInd_and;
  TH1F* h21_yInd_x1Only;
  TH1F* h21_yInd_x2Only;
  TH1F* h21_yInd_x1Onlyck;
  TH1F* h21_yInd_x2Onlyck;
  std::map<RotTrChView, TH1F*> h_res_rot;
  std::map<RotTrChView, TH2F*> h_resVspre_r;
  std::map<unsigned int, TH1F*> h21_res;
  std::map<unsigned int, TH1F*> h21_res_str;
  std::map<unsigned int, TH1F*> h21_res_str_ap;
  std::map<unsigned int, TH1F*> h21_res_str_vp;
  TH1F* h21_rest;
  std::map<unsigned int, TH2F*> h21_resVspre;
  std::map<unsigned int, TH2F*> h21_resVspre_str;
  std::map<unsigned int, TH2F*> h21_resVspre_cstr;
  std::map<unsigned int, TH2F*> h21_resVspre_str_ap;
  std::map<unsigned int, TH2F*> h21_resVspre_str_vp;

private:
  double th_c;
  double th_a;
  int nbin;
  float th_init;
  double th_bin;
  float miny,maxy;

};

#endif

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
  geometry_token_ = esConsumes<GEMGeometry, MuonGeometryRecord>();

  miny=0;
  maxy=0;
  gemrhToken_=consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("gemRecHitLabel"));
  exge21Token_=consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackge21"));
  exgt01Token_=consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt01"));
  exgt02Token_=consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt02"));
  exgt03Token_=consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt03"));
  exgt04Token_=consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("extrackgt04"));
  th_c=iConfig.getUntrackedParameter<double>("thetaCentral");
  th_a=iConfig.getUntrackedParameter<double>("thetaRange");
  nbin=iConfig.getUntrackedParameter<int>("thetaNPoints");
  th_init=th_c-th_a/2.;
  th_bin=th_a/float(nbin-1);
  std::cout <<"Rotation Analysis "<<std::endl;
  for (int ir=0;ir<nbin;ir++){
    float theta = th_init+ir*th_bin;
    std::cout <<"Rotation "<<std::setw(3)<<ir<<" is "<<theta*1000<<" mrad"<<std::endl;
  }
  edm::Service<TFileService> fs;

  h21_yInd1 = fs->make<TH2F>("GE21yInd1","GE21 y1 Vs induction",500,-3.5,3.5,1000,-6.,-4.5);
  h21_yInd2 = fs->make<TH2F>("GE21yInd2","GE21 y2 Vs induction",500,-3.5,3.5,1000,4.5,6.);
  h21_yInd = fs->make<TH2F>("GE21yInd","GE21 y Vs induction",500,-3.5,3.5,1000,-2.5,2.5);
  xoff = -0.7107;
  offset = 0.2385;
  offset14 =  5.395;
  offset15 = -5.395;
  h21_yInd_den    = fs->make<TH1F>("GE21yIndDen","GE21 extrapolated y all tracks",1000,-5.0,5.0);
  h21_yInd_eff    = fs->make<TH1F>("GE21yIndOr" ,"GE21 extrapolated y partition fired 14 or 15",1000,-5.0,5.0);
  h21_yInd_and    = fs->make<TH1F>("GE21yIndAnd","GE21 extrapolated y partition fired 14 and 15",1000,-5.0,5.0);
  h21_yInd_x1Only = fs->make<TH1F>("GE21yIndx1" ,"GE21 extrapolated y Vs partition fired 14 only",1000,-5.0,5.0);
  h21_yInd_x2Only = fs->make<TH1F>("GE21yIndx2" ,"GE21 extrapolated y Vs partition fired 15 only",1000,-5.0,5.0);
  h21_yInd_x1Onlyck = fs->make<TH1F>("GE21yIndx1ck" ,"GE21 extrapolated y Vs partition fired 14 only",1000,-5.0,5.0);
  h21_yInd_x2Onlyck = fs->make<TH1F>("GE21yIndx2ck" ,"GE21 extrapolated y Vs partition fired 15 only",1000,-5.0,5.0);
  h21_rest = fs->make<TH1F>("Residuals for GE21","ResGE21",500,-5.,5.);
  for (unsigned int ip=0;ip<2;ip++){
    unsigned int partition=14+ip;
    std::stringstream ti21,st21,ti21s1,st21s1,ti21s2,st21s2,ti21s3,st21s3;
    ti21<<"x Residuals for GE21 partition "<<partition;
    st21<<"ResGE21p"<<partition;
    h21_res[partition] = fs->make<TH1F>(st21.str().c_str(),ti21.str().c_str(),500,-5.,5.);
    ti21s1<<"x Residuals in strip units for GE21 partition "<<partition;
    st21s1<<"ResGE21pstr"<<partition;
    h21_res_str[partition] = fs->make<TH1F>(st21s1.str().c_str(),ti21s1.str().c_str(),500,-50.,50.);
    ti21s2<<"x Residuals in strip units av pitch for GE21 partition "<<partition;
    st21s2<<"ResGE21pstrap"<<partition;
    h21_res_str_ap[partition] = fs->make<TH1F>(st21s2.str().c_str(),ti21s2.str().c_str(),500,-5.,5.);
    ti21s3<<"x Residuals in strip units lo pitch for GE21 partition "<<partition;
    st21s3<<"ResGE21pstrvp"<<partition;
    h21_res_str_vp[partition] = fs->make<TH1F>(st21s3.str().c_str(),ti21s3.str().c_str(),500,-5.,5.);

    std::string st212d=st21.str()+"VsExtr";
    std::string ti212d=" Extr Vs "+ti21.str();
    std::string st212dc1=st21s1.str()+"VsExtrmod";
    std::string ti212dc1=" Extr Vs mod "+ti21s1.str();
    std::string st212ds1=st21s1.str()+"VsExtr";
    std::string ti212ds1=" Extr Vs "+ti21s1.str();
    std::string st212ds2=st21s2.str()+"VsExtr";
    std::string ti212ds2=" Extr Vs "+ti21s2.str();
    std::string st212ds3=st21s3.str()+"VsExtr";
    std::string ti212ds3=" Extr Vs "+ti21s3.str();
    h21_resVspre[partition] = fs->make<TH2F>(st212d.c_str(),ti212d.c_str(),500,-5.,5.,200,-12.,12.);
    h21_resVspre_str[partition] = fs->make<TH2F>(st212ds1.c_str(),ti212ds1.c_str(),500,-50.,50.,200,-12.,12.);
    h21_resVspre_cstr[partition] = fs->make<TH2F>(st212dc1.c_str(),ti212dc1.c_str(),500,-50.,50.,200,-12.,12.);
    h21_resVspre_str_ap[partition] = fs->make<TH2F>(st212ds2.c_str(),ti212ds2.c_str(),500,-5.,5.,200,-12.,12.);
    h21_resVspre_str_vp[partition] = fs->make<TH2F>(st212ds3.c_str(),ti212ds3.c_str(),500,-5.,5.,200,-12.,12.);
  }

  std::vector<std::string> sv;
  sv.push_back("x");
  sv.push_back("y");
  for (unsigned int ch=1;ch<=4;ch++){
    for (std::vector<std::string>::iterator iv=sv.begin(); iv < sv.end(); iv++){
      TrChView view(ch,*iv);
      std::stringstream  title1, shtit1;
      title1 <<"Residuals for Tracker Ch "<<view.first
	     <<" view "<<view.second;
      shtit1<<view.second<<"resCh"<<view.first;
      h_res[view] = fs->make<TH1F>(shtit1.str().c_str(),title1.str().c_str(),200,-0.1,0.1);
      std::string sh2d=shtit1.str()+"VsExtr";
      std::string ti2d=" Extr Vs "+title1.str();
      h_resVspre[view] = fs->make<TH2F>(sh2d.c_str(),ti2d.c_str(),200,-0.1,0.1,48,-6.,6);
      
      for (int ir = 0;ir <nbin;ir++){
	std::stringstream rtit,rsht;
	RotTrChView rview(ir,view);
	rtit<<"Residual Rotation Step "<<std::setw(3)<<ir<<" Ch "<<view.first<<" view "<<view.second;
	rsht<<view.second<<"ResRotSte"<<std::setw(3)<<std::setfill('0')<<ir<<"_"<<view.first;
	h_res_rot[rview] = fs->make<TH1F>(rsht.str().c_str(),rtit.str().c_str(),200,-0.1,0.1);
	std::string shr=rsht.str()+"VsExtr";
	std::string rti=" Extr Vs "+rtit.str();
	h_resVspre_r[rview] = fs->make<TH2F>(shr.c_str(),rti.c_str(),200,-0.1,0.1,48,-6.,6);
      }

    }
  }
}

GEMRot::~GEMRot() {
  std::vector<std::string> sv;
  sv.push_back("x");
  sv.push_back("y");

  std::cout <<"\n----------------------------------------"<<std::endl;
  for (unsigned int ch=1;ch<=4;ch++){
    std::cout <<"|* BARI00"<<ch<<"     X       Y    view"<<std::endl;
    TrChView view1(ch,"x");
    TrChView view2(ch,"y");
    for (int ir = 0;ir <nbin;ir++){
      RotTrChView rview1(ir,view1);
      RotTrChView rview2(ir,view2);
      double theta = th_init+ir*th_bin;
      double rmsx=h_res_rot[rview1]->GetStdDev();
      double rmsy=h_res_rot[rview2]->GetStdDev();
      std::cout<<" Theta "<<theta*1000<<" mrad   Sigma x = "<<rmsx*10000<<" y = "<<rmsy*10000<<std::endl;      
    }
  }
}
//
// member functions
//

// ------------ method called for each event  ------------
void GEMRot::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {

  edm::ESHandle<GEMGeometry> gemg = iSetup.getHandle(geometry_token_);
  mgeom = &*gemg;

  GEMRecHitCollection rhs = iEvent.get(gemrhToken_);

  GEMRecHitCollection exge21 = iEvent.get(exge21Token_);

  GEMRecHitCollection exgt01 = iEvent.get(exgt01Token_);

  GEMRecHitCollection exgt02 = iEvent.get(exgt02Token_);

  GEMRecHitCollection exgt03 = iEvent.get(exgt03Token_);

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
  
  //GE21
  this->Analyse21(rhs,exge21);

}

void GEMRot::Fill(const GEMRecHitCollection& rhs, const GEMRecHitCollection& exgt){

  unsigned int nck_ba = exgt.id_size();
  //must be 1 (maybe 0 if completely misaligned)                                                                                                                                      
  if (nck_ba == 1){
    auto trx_id = *exgt.id_begin();
    GEMDetId try_id(trx_id.region(),trx_id.ring(),trx_id.station(),trx_id.layer(),trx_id.chamber(),trx_id.ieta()+1);
    int TrChNo = (trx_id.chamber()/2-1)*2 + trx_id.ieta()/2 + trx_id.ieta()%2;
    //    std::cout <<" Tracking Chanmber BA00"<<TrChNo<<std::endl;
    auto rhsx = rhs.get(trx_id);
    auto rhsy = rhs.get(try_id);
    unsigned int nclsx = 0;
    unsigned int nclsy = 0;
    for (auto ix = rhsx.first; ix != rhsx.second; ++ix){
      nclsx++;
    }
    for (auto iy = rhsy.first; iy != rhsy.second; ++iy){
      nclsy++;
    }
    auto ehs = exgt.get(trx_id);
    unsigned int nexp = 0;
    for (auto ex = ehs.first; ex != ehs.second; ++ex){
      nexp++;
    }

    // rotate the extraopilated point 
    //  x_n = x*cos(theta) -y*sin(theta)
    //  y_n = x*sin(theta) +y*cos(theta)
    if (nclsx == 1 && nexp == 1){
      TrChView vw(TrChNo,"x");
      auto lp_rh = rhsx.first->localPosition();
      auto lp_ex = ehs.first->localPosition();
      h_res[vw]->Fill(lp_rh.x()-lp_ex.x());
      h_resVspre[vw]->Fill(lp_rh.x()-lp_ex.x(),lp_ex.y());
      for (int ir=0;ir<nbin;ir++){	
	RotTrChView rtv(ir,vw);
	double theta = th_init+ir*th_bin;
	double x_n = lp_ex.x()*cos(theta)-lp_ex.y()*sin(theta);
	double y_n = lp_ex.x()*sin(theta)+lp_ex.y()*cos(theta);	
	h_res_rot[rtv]->Fill(lp_rh.x()-x_n);
	h_resVspre_r[rtv]->Fill(lp_rh.x()-x_n,y_n);
      }
    }
    if (nclsy == 1 && nexp == 1){
      TrChView vw(TrChNo,"y");
      auto lp_rh = rhsy.first->localPosition();
      auto lp_ex = ehs.first->localPosition();
      h_res[vw]->Fill(lp_rh.x()+lp_ex.y());
      h_resVspre[vw]->Fill(lp_rh.x()+lp_ex.y(),lp_ex.x());
      for (int ir=0;ir<nbin;ir++){	
	RotTrChView rtv(ir,vw);
	double theta = th_init+ir*th_bin;
	double x_n = lp_ex.x()*cos(theta)-lp_ex.y()*sin(theta);
	double y_n = lp_ex.x()*sin(theta)+lp_ex.y()*cos(theta);	
	h_res_rot[rtv]->Fill(lp_rh.x()+y_n);
	h_resVspre_r[rtv]->Fill(lp_rh.x()+y_n,x_n);
      }
    }
  }
}


void GEMRot::Analyse21(const GEMRecHitCollection& rhs, const GEMRecHitCollection& exgt){

  LocalPoint lp0(0.,0.,0.);
  LocalPoint lp1(10.,0.,0.);
  LocalPoint lp2(10.,-5.,0.);
  LocalPoint lp3(10.,-10.,0.);
  

  float d=0;
  unsigned int nck_ba = exgt.id_size();
  std::map<int, float> yextr;
  std::map<int, float> xextr;
  std::map<int, float> xmeas;
  std::map<int, float> cstrip;
  std::map<int, int>   clsize;
  std::map<int, int>   fstrip;
  std::map<int, float> estrip;
  std::map<int, float> ecstrip;
  std::map<int, float> emstrip;
  //  std::cout<<" New events # of extrapolaed detectors "<<nck_ba<<std::endl;
  for (auto i = exgt.id_begin(); i!=exgt.id_end(); i++){
    //    std::cout <<" ID "<<*i<<std::endl;
    auto irh = exgt.get(*i);
    GEMDetId mid(*i);
    int part = mid.ieta();
    for (auto rh=irh.first;rh!=irh.second;rh++){
      yextr[part]=rh->localPosition().y() + offset;
      xextr[part]=rh->localPosition().x() + xoff;
      estrip[part]=mgeom->etaPartition(mid)->strip(LocalPoint(xextr[part],yextr[part],0.));
      ecstrip[part]=mgeom->etaPartition(mid)->strip(LocalPoint(xextr[part],0.,0.));
      //      std::cout <<" Partition "<<part<<" center "<<mgeom->etaPartition(mid)->strip(lp0)<<" 10cm displaced "<<mgeom->etaPartition(mid)->strip(lp1)<<" top "<<mgeom->etaPartition(mid)->strip(lp2)
      //		<<" bottom "<<mgeom->etaPartition(mid)->strip(lp3)<<std::endl;
    if (nck_ba == 2) {
	d=d+abs(rh->localPosition().y());
      }
    }
  }
  float y = -10000.;
  float x = -10000.;
  float s = -1.;
  float sc = -1.;
  if ( yextr.find(14) != yextr.end()){
    y = yextr[14]+offset14;
    x = xextr[14];
    s = estrip[14];
    sc = ecstrip[14];
  } else if (yextr.find(15) != yextr.end()){
    y = yextr[15]+offset15;
    x = xextr[15];
    s = estrip[15];
    sc = ecstrip[15];
  } else {
    std::cout <<" Strange no track extrapolated"<<std::endl;
  }

  for (auto i = rhs.id_begin(); i!=rhs.id_end(); i++){
    GEMDetId mid(*i);
    if (mid.station()==2) {
      //      std::cout <<" ID "<<*i<<std::endl;      
      auto mrh=rhs.get(*i);
      int part = mid.ieta();
      int xp = 0;
      float pitch=mgeom->etaPartition(mid)->pitch();
      float lpi = 100;
      if (part==14) {
	xp=14;
	if (yextr.find(xp) != yextr.end())
	  lpi=mgeom->etaPartition(mid)->localPitch(LocalPoint(x,yextr[xp],0.));
	else
	  lpi=mgeom->etaPartition(mid)->localPitch(LocalPoint(x,yextr[15],0.));
      }
      if (part==15) {
	xp=15;
	if (yextr.find(xp) != yextr.end())
	  lpi=mgeom->etaPartition(mid)->localPitch(LocalPoint(x,yextr[xp],0.));
	else
	  lpi=mgeom->etaPartition(mid)->localPitch(LocalPoint(x,yextr[14],0.));
      }
      int nhit=0;
      float xn=100000000.;
      int fs=-1;
      int cls=-1;
      for (auto mh=mrh.first;mh!=mrh.second;mh++){	
	nhit++;
	if (abs(mh->localPosition().x()-x) < xn){
	  xn = mh->localPosition().x();
	  fs = mh->firstClusterStrip();
	  cls = mh->clusterSize();	  
	  emstrip[part]=mgeom->etaPartition(mid)->strip(mh->localPosition());
	}
      }	
      if (xp !=0) {
	xmeas[xp]=xn;
	fstrip[xp]=fs;
	clsize[xp]=cls;
	cstrip[xp]=fs+(cls-1.)/2.;
	h21_res[xp]->Fill(xn-x);
	h21_res_str[xp]->Fill(emstrip[xp]-s);
	h21_res_str_ap[xp]->Fill((emstrip[xp]-s)*pitch);
	h21_res_str_vp[xp]->Fill((emstrip[xp]-s)*lpi);
	h21_rest->Fill(xn-x);
	h21_resVspre[xp]->Fill(xn-x,y);
	h21_resVspre_str[xp]->Fill(emstrip[xp]-s,y);
	h21_resVspre_cstr[xp]->Fill(emstrip[xp]-sc,y);
	h21_resVspre_str_ap[xp]->Fill((emstrip[xp]-s)*pitch,y);
	h21_resVspre_str_vp[xp]->Fill((emstrip[xp]-s)*lpi,y);
      }
      //	std::cout <<"-> Reco Position "<<mh->localPosition()<<std::endl;
    }

    //      if (nhit>1) 
    //	std::cout <<" Number of rechits in partiion "<<part<<" = "<<nhit<<std::endl;
    //	if (nhit==0) 
    //	std::cout <<" No hits in partition "<<part<<" = "<<std::endl;
  }
  
  /*
  if (xextr.find(14) != xextr.end())
    std::cout <<"  Extapolated hits part14 x="<<xextr[14]<<" y="<<yextr[14];
  
  if (xextr.find(15) != xextr.end())
    std::cout <<"  Extapolated hits part15 x="<<xextr[15]<<" y="<<yextr[15];

  if (xmeas.find(14) != xmeas.end())
    std::cout <<"  Measured hits part14 x="<<xmeas[14];

  if (xmeas.find(15) != xmeas.end())
    std::cout <<"  Measured hits part15 x="<<xmeas[15];

  std::cout <<"  aligned y "<<y<<std::endl;
  */
  h21_yInd_den->Fill(y);
  
  if (xmeas.find(14) != xmeas.end() && xmeas.find(15) != xmeas.end()){
    /*
    std::cout <<" --> two simultaneous measured hits "<<xmeas[14]<<" "<<xmeas[15]<<std::endl;
    if (estrip.find(14) !=estrip.end())
      std::cout <<" Partition 14 extrapolated strip " <<std::endl;
    if (estrip.find(15) !=estrip.end())
      std::cout <<" Partition 15 extrapolated strip " <<std::endl;
    std::cout <<"     Eta 14 fstrip="<<std::setw(3)<<fstrip[14]<<" cls="<<std::setw(3)<<clsize[14]<<" central strip="<<cstrip[14]<<" estimate "<<emstrip[14]<<" "<<estrip[14]<<std::endl;
    std::cout <<"     Eta 15 fstrip="<<std::setw(3)<<fstrip[15]<<" cls="<<std::setw(3)<<clsize[15]<<" central strip="<<cstrip[15]<<" estimate "<<emstrip[15]<<" "<<estrip[15]<<std::endl;
    */
    h21_yInd->Fill(xmeas[14]-xmeas[15],y);
    h21_yInd_and->Fill(y);
    h21_yInd_eff->Fill(y);
    h21_yInd1->Fill(xmeas[14]-xmeas[15],yextr[14]);
    h21_yInd2->Fill(xmeas[14]-xmeas[15],yextr[15]);

  }else if (xmeas.find(14) != xmeas.end()){
    h21_yInd_x1Only->Fill(y);
    if (yextr.find(14) != yextr.end()) h21_yInd_x1Onlyck->Fill(y);
    h21_yInd_eff->Fill(y);
  }else if (xmeas.find(15) != xmeas.end()){
    h21_yInd_x2Only->Fill(y);
    if (yextr.find(15) != yextr.end()) h21_yInd_x2Onlyck->Fill(y);
    h21_yInd_eff->Fill(y);
  } else {
    //    std::cout <<" No Measured Hits"<<std::endl;
  }
  // if (nck_ba == 1) std::cout <<" One extratrpolation Runnimin min "<<miny<<" Running max "<<maxy<<std::endl;
}

// ------------ method called once each job just before starting event loop  ------------
void GEMRot::beginJob() {
  // please remove this method if not needed
}


// ------------ method called once each job just after ending the event loop  ------------
void GEMRot::endJob() {
  // please remove this method if not needed
  std::cout <<" Min "<<miny<<"  MAX "<<maxy<<std::endl;
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

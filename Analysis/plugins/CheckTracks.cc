#ifndef CheckTracks_H
#define CheckTracks_H
// -*- C++ -*-
//
// Package:    TB2021/CheckTracks
// Class:      CheckTracks
//
/**\class CheckTracks CheckTracks.cc TB2021/CheckTracks/plugins/CheckTracks.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Marcello Maggi
//         Created:  Thu, 09 Dec 2021 14:16:55 GMT
//
//

// system include files
#include <memory>
#include <sstream>
#include <fstream>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/one/EDAnalyzer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"

// Track 
#include "DataFormats/TrackReco/interface/Track.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "TrackingTools/TrackFitters/interface/TrajectoryStateCombiner.h"

//Muon
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "RecoMuon/TransientTrackingRecHit/interface/MuonTransientTrackingRecHit.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "RecoMuon/StandAloneTrackFinder/interface/StandAloneMuonSmoother.h"

// GEM
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

// Root                                                                                                                            
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CommonTools/UtilAlgos/interface/TFileService.h"
#include "TH2F.h"

typedef std::pair<int, std::string> TrChView;
typedef std::pair<int, TrChView> ClsTrChView;
//
// class declaration
//
class CheckTracks : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit CheckTracks(const edm::ParameterSet&);
  ~CheckTracks();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<reco::TrackCollection> tracks1_;
  edm::EDGetTokenT<reco::TrackCollection> tracks2_;
  edm::EDGetTokenT<reco::TrackCollection> tracks3_;
  edm::EDGetTokenT<reco::TrackCollection> tracks4_;
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> hGEMGeom_;
  std::map<TrChView, TH1F*> h_res,h_rec,h_pre;
  std::map<TrChView, TH2F*> h_resVspre;
  std::map<ClsTrChView, TH1F*> h_res_cl,h_rec_cl,h_pre_cl;
private:
  std::ofstream fout;
private:
  bool first;
};

#endif

CheckTracks::CheckTracks(const edm::ParameterSet& iConfig) 
  : hGEMGeom_(esConsumes()),
    first(true)
{
  fout.open("GEMRecHit.output");
  tracks1_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("track1"));
  tracks2_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("track2"));
  tracks3_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("track3"));
  tracks4_ = consumes<reco::TrackCollection>(iConfig.getParameter<edm::InputTag>("track4"));
  gemRecHits_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("gemRecHitLabel"));

  edm::Service<TFileService> fs;
  std::vector<std::string> sv;
  sv.push_back("x");
  sv.push_back("y");
  for (unsigned int ch = 2; ch <= 4; ch++) {
    for (std::vector<std::string>::iterator iv=sv.begin(); iv < sv.end(); iv++) {
      TrChView view(ch, *iv);
      std::stringstream  title1, shtit1;
      std::stringstream  title2, shtit2;
      std::stringstream  title3, shtit3;
      title1 << "Residuals for Tracker Ch " << view.first
             <<" view " << view.second;
      shtit1<<view.second << "resCh" << view.first;
      h_res[view] = fs->make<TH1F>(shtit1.str().c_str(), title1.str().c_str(), 2000, -1., 1.);
      std::string sh2d = shtit1.str()+"VsExtr";
      std::string ti2d = " Extr Vs "+title1.str();
      h_resVspre[view] = fs->make<TH2F>(sh2d.c_str(), ti2d.c_str(), 2000, -1., 1., 48, -6., 6);
      title2 << " Rechit position for Tracker Ch " << view.first
             << " view " << view.second;
      shtit2<<view.second << "recCh" << view.first;
      h_rec[view] = fs->make<TH1F>(shtit2.str().c_str(), title2.str().c_str(), 1920, -6., 6.);
      title3 << " Extrapolated position for Tracker Ch " << view.first
             << " view " << view.second;
      shtit3<<view.second << "extCh" << view.first;
      h_pre[view] = fs->make<TH1F>(shtit3.str().c_str(), title3.str().c_str(), 3840, -6., 6.);
      for (unsigned int cl = 1; cl <= 5; cl++) {
         ClsTrChView clv(cl,view);
         std::stringstream titlecl1, shtitcl1;
         std::stringstream titlecl2, shtitcl2;
         std::stringstream titlecl3, shtitcl3;
         titlecl1<<title1.str() << " cl size = " << cl;
         shtitcl1<<shtit1.str() << "cls" << cl;
         h_res_cl[clv] = fs->make<TH1F>(shtitcl1.str().c_str(), titlecl1.str().c_str(), 2000, -1., 1.);
         titlecl2<<title2.str() << " cl size = " << cl;
         shtitcl2<<shtit2.str() << "cls" << cl;
         h_rec_cl[clv] = fs->make<TH1F>(shtitcl2.str().c_str(), titlecl2.str().c_str(), 2000, -6., 6.);
         titlecl3<<title3.str() << " cl size = " << cl;
         shtitcl3<<shtit3.str() << "cls" << cl;
         h_pre_cl[clv] = fs->make<TH1F>(shtitcl3.str().c_str(), titlecl3.str().c_str(), 2000, -6., 6.);
      }
    }
  }
}

CheckTracks::~CheckTracks() {
}

//
// member functions
//

// ------------ method called for each event  ------------
void CheckTracks::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  fout << "+++ Event Number " << iEvent.id().event() << std::endl;
  if (iEvent.id().event()%1000 == 0 )
    std::cout << " Event process " << iEvent.id().event() << std::endl;
  /* GEM Geometry */

  edm::ESHandle<GEMGeometry> hGEMGeom = iSetup.getHandle(hGEMGeom_);
//  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  std::vector<edm::Handle<std::vector<reco::Track>>> tracks(4);
  iEvent.getByToken(tracks1_, tracks[0]);
  iEvent.getByToken(tracks2_, tracks[1]);
  iEvent.getByToken(tracks3_, tracks[2]);
  iEvent.getByToken(tracks4_, tracks[3]);

  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_, gemRecHits);

  std::vector<const GEMEtaPartition*> xCham(4), yCham(4);
  for (auto et : GEMGeometry_->etaPartitions()) {
    if (first) {
      std::cout << " station id" << et->id() << std::endl;
    }
    int st = et->id().station();
    if (st == 1 ){
      int ieta = et->id().ieta();
      int ch = et->id().chamber();
      int idx = (ch/2-1)*2 + ieta/2 + ieta%2 ;
      if (idx <= 4) {
        if (ieta%2 == 1){
           xCham[idx-1] = et;
        } else {
           yCham[idx-1] = et;
        }
      }
    }
  }
  if (first) first = false;

  //  unsigned int nGoodCh=0;
  for (unsigned int i = 0; i < 4;i++) {
  
    GEMRecHitCollection::range rx = gemRecHits->get(xCham[i]->id());
    GEMRecHitCollection::range ry = gemRecHits->get(yCham[i]->id());
    unsigned int nhitX = 0;
    for (auto ix = rx.first; ix != rx.second; ++ix) {
      nhitX++;
    }
    unsigned int nhitY = 0;
    for (auto iy = ry.first; iy != ry.second; ++iy) {
      nhitY++;
    }

    if (nhitX == 1 && nhitY == 1 ) {
      fout << "Tracker chamber BARI00" << i+1 << std::endl;
      fout << " X view: ClusterSize " << rx.first->clusterSize() << " Local position " << rx.first->localPosition() << " global " << xCham[i]->toGlobal(rx.first->localPosition()) << std::endl;
      fout << " Y view: ClusterSize " << ry.first->clusterSize() << " Local position " << ry.first->localPosition() << " global " << yCham[i]->toGlobal(ry.first->localPosition()) << std::endl;
      if ( rx.first->clusterSize() <= 5 && ry.first->clusterSize() <= 5 ) {}
    }
  } 

  std::cout << " Hit studies for theis evebt" << std::endl;
  for (int excl = 0; excl < 4; excl++) {
    std::cout << " Excluded Chamber BARI000" << excl+2 << std::endl;
    for (std::vector<reco::Track>::const_iterator track = tracks[excl]->begin(); track != tracks[excl]->end(); ++track) {
      for (auto hit : track->recHits()) {
         auto etaPart = GEMGeometry_->etaPartition(hit->geographicalId());
         auto etaPartId = etaPart->id();
         std::cout << " Rechit Geo Id for track " << etaPartId << std::endl;
         int st = etaPartId.station();
         if (st == 1 ){
           int ieta = etaPartId.ieta();
           int ch = etaPartId.chamber();
           int idx = (ch/2-1)*2 + ieta/2 + ieta%2 ;
           if (idx <= 4) {
             std::string sv;
             if (ieta%2 == 1){
               sv = "x";
             } else {
               sv = "y";
             }
             TrChView view(idx, sv);
             std::cout << "Track associated Rec Hit for BARI00" << idx << " View " << sv << std::endl;
             auto lp_track = hit->localPosition();
             auto gp_track = etaPart->toGlobal(lp_track);
             auto range = gemRecHits->get(etaPartId);
             for (auto rechit = range.first; rechit != range.second; ++rechit) {
               auto lp_rechit = rechit->localPosition();             
               auto gp_rechit = etaPart->toGlobal(lp_rechit);               
               if (idx == excl+2) {
                  std::cout << " global position track " << gp_track << " rechit " << gp_rechit << std::endl;
                  h_res[view]->Fill(lp_rechit.x()-lp_track.x());
                  h_resVspre[view]->Fill(lp_rechit.x()-lp_track.x(), lp_track.x());
                  h_rec[view]->Fill(lp_rechit.x());
                  h_pre[view]->Fill(lp_track.x());
                  int cls = rechit->clusterSize();
                  ClsTrChView clv(cls,view);
                  h_res_cl[clv]->Fill(lp_rechit.x()-lp_track.x());
                  h_rec_cl[clv]->Fill(lp_rechit.x());
                  h_pre_cl[clv]->Fill(lp_track.x());
                  fout << "Excluding BARI00" << idx << " extrapolated tracks local " << lp_track << " global " << gp_track << " rechit local " << lp_rechit << " global " << gp_rechit << std::endl;
               }               
             }
           }
         }
      }
    }
  }
}

// ------------ method called once each job just before starting event loop  ------------
void CheckTracks::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void CheckTracks::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void CheckTracks::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
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
DEFINE_FWK_MODULE(CheckTracks);

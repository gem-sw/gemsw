#ifndef PerfTrack_H
#define PerfTrack_H
// -*- C++ -*-
//
// Package:    TB2021/PerfTrack
// Class:      PerfTrack
//
/**\class PerfTrack PerfTrack.cc TB2021/PerfTrack/plugins/PerfTrack.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Marcello Maggi
//         Created:  Fri, 10 Dec 2021 12:47:24 GMT
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDFilter.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/MakerMacros.h"

#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/StreamID.h"

#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

//
// class declaration
//

class PerfTrack : public edm::stream::EDFilter<> {
public:
  explicit PerfTrack(const edm::ParameterSet&);
  ~PerfTrack();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  virtual void beginStream(edm::StreamID) override;
  virtual bool filter(edm::Event&, const edm::EventSetup&) override;
  virtual void endStream() override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<GEMRecHitCollection> theGEMRecHitToken_;
  unsigned int nHitSel;
};

#endif

PerfTrack::PerfTrack(const edm::ParameterSet& iConfig) {
  theGEMRecHitToken_ = consumes<GEMRecHitCollection>(iConfig.getParameter<edm::InputTag>("recHitLabel"));
  nHitSel=iConfig.getParameter<unsigned int>("nHitmin");
}

PerfTrack::~PerfTrack() {
}

//
// member functions
//

// ------------ method called on each new Event  ------------
bool PerfTrack::filter(edm::Event& iEvent, const edm::EventSetup& iSetup) {
 
  edm::ESHandle<GEMGeometry> gemg;
  iSetup.get<MuonGeometryRecord>().get(gemg);
  const GEMGeometry* mgeom = &*gemg;
  GEMRecHitCollection rechits=iEvent.get(theGEMRecHitToken_);


  std::vector<const GEMEtaPartition*> xCham(4),yCham(4);
  for (auto et : mgeom->etaPartitions()) {
    int st = et->id().station();
    if (st == 1 ) {
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

  unsigned int nGoodCh=0;
  for (unsigned int i=0; i<4; i++) {
    GEMRecHitCollection::range rx = rechits.get(xCham[i]->id());
    GEMRecHitCollection::range ry = rechits.get(yCham[i]->id());
    unsigned int nhitX = 0;
    for (auto ix = rx.first; ix != rx.second; ++ix) {
      nhitX++;
    }
    unsigned int nhitY = 0;
    for (auto iy = ry.first; iy != ry.second; ++iy) {
      nhitY++;
    }

    if (nhitX == 1 && nhitY ==1 ) {
      if ( rx.first->clusterSize() <=5 && ry.first->clusterSize() <=5 ) {
        nGoodCh++;
      }
    }
  } 
  if (iEvent.id().event() % 100000 == 0 ) std::cout << " Event " << iEvent.id().event() << " Number of god chamber "<< nGoodCh << std::endl;
  return nGoodCh >= nHitSel;
}

// ------------ method called once each stream before processing any runs, lumis or events  ------------
void PerfTrack::beginStream(edm::StreamID) {
  // please remove this method if not needed
}

// ------------ method called once each stream after processing all runs, lumis and events  ------------
void PerfTrack::endStream() {
  // please remove this method if not needed
}

// ------------ method called when starting to processes a run  ------------
/*
void
PerfTrack::beginRun(edm::Run const&, edm::EventSetup const&)
{ 
}
*/

// ------------ method called when ending the processing of a run  ------------
/*
void
PerfTrack::endRun(edm::Run const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when starting to processes a luminosity block  ------------
/*
void
PerfTrack::beginLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method called when ending the processing of a luminosity block  ------------
/*
void
PerfTrack::endLuminosityBlock(edm::LuminosityBlock const&, edm::EventSetup const&)
{
}
*/

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void PerfTrack::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);
}
//define this as a plug-in
DEFINE_FWK_MODULE(PerfTrack);

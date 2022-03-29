gi// -*- C++ -*-
//
// Package:    TB2021/ExtraPGE21
// Class:      ExtraPGE21
//
/**\class ExtraPGE21 ExtraPGE21.cc TB2021/ExtraPGE21/plugins/ExtraPGE21.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  mmaggi
//         Created:  Mon, 24 Jan 2022 10:16:29 GMT
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
#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrackReco/interface/TrackFwd.h"

#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"


//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.

using reco::TrackCollection;

class ExtraPGE21 : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit ExtraPGE21(const edm::ParameterSet&);
  ~ExtraPGE21();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

  // ----------member data ---------------------------
  edm::EDGetTokenT<TrackCollection> tracksToken_;  //used to select what tracks to read from configuration file
  edm::EDGetTokenT<GEMRecHitCollection> gemRecHits_;

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
ExtraPGE21::ExtraPGE21(const edm::ParameterSet& iConfig)
  : tracksToken_(consumes<reco::TrackCollection>(iConfig.getUntrackedParameter<edm::InputTag>("track4"))) ,
    gemRecHits_(consumes<GEMRecHitCollection>(iConfig.getUntrackedParameter<edm::InputTag>("gemRecHitLabel")))
{}

ExtraPGE21::~ExtraPGE21() {

}

//
// member functions
//

// ------------ method called for each event  ------------
void ExtraPGE21::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  if (iEvent.id().event()%1 == 0 ) std::cout << " Event process " << iEvent.id().event() << std::endl;

  edm::ESHandle<GEMGeometry> hGEMGeom;
  iSetup.get<MuonGeometryRecord>().get(hGEMGeom);
  const GEMGeometry* GEMGeometry_ = &*hGEMGeom;

  edm::Handle<GEMRecHitCollection> gemRecHits;
  iEvent.getByToken(gemRecHits_, gemRecHits);

  edm::Handle<std::vector<reco::Track>> tracks;

  iEvent.getByToken(tracksToken_, tracks);

  for (std::vector<reco::Track>::const_iterator track = tracks->begin(); track < tracks->end(); track++) {
    std::cout << " TRACK " << std::endl;
    for (auto hit : track->recHits()) {
      auto etaPart = GEMGeometry_->etaPartition(hit->geographicalId());
      auto etaPartId = etaPart->id();
      std::cout << " Rechit Geo Id for track " << etaPartId << std::endl;
    }
  }
}

// ------------ method called once each job just before starting event loop  ------------
void ExtraPGE21::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void ExtraPGE21::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void ExtraPGE21::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
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
DEFINE_FWK_MODULE(ExtraPGE21);

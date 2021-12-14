// -*- C++ -*-
//
// Package:    TB2021/ReadAli
// Class:      ReadAli
//
/**\class ReadAli ReadAli.cc TB2021/ReadAli/plugins/ReadAli.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Marcello Maggi
//         Created:  Wed, 01 Dec 2021 09:05:06 GMT
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
#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"

// Alignments                                                                                                                                            
#include "CondFormats/Alignment/interface/DetectorGlobalPosition.h"
#include "CondFormats/Alignment/interface/AlignmentErrorsExtended.h"
#include "CondFormats/AlignmentRecord/interface/GlobalPositionRcd.h"
#include "CondFormats/AlignmentRecord/interface/GEMAlignmentRcd.h"
#include "CondFormats/AlignmentRecord/interface/GEMAlignmentErrorExtendedRcd.h"
#include "Geometry/CommonTopologies/interface/GeometryAligner.h"


//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.


class ReadAli : public edm::one::EDAnalyzer<edm::one::SharedResources> {
public:
  explicit ReadAli(const edm::ParameterSet&);
  ~ReadAli();

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

private:
  void beginJob() override;
  void analyze(const edm::Event&, const edm::EventSetup&) override;
  void endJob() override;

private:
  //  edm::ESGetToken<Alignments, GlobalPositionRcd> globalPositionToken_;
  edm::ESGetToken<Alignments, GEMAlignmentRcd> alignmentsToken_;
  edm::ESGetToken<AlignmentErrorsExtended, GEMAlignmentErrorExtendedRcd> alignmentErrorsToken_;

  // ----------member data ---------------------------
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
ReadAli::ReadAli(const edm::ParameterSet& iConfig){

  alignmentsToken_ = esConsumes<Alignments, GEMAlignmentRcd>();
  alignmentErrorsToken_ = esConsumes<AlignmentErrorsExtended, GEMAlignmentErrorExtendedRcd>();
}

ReadAli::~ReadAli() {
  // do anything here that needs to be done at desctruction time
  // (e.g. close files, deallocate resources etc.)
  //
  // please remove this method altogether if it would be left empty
}

//
// member functions
//

// ------------ method called for each event  ------------
void ReadAli::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup) {
  using namespace edm;
  
  std::cout <<" ESHandle Geo " <<std::endl;
  edm::ESHandle<GEMGeometry> gemg;
  std::cout <<" GET " <<std::endl;
  iSetup.get<MuonGeometryRecord>().get(gemg);
  std::cout <<" Geo "<<std::endl;
  const GEMGeometry* mgeom = &*gemg;
  std::cout <<" Geo ok "<<std::endl;

  //  const auto& globalPosition = record.get(globalPositionToken_);
  
  std::cout <<" Alignement "<<std::endl;
  const auto& alignments = iSetup.getData(alignmentsToken_);
  std::cout <<" Alignement errors"<<std::endl;
  const auto& alignmentErrors = iSetup.getData(alignmentErrorsToken_);
  std::cout <<" Aligner"<<std::endl;

  GeometryAligner aligner;
  std::cout <<" Apply " <<std::endl;
  aligner.applyAlignments<GEMGeometry>(mgeom,
				       &alignments,
				       &alignmentErrors,
  				       AlignTransform());
  
  std::cout <<" done"<<std::endl;



  for (auto et : mgeom->etaPartitions()){
    std::cout <<" Chamber \n"<<et->id()<<std::endl;
    int st = et->id().station();
    if (st == 1 ){
      int ieta = et->id().ieta();
      int ch = et->id().chamber();
      int idx = (ch/2-1)*2 + ieta/2 + ieta%2 ;
      std::cout <<"BARI0"<<idx<<std::endl;
      if (ieta%2==1){

      } else {

      }
      auto center = et->surface().toGlobal(LocalPoint(0,0,0));
      std::cout <<" Ccenter "<<center<<std::endl;
    }
  }
}

// ------------ method called once each job just before starting event loop  ------------
void ReadAli::beginJob() {
  // please remove this method if not needed
}

// ------------ method called once each job just after ending the event loop  ------------
void ReadAli::endJob() {
  // please remove this method if not needed
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void ReadAli::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
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
DEFINE_FWK_MODULE(ReadAli);

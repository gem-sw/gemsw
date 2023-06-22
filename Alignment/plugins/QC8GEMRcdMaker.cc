// -*- C++ -*-
//
// Package:    QC8GEMRcdMaker
// Class:      QC8GEMRcdMaker
//
/**\class QC8GEMRcdMaker QC8GEMRcdMaker.cc QC8GEMRcdMaker/plugins/QC8GEMRcdMaker.cc

 Description: [one line class summary]

 Implementation:
     [Notes on implementation]
*/
//
// Original Author:  Ian James Watson
//         Created:  Wed, 13 Feb 2019 14:29:19 GMT
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

#include "CondFormats/AlignmentRecord/interface/GEMAlignmentRcd.h"
#include "CondFormats/AlignmentRecord/interface/GEMAlignmentErrorRcd.h"
#include "CondFormats/AlignmentRecord/interface/GEMAlignmentErrorExtendedRcd.h"

#include "CondFormats/Alignment/interface/Alignments.h"
#include "CondFormats/Alignment/interface/AlignmentErrors.h"
#include "CondFormats/Alignment/interface/AlignmentErrorsExtended.h"

#include "FWCore/ServiceRegistry/interface/Service.h"
#include "CondCore/DBOutputService/interface/PoolDBOutputService.h"

#include "CLHEP/Vector/EulerAngles.h"
#include "CLHEP/Vector/Rotation.h"
#include "CLHEP/Vector/ThreeVector.h"
#include "CLHEP/Geometry/Transform3D.h"
#include "CondFormats/Alignment/interface/AlignTransform.h"

#include "DataFormats/DetId/interface/DetId.h"
#include "DataFormats/MuonDetId/interface/MuonSubdetId.h"
#include "DataFormats/MuonDetId/interface/GEMDetId.h"

#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/Records/interface/MuonGeometryRecord.h"

#include "Alignment/CommonAlignment/interface/Utilities.h"

//
// class declaration
//

// If the analyzer does not use TFileService, please remove
// the template argument to the base class so the class inherits
// from  edm::one::EDAnalyzer<>
// This will improve performance in multithreaded jobs.


using reco::TrackCollection;

class QC8GEMRcdMaker : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit QC8GEMRcdMaker(const edm::ParameterSet&);
      ~QC8GEMRcdMaker();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


  private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      // ----------member data ---------------------------
      edm::ESGetToken<GEMGeometry, MuonGeometryRecord> gemGeoToken_; 
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
QC8GEMRcdMaker::QC8GEMRcdMaker(const edm::ParameterSet& p)
  : gemGeoToken_(esConsumes())
{}


QC8GEMRcdMaker::~QC8GEMRcdMaker()
{
   // do anything here that needs to be done at desctruction time
   // (e.g. close files, deallocate resources etc.)
}


//
// member functions
//

using CLHEP::Hep3Vector;
using CLHEP::HepRotation;
// ------------ method called for each event  ------------
void
QC8GEMRcdMaker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  // Get the GEM Geometry
  edm::ESHandle<GEMGeometry> gemGeoHandle = iSetup.getHandle(gemGeoToken_);
  const GEMGeometry* gemGeo = &*gemGeoHandle;

//  edm::ESHandle<GEMGeometry> gemGeo;
//  iSetup.get<MuonGeometryRecord>().get(gemGeo);


  Alignments* QC8GEMAlignment = new Alignments();
  AlignmentErrorsExtended* QC8GEMAlignmentErrorExtended = new AlignmentErrorsExtended();


  for (auto roll : gemGeo->etaPartitions()) {
    auto center = roll->surface().toGlobal(LocalPoint(0,0,0));
    std::cout <<" Start from center "<<center<<std::endl;
    auto rot = roll->surface().rotation();
    std::cout <<" and rotation     \n"<<rot<<std::endl;
    auto hrot = HepRotation(Hep3Vector(rot.xx(), rot.xy(), rot.xz()).unit(),
                            Hep3Vector(rot.yx(), rot.yy(), rot.yz()).unit(),
                            Hep3Vector(rot.zx(), rot.zy(), rot.zz()).unit()
                            );
    auto euler = hrot.inverse().eulerAngles();
    auto phir = HepRotation();


    // Now Take jut the tracker GEM chamber identified as Station 1
    QC8GEMAlignment->m_align.push_back(AlignTransform(AlignTransform::Translation(center.x(), center.y(), center.z()),
                                                     euler,
                                                     roll->id()));
    QC8GEMAlignmentErrorExtended->m_alignError.push_back(AlignTransformErrorExtended(AlignTransformErrorExtended::SymMatrix(6),    
                                                                                    roll->id()));

  }
  
  // Madatory to incude alll the pieces of the Geometry into record
  for (auto chmb : gemGeo->chambers()) {
    auto center = chmb->surface().toGlobal(LocalPoint(0,0,0));
    auto rot = chmb->surface().rotation();
    auto hrot = HepRotation(Hep3Vector(rot.xx(), rot.xy(), rot.xz()).unit(),
                            Hep3Vector(rot.yx(), rot.yy(), rot.yz()).unit(),
                            Hep3Vector(rot.zx(), rot.zy(), rot.zz()).unit()
                            );
    auto euler = hrot.inverse().eulerAngles();
    QC8GEMAlignment->m_align.push_back(AlignTransform(AlignTransform::Translation(center.x(), center.y(), center.z()),
                                                                                 euler,
                                                                                 chmb->id()));
    QC8GEMAlignmentErrorExtended->m_alignError.push_back(AlignTransformErrorExtended(AlignTransformErrorExtended::SymMatrix(6),
                                                                                    chmb->id()));
  }

  for (auto sch : gemGeo->superChambers()) {
    auto center = sch->surface().toGlobal(LocalPoint(0,0,0));
    auto rot = sch->surface().rotation();
    auto hrot = HepRotation(Hep3Vector(rot.xx(), rot.xy(), rot.xz()).unit(),
                            Hep3Vector(rot.yx(), rot.yy(), rot.yz()).unit(),
                            Hep3Vector(rot.zx(), rot.zy(), rot.zz()).unit()
                            );
    auto euler = hrot.inverse().eulerAngles();
    QC8GEMAlignment->m_align.push_back(AlignTransform(AlignTransform::Translation(center.x(), center.y(), center.z()),
                                                                                 euler,
                                                                                 sch->id()));
    QC8GEMAlignmentErrorExtended->m_alignError.push_back(AlignTransformErrorExtended(AlignTransformErrorExtended::SymMatrix(6),
                                                                                    sch->id()));
  }
  // GeometryAligner expects ordering by raw ID
  std::sort(QC8GEMAlignment->m_align.begin(), QC8GEMAlignment->m_align.end(), [](AlignTransform a, AlignTransform b){return a.rawId() < b.rawId();});
  std::sort(QC8GEMAlignmentErrorExtended->m_alignError.begin(), QC8GEMAlignmentErrorExtended->m_alignError.end(), [](auto a, auto b){return a.rawId() < b.rawId();});
  edm::Service<cond::service::PoolDBOutputService> poolDbService;
  if( poolDbService.isAvailable() ) {
    poolDbService->writeOneIOV<Alignments>(*QC8GEMAlignment, poolDbService->currentTime(),
                                        "GEMAlignmentRcd"  );
    poolDbService->writeOneIOV<AlignmentErrorsExtended>(*QC8GEMAlignmentErrorExtended, poolDbService->currentTime(),
                                                     "GEMAlignmentErrorExtendedRcd"  );
  }
  else
    throw std::runtime_error("PoolDBService required.");
}


// ------------ method called once each job just before starting event loop  ------------
void
QC8GEMRcdMaker::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void
QC8GEMRcdMaker::endJob()
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
QC8GEMRcdMaker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  //The following says we do not know what parameters are allowed so do no validation
  // Please change this to state exactly what you do use, even if it is no parameters
  edm::ParameterSetDescription desc;
  desc.setUnknown();
  descriptions.addDefault(desc);

  //Specify that only 'tracks' is allowed
  //To use, remove the default given above and uncomment below
  //ParameterSetDescription desc;
  //desc.addUntracked<edm::InputTag>("tracks","ctfWithMaterialTracks");
  //descriptions.addDefault(desc);
}

//define this as a plug-in
DEFINE_FWK_MODULE(QC8GEMRcdMaker);

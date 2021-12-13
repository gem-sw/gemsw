// -*- C++ -*-
//
// Package:    TBGEMRcdMaker
// Class:      TBGEMRcdMaker
//
/**\class TBGEMRcdMaker TBGEMRcdMaker.cc TBGEMRcdMaker/plugins/TBGEMRcdMaker.cc

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

class TBGEMRcdMaker : public edm::one::EDAnalyzer<edm::one::SharedResources>  {
   public:
      explicit TBGEMRcdMaker(const edm::ParameterSet&);
      ~TBGEMRcdMaker();

      static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);


  private:
      virtual void beginJob() override;
      virtual void analyze(const edm::Event&, const edm::EventSetup&) override;
      virtual void endJob() override;

      float xShift1;
      float yShift1;
      float pShift1;
      float xShift2;
      float yShift2;
      float pShift2;
      float xShift3;
      float yShift3;
      float pShift3;
      float xShift4;
      float yShift4;
      float pShift4;

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
TBGEMRcdMaker::TBGEMRcdMaker(const edm::ParameterSet& p)
{
  xShift1 = p.getParameter<double>("xShift1");
  yShift1 = p.getParameter<double>("yShift1");
  pShift1 = p.getParameter<double>("phiShift1");
  xShift2 = p.getParameter<double>("xShift2");
  yShift2 = p.getParameter<double>("yShift2");
  pShift2 = p.getParameter<double>("phiShift2");
  xShift3 = p.getParameter<double>("xShift3");
  yShift3 = p.getParameter<double>("yShift3");
  pShift3 = p.getParameter<double>("phiShift3");
  xShift4 = p.getParameter<double>("xShift4");
  yShift4 = p.getParameter<double>("yShift4");
  pShift4 = p.getParameter<double>("phiShift4");
}


TBGEMRcdMaker::~TBGEMRcdMaker()
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
TBGEMRcdMaker::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{
  // Get the GEM Geometry
  edm::ESHandle<GEMGeometry> gemGeo;
  iSetup.get<MuonGeometryRecord>().get(gemGeo);


  Alignments* TBGEMAlignment = new Alignments();
  AlignmentErrorsExtended* TBGEMAlignmentErrorExtended = new AlignmentErrorsExtended();


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
    int st = roll->id().station();
    if (st == 1 ){
      // The X and Y view are tw different eta partition but thery are the same chamber with the same geomtrical position.
      // From the GEMDetId (ieta, chamber the Chmaber number and the X Y view are identified.
      int ieta = roll->id().ieta();
      int ch = roll->id().chamber();
      std::cout << ch << " ::: " << ieta << " ::: ";
      // idx will be the tracker chamber index from 1 to 4. 1 Will not be touched, instead 2 3 and 4 will be shifted and rotaed
      int idx = (ch/2-1)*2 + ieta/2 + ieta%2 ;
      std::cout << idx << std::endl;
      
      if (idx == 1){
        if (ieta%2 == 1){
          // xview 
          center = roll->surface().toGlobal(LocalPoint(xShift1,yShift1,0.));
          phir = HepRotation(Hep3Vector(cos(pShift1),-sin(pShift1),0.).unit(),
                              Hep3Vector(sin(pShift1),cos(pShift1),0.).unit(),
                              Hep3Vector(0.,0.,1.).unit());
        }else{
          //y view having a different rotation
          center = roll->surface().toGlobal(LocalPoint(-yShift1,xShift1,0.));
          phir = HepRotation(Hep3Vector(cos(pShift1),-sin(pShift1),0.).unit(),
                             Hep3Vector(sin(pShift1),cos(pShift1),0.).unit(),
                             Hep3Vector(0.,0.,1.).unit());

        }
        auto totr = phir*hrot;
        euler = totr.inverse().eulerAngles();
      }if (idx == 2){
        if (ieta%2 == 1){
          // xview 
          center = roll->surface().toGlobal(LocalPoint(xShift2,yShift2,0.));
          phir = HepRotation(Hep3Vector(cos(pShift2),-sin(pShift2),0.).unit(),
                              Hep3Vector(sin(pShift2),cos(pShift2),0.).unit(),
                              Hep3Vector(0.,0.,1.).unit());
        }else{
          //y view having a different rotation
          center = roll->surface().toGlobal(LocalPoint(-yShift2,xShift2,0.));
          phir = HepRotation(Hep3Vector(cos(pShift2),-sin(pShift2),0.).unit(),
                             Hep3Vector(sin(pShift2),cos(pShift2),0.).unit(),
                             Hep3Vector(0.,0.,1.).unit());

        }
        auto totr = phir*hrot;
        euler = totr.inverse().eulerAngles();
      } else if (idx == 3) {
        if (ieta%2 == 1){
          // xview 
          center = roll->surface().toGlobal(LocalPoint(xShift3,yShift3,0.));
          phir = HepRotation(Hep3Vector(cos(pShift3),-sin(pShift3),0.).unit(),
                             Hep3Vector(sin(pShift3),cos(pShift3),0.).unit(),
                             Hep3Vector(0.,0.,1.).unit());
          
        }else{
          //y view having a different rotation     
          center = roll->surface().toGlobal(LocalPoint(-yShift3,xShift3,0.));
          phir = HepRotation(Hep3Vector(cos(pShift3),-sin(pShift3),0.).unit(),
                             Hep3Vector(sin(pShift3),cos(pShift3),0.).unit(),
                             Hep3Vector(0.,0.,1.).unit());
          

        }
        auto totr = phir*hrot;
        euler = totr.inverse().eulerAngles();
      } else if (idx == 4){
        if (ieta%2 == 1){
          // xview 
          center = roll->surface().toGlobal(LocalPoint(xShift4,yShift4,0.));
          phir = HepRotation(Hep3Vector(cos(pShift4),sin(pShift4),0.).unit(),
                             Hep3Vector(-sin(pShift4),cos(pShift4),0.).unit(),
                             Hep3Vector(0.,0.,1.).unit());
        }else{
          //y view having a different rotation     
          center = roll->surface().toGlobal(LocalPoint(-yShift4,xShift4,0.));
          phir = HepRotation(Hep3Vector(cos(pShift4),sin(pShift4),0.).unit(),
                             Hep3Vector(-sin(pShift4),cos(pShift4),0.).unit(),
                             Hep3Vector(0.,0.,1.).unit());
        }
        auto totr = phir*hrot;
        euler = totr.inverse().eulerAngles();

      }     
      
      std::cout <<"ID "<<roll->id()<<"\nCenter"<<center<<"\n ________________________________________________ \n "<<std::endl;
      
    }
    TBGEMAlignment->m_align.push_back(AlignTransform(AlignTransform::Translation(center.x(), center.y(), center.z()),
                                                     euler,
                                                     roll->id()));
    TBGEMAlignmentErrorExtended->m_alignError.push_back(AlignTransformErrorExtended(AlignTransformErrorExtended::SymMatrix(6),    
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
    TBGEMAlignment->m_align.push_back(AlignTransform(AlignTransform::Translation(center.x(), center.y(), center.z()),
                                                                                 euler,
                                                                                 chmb->id()));
    TBGEMAlignmentErrorExtended->m_alignError.push_back(AlignTransformErrorExtended(AlignTransformErrorExtended::SymMatrix(6),
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
    TBGEMAlignment->m_align.push_back(AlignTransform(AlignTransform::Translation(center.x(), center.y(), center.z()),
                                                                                 euler,
                                                                                 sch->id()));
    TBGEMAlignmentErrorExtended->m_alignError.push_back(AlignTransformErrorExtended(AlignTransformErrorExtended::SymMatrix(6),
                                                                                    sch->id()));
  }
  // GeometryAligner expects ordering by raw ID
  std::sort(TBGEMAlignment->m_align.begin(), TBGEMAlignment->m_align.end(), [](AlignTransform a, AlignTransform b){return a.rawId() < b.rawId();});
  std::sort(TBGEMAlignmentErrorExtended->m_alignError.begin(), TBGEMAlignmentErrorExtended->m_alignError.end(), [](auto a, auto b){return a.rawId() < b.rawId();});
  edm::Service<cond::service::PoolDBOutputService> poolDbService;
  if( poolDbService.isAvailable() ) {
    poolDbService->writeOne<Alignments>(TBGEMAlignment, poolDbService->currentTime(),
                                        "GEMAlignmentRcd"  );
    poolDbService->writeOne<AlignmentErrorsExtended>(TBGEMAlignmentErrorExtended, poolDbService->currentTime(),
                                                     "GEMAlignmentErrorExtendedRcd"  );
  }
  else
    throw std::runtime_error("PoolDBService required.");
}


// ------------ method called once each job just before starting event loop  ------------
void
TBGEMRcdMaker::beginJob()
{
}

// ------------ method called once each job just after ending the event loop  ------------
void
TBGEMRcdMaker::endJob()
{
}

// ------------ method fills 'descriptions' with the allowed parameters for the module  ------------
void
TBGEMRcdMaker::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
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
DEFINE_FWK_MODULE(TBGEMRcdMaker);


#include <ostream>

#include "gemsw/Simulation/interface/TestbeamGun.h"

#include "SimDataFormats/GeneratorProducts/interface/HepMCProduct.h"
#include "SimDataFormats/GeneratorProducts/interface/GenEventInfoProduct.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/ServiceRegistry/interface/Service.h"
#include "FWCore/Utilities/interface/RandomNumberGenerator.h"

#include "CLHEP/Random/RandFlat.h"
#include "CLHEP/Random/RandGauss.h"

using namespace edm;
using namespace std;

TestbeamGun::TestbeamGun(const ParameterSet& pset) : BaseFlatGunProducer(pset)
{
   ParameterSet defpset;
   ParameterSet pgun_params = pset.getParameter<ParameterSet>("PGunParameters");
  
   fMinPt = pgun_params.getParameter<double>("MinPt");
   fMaxPt = pgun_params.getParameter<double>("MaxPt");
  
  produces<HepMCProduct>("unsmeared");
  produces<GenEventInfoProduct>();
}

TestbeamGun::~TestbeamGun()
{
   // no need to cleanup GenEvent memory - done in HepMCProduct
}


void TestbeamGun::produce(Event &e, const EventSetup& es) 
{
   edm::Service<edm::RandomNumberGenerator> rng;
   CLHEP::HepRandomEngine* engine = &rng->getEngine(e.streamID());
   if ( fVerbosity > 0 )
   {
      cout << " TestbeamGun : Begin New Event Generation" << endl ; 
   }
   
   fEvt = new HepMC::GenEvent() ;
   
   double dVx;
   double dVz;
   double dVy = 0.0;
   HepMC::GenVertex* Vtx = NULL;

   // loop over particles

   int barcode = 1 ;
   for (unsigned int ip=0; ip<fPartIDs.size(); ++ip)
   {
       double px, py, pz, mom;
       double phi, theta;
       int j = 0;
       
       int PartID = fPartIDs[ip] ;
       while (j < 10000) // j < 10000 to avoid too long computational time
       {
         if (PartID == 13)
         {
           dVx = CLHEP::RandFlat::shoot(engine, -50., 50.) ;
           dVz = CLHEP::RandFlat::shoot(engine, -50., 50.) ;
         }
         
         if (PartID == 211)
         {
           dVx = CLHEP::RandGauss::shoot(engine, 0., 12.) ;
           dVz = CLHEP::RandGauss::shoot(engine, 0., 11.) ;
         }
         
         mom   = CLHEP::RandFlat::shoot(engine, fMinPt, fMaxPt) ;
         phi   = CLHEP::RandFlat::shoot(engine, fMinPhi, fMaxPhi) ;
         theta = 0;

         if (fIsThetaFlat)
         {
             theta  = CLHEP::RandFlat::shoot(engine, fMinTheta, fMaxTheta);
         }
         
         if (!fIsThetaFlat)
         {
             double u = CLHEP::RandFlat::shoot(engine, 0.0, 0.785398); // u = Uniform[0;Pi/4]
             theta = 0;
             while(abs(u-(0.5*theta+0.25*sin(2*theta)))>0.000015)
             {
                 theta+=0.00001;
             }             
         }
         px     =  mom*sin(theta)*cos(phi) ;
         py     =  mom*sin(theta)*sin(phi) ;
         pz     =  mom*cos(theta) ;

         j++;
         
       }
       
       const HepPDT::ParticleData* 
       PData = fPDGTable->particle(HepPDT::ParticleID(abs(PartID))) ;
       double mass   = PData->mass().value() ;
       Vtx = new HepMC::GenVertex(HepMC::FourVector(dVx,dVy,dVz));

       double energy2= mom*mom + mass*mass ;
       double energy = sqrt(energy2) ;
       HepMC::FourVector p(px,py,pz,energy) ;
       HepMC::GenParticle* Part = new HepMC::GenParticle(p,PartID,1);
       Part->suggest_barcode( barcode ) ;
       barcode++ ;
       Vtx->add_particle_out(Part);

       if ( fAddAntiParticle )
       {
          HepMC::FourVector ap(-px,-py,-pz,energy) ;
	      int APartID = -PartID ;
	      if ( PartID == 22 || PartID == 23 )
	      {
	        APartID = PartID ;
	      }	  
	      HepMC::GenParticle* APart = new HepMC::GenParticle(ap,APartID,1);
	      APart->suggest_barcode( barcode ) ;
	      barcode++ ;
	      Vtx->add_particle_out(APart) ;
       }
   }

   fEvt->add_vertex(Vtx) ;
   fEvt->set_event_number(e.id().event()) ;
   fEvt->set_signal_process_id(20) ; 
        
   if ( fVerbosity > 0 )
   {
      fEvt->print() ;  
   }

   unique_ptr<HepMCProduct> BProduct(new HepMCProduct()) ;
   BProduct->addHepMCData( fEvt );
   e.put(std::move(BProduct), "unsmeared");

   unique_ptr<GenEventInfoProduct> genEventInfo(new GenEventInfoProduct(fEvt));
   e.put(std::move(genEventInfo));
    
   if ( fVerbosity > 0 )
   {
      cout << " TestbeamGun : Event Generation Done " << endl;
   }
}

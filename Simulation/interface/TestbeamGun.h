#ifndef TestbeamGun_H
#define TestbeamGun_H

/** \class TestbeamGun
 *
 * Generates single particle gun in HepMC format
 * Julia Yarba 12/2005 
 ***************************************/

#include "FWCore/PluginManager/interface/ModuleDef.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "gemsw/Simulation/interface/BaseFlatGunProducer.h"

namespace edm
{
  
  class TestbeamGun : public BaseFlatGunProducer
  {
  
  public:
    TestbeamGun(const ParameterSet & pset);
    virtual ~TestbeamGun() override;
   
    virtual void produce(Event & e, const EventSetup& es) override;

  private:
    
    // data members
    
    double            fMinPt   ;
    double            fMaxPt   ;

  };
}
#endif

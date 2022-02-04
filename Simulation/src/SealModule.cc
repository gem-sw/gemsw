#include "FWCore/Framework/interface/MakerMacros.h"
// #include "IOMC/Input/interface/MCFileSource.h"

// Julia Yarba : related to particle gun prototypes
//
//#include "IOMC/ParticleGuns/interface/FlatEGunASCIIWriter.h"

#include "gemsw/Simulation/interface/CosmicGun.h"
#include "gemsw/Simulation/interface/TestbeamGun.h"


// particle gun prototypes
//
  
  
/*
using edm::FlatEGunASCIIWriter;
DEFINE_FWK_MODULE(FlatEGunASCIIWriter);
*/

using edm::CosmicGun;
DEFINE_FWK_MODULE(CosmicGun);

using edm::TestbeamGun;
DEFINE_FWK_MODULE(TestbeamGun);


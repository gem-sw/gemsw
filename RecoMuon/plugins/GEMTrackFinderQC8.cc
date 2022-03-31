/** \class GEMTrackFinderQC8  
 * Produces a collection of tracks's in the test beam set up. 
 *
 * \author Jason Lee
 */

#ifndef GEMTrackFinderQC8_H
#define GEMTrackFinderQC8_H

#include "gemsw/RecoMuon/interface/GEMTrackFinder.h"

using namespace std;

class GEMTrackFinderQC8 : public GEMTrackFinder {
public:
  /// Constructor
  explicit GEMTrackFinderQC8(const edm::ParameterSet&);
  /// Destructor
  virtual ~GEMTrackFinderQC8() {}

private:
  void setSeedingLayers() override;
  void makeTrajectorySeeds(const GEMRecHitCollection* gemHits) override;

  MuonTransientTrackingRecHit::MuonRecHitContainer BuildSeedHits(const GEMEtaPartition* etaPart,
                                                                 const GEMRecHitCollection* gemHits);
  bool useModuleColumns_;
  array<const GEMEtaPartition*, 16> topChamb_, botChamb_;
  int nColumns_;
};

GEMTrackFinderQC8::GEMTrackFinderQC8(const edm::ParameterSet& ps) : GEMTrackFinder(ps) {
  useModuleColumns_ = ps.getParameter<bool>("useModuleColumns");
  if (useModuleColumns_) nColumns_ = 4;
  else nColumns_ = 1;
}

#endif

void GEMTrackFinderQC8::setSeedingLayers() {
  float hTop = 0;
  float hBot = 999999;
  for (auto chmap : detLayerMap_) {
    auto h = chmap.first;
    hTop = h>hTop?h:hTop;
    hBot = h<hBot?h:hBot;
  }

  for (auto chmap : detLayerMap_){    
    auto etaPart = chmap.second;
    auto etaPartId = etaPart->id();
    int ieta = etaPartId.ieta();
    int ietaIdx = 16 - ieta;
    if (chmap.first == hTop) {
      topChamb_[ietaIdx] = etaPart;
    }
    if (chmap.first == hBot) {
      botChamb_[ietaIdx] = etaPart;
    }
  } 
}

void GEMTrackFinderQC8::makeTrajectorySeeds(const GEMRecHitCollection* gemRecHits) {
  for (int col = 0; col < nColumns_; col++) {
    for (int itop = 0; itop < 16/nColumns_; itop++) {
      int idxTop = 4 * col + itop;
      auto topHits = BuildSeedHits(topChamb_[idxTop], gemRecHits);
      for (int ibot = 0; ibot < 16/nColumns_; ibot++) {
        int idxBot = 4 * col + ibot;
        auto botHits = BuildSeedHits(botChamb_[idxBot], gemRecHits);
        // Push_back trajectory candidates in trajectorySeedCands_
        makeSeedsFromHits(topHits, botHits);
      }
    }
  }
}

MuonTransientTrackingRecHit::MuonRecHitContainer GEMTrackFinderQC8::BuildSeedHits(const GEMEtaPartition* etaPart, 
                                                                                  const GEMRecHitCollection* gemHits)
{
  MuonTransientTrackingRecHit::MuonRecHitContainer hits;
  GEMDetId etaPartID = etaPart->id();
  
  GEMRecHitCollection::range range = gemHits->get(etaPartID);
  for (GEMRecHitCollection::const_iterator rechit = range.first; rechit!=range.second; ++rechit){
    if (rechit->clusterSize() > maxClusterSize_ or rechit->clusterSize() < minClusterSize_) continue;
    const GeomDet* geomDet(etaPart);
    hits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
  }
  return hits;
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GEMTrackFinderQC8);

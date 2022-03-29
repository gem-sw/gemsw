/** \class GEMTrackFinderTB  
 * Produces a collection of tracks's in the test beam set up. 
 *
 * \author Jason Lee
 */

#include "gemsw/RecoMuon/interface/GEMTrackFinder.h"

using namespace std;

class GEMTrackFinderTB : public GEMTrackFinder {
public:
  /// Constructor
  explicit GEMTrackFinderTB(const edm::ParameterSet&);
  /// Destructor
  virtual ~GEMTrackFinderTB() {}

private:
  void setSeedingLayers() override;
  void makeTrajectorySeeds(const GEMRecHitCollection* gemHits) override;
  bool checkExclude(int idx);

  MuonTransientTrackingRecHit::MuonRecHitContainer Build2DSeedHits(const GEMEtaPartition* etaPartX, 
                                                                   const GEMEtaPartition* etaPartY, 
                                                                   const GEMRecHitCollection* gemHits);

  MuonTransientTrackingRecHit::MuonRecHitContainer Build1DSeedHits(const GEMEtaPartition* etaPartX, 
                                                                   const GEMEtaPartition* etaPartY, 
                                                                   const GEMRecHitCollection* gemHits);

  bool skipLargeChamber_, use1DSeeds_, requireUniqueHit_;
  vector<int> excludingTrackers_;

  array<const GEMEtaPartition*, 4> xChamb_, yChamb_;
};

GEMTrackFinderTB::GEMTrackFinderTB(const edm::ParameterSet& ps) : GEMTrackFinder(ps) {
  skipLargeChamber_ = ps.getParameter<bool>("skipLargeChamber");
  use1DSeeds_ = ps.getParameter<bool>("use1DSeeds");
  requireUniqueHit_ = ps.getParameter<bool>("requireUniqueHit");
  excludingTrackers_ = ps.getParameter<vector<int>>("excludingTrackers");
}

void GEMTrackFinderTB::setSeedingLayers() {
  for (auto chmap : detLayerMap_){    
    auto etaPart = chmap.second;
    auto etaPartId = etaPart->id();
    int ieta = etaPartId.ieta();
    int ch = etaPartId.chamber();
    int st = etaPartId.station();
    bool isTrackingChamber = (st == 1 and ieta < 5);
    if (st == 0) excludingChambers_[etaPartId] = true;
    if ( isTrackingChamber ) {
      int idx_offset = ch/2 - 1;
      if (ieta%2 == 1) {
        int local_idx = ieta/2;
        int idx = idx_offset*2 + local_idx;
        xChamb_[idx] = etaPart;
        if (checkExclude(idx)) excludingChambers_[etaPartId] = true;
      } else {
        int local_idx = (ieta-1)/2;
        int idx = idx_offset*2 + local_idx;
        yChamb_[idx] = etaPart;
        if (checkExclude(idx)) excludingChambers_[etaPartId] = true;
      }
    } else if (skipLargeChamber_) {
      excludingChambers_[etaPartId] = true;
    }   
  } 
}

void GEMTrackFinderTB::makeTrajectorySeeds(const GEMRecHitCollection* gemRecHits) {
  if (requireUniqueHit_) {
    for (int i = 0; i < 4; i++) {
      GEMDetId etaPartID = xChamb_[i]->id();
      GEMRecHitCollection::range range = gemRecHits->get(etaPartID);
      if (range.second - range.first > 1) return;
    }
    for (int i = 0; i < 4; i++) {
      GEMDetId etaPartID = yChamb_[i]->id();
      GEMRecHitCollection::range range = gemRecHits->get(etaPartID);
      if (range.second - range.first > 1) return;
    }
  }
  if (use1DSeeds_) {
    for (int i = 0; i < 2; i++) {
      if (checkExclude(i)) continue;
      auto frontHits = Build1DSeedHits(xChamb_[i], yChamb_[i], gemRecHits);
      for (int j = 2; j < 4; j++) {
        if (checkExclude(j)) continue;
        auto rearHits = Build1DSeedHits(xChamb_[j], yChamb_[j], gemRecHits);
        // Push_back trajectory candidates in trajectorySeedCands_
        makeSeedsFromHits(frontHits, rearHits);
      }
    }
  } else {
    for (int i = 0; i < 2; i++) {
      if (checkExclude(i)) continue;
      auto frontHits = Build2DSeedHits(xChamb_[i], yChamb_[i], gemRecHits);
      for (int j = 2; j < 4; j++) {
        if (checkExclude(j)) continue;
        auto rearHits = Build2DSeedHits(xChamb_[j], yChamb_[j], gemRecHits);
        // Push_back trajectory candidates in trajectorySeedCands_
        makeSeedsFromHits(frontHits, rearHits);
      }
    }
  }
}

MuonTransientTrackingRecHit::MuonRecHitContainer GEMTrackFinderTB::Build2DSeedHits(const GEMEtaPartition* etaPartX, 
                                                                                   const GEMEtaPartition* etaPartY, 
                                                                                   const GEMRecHitCollection* gemHits)
{
  MuonTransientTrackingRecHit::MuonRecHitContainer hits;
  GEMDetId etaPartIDx = etaPartX->id();
  GEMDetId etaPartIDy = etaPartY->id();
  
  GEMRecHitCollection::range rangeX = gemHits->get(etaPartIDx);
  GEMRecHitCollection::range rangeY = gemHits->get(etaPartIDy);
  for (GEMRecHitCollection::const_iterator rechitX = rangeX.first; rechitX!=rangeX.second; ++rechitX){
    if (rechitX->clusterSize() > maxClusterSize_ or rechitX->clusterSize() < minClusterSize_) continue;
    const GeomDet* geomDet(etaPartX);
    auto localPointX = rechitX->localPosition();
    auto localErrX = rechitX->localPositionError();
    for (GEMRecHitCollection::const_iterator rechitY = rangeY.first; rechitY!=rangeY.second; ++rechitY){
      if (rechitY->clusterSize() > maxClusterSize_ or rechitY->clusterSize() < minClusterSize_) continue;
      auto localPointY = rechitY->localPosition();
      auto localErrY = rechitY->localPositionError();

      Float_t lpx = localPointX.x();
      Float_t lpy = -localPointY.x();
      Float_t lpz = localPointX.z();
      Float_t lpxx = localErrX.xx();
      Float_t lpyy = localErrY.xx();
      Float_t lpxy = 0;

      auto localPoint = LocalPoint(lpx, lpy, lpz);
      auto localError = LocalError(lpxx, lpxy, lpyy);
      auto rechit = rechitX->clone();
      rechit->setPositionAndError(localPoint, localError);

      hits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechit));
    }
  }
  return hits;
}

MuonTransientTrackingRecHit::MuonRecHitContainer GEMTrackFinderTB::Build1DSeedHits(const GEMEtaPartition* etaPartX, 
                                                                                   const GEMEtaPartition* etaPartY, 
                                                                                   const GEMRecHitCollection* gemHits)
{
  MuonTransientTrackingRecHit::MuonRecHitContainer hits;
  GEMDetId etaPartIDx = etaPartX->id();
  GEMDetId etaPartIDy = etaPartY->id();
  
  GEMRecHitCollection::range rangeX = gemHits->get(etaPartIDx);
  GEMRecHitCollection::range rangeY = gemHits->get(etaPartIDy);
  for (GEMRecHitCollection::const_iterator rechitX = rangeX.first; rechitX!=rangeX.second; ++rechitX){
    if (rechitX->clusterSize() > maxClusterSize_ or rechitX->clusterSize() < minClusterSize_) continue;
    const GeomDet* geomDet(etaPartX);
    hits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechitX));
  }
  for (GEMRecHitCollection::const_iterator rechitY = rangeY.first; rechitY!=rangeY.second; ++rechitY){
    if (rechitY->clusterSize() > maxClusterSize_ or rechitY->clusterSize() < minClusterSize_) continue;
    const GeomDet* geomDet(etaPartY);
    hits.push_back(MuonTransientTrackingRecHit::specificBuild(geomDet,&*rechitY));
  }
  return hits;
}

bool GEMTrackFinderTB::checkExclude(int idx) {
  return find(excludingTrackers_.begin(), excludingTrackers_.end(), idx) != excludingTrackers_.end();
}

#include "FWCore/Framework/interface/MakerMacros.h"
DEFINE_FWK_MODULE(GEMTrackFinderTB);

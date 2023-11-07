#ifndef GEMTRACKFINDER_H
#define GEMTRACKFINDER_H
/** \class GEMTrackFinder  
 * Produces a collection of tracks's non-p5 gem set up. 
 *
 * \author Jason Lee / Yechan Kang
 */

#include "FWCore/Framework/interface/Frameworkfwd.h"
#include "FWCore/Framework/interface/stream/EDProducer.h"
#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ConsumesCollector.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"
#include "FWCore/Utilities/interface/InputTag.h"
#include "FWCore/MessageLogger/interface/MessageLogger.h"

#include "MagneticField/Engine/interface/MagneticField.h"
#include "MagneticField/Records/interface/IdealMagneticFieldRecord.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"
#include "Geometry/GEMGeometry/interface/GEMGeometry.h"
#include "Geometry/CommonTopologies/interface/GEMStripTopology.h"

#include "DataFormats/Common/interface/Handle.h"
#include "DataFormats/GEMRecHit/interface/GEMRecHitCollection.h"

#include "DataFormats/TrackReco/interface/Track.h"
#include "DataFormats/TrajectorySeed/interface/PropagationDirection.h"
#include "DataFormats/TrajectorySeed/interface/TrajectorySeed.h"
#include "DataFormats/TrackingRecHit/interface/TrackingRecHit.h"
#include "DataFormats/TrackingRecHit/interface/InvalidTrackingRecHit.h"

#include "RecoMuon/TransientTrackingRecHit/interface/MuonTransientTrackingRecHit.h"
#include "RecoMuon/TrackingTools/interface/MuonServiceProxy.h"
#include "RecoMuon/StandAloneTrackFinder/interface/StandAloneMuonSmoother.h"
#include "TrackingTools/TrajectoryState/interface/TrajectoryStateTransform.h"
#include "TrackingTools/KalmanUpdators/interface/Chi2MeasurementEstimator.h"
#include "TrackingTools/KalmanUpdators/interface/KFUpdator.h"
#include "RecoTracker/TrackProducer/src/TrajectoryToResiduals.h"

#include "Geometry/Records/interface/MuonGeometryRecord.h"

using namespace std;

class GEMTrackFinder : public edm::stream::EDProducer<> {
public:
  /// Constructor
  explicit GEMTrackFinder(const edm::ParameterSet&);
  /// Destructor
  virtual ~GEMTrackFinder() {}
  /// Produce the GEMSegment collection
  void beginRun(const edm::Run&, const edm::EventSetup&) override;
  void produce(edm::Event&, const edm::EventSetup&) override;

private:
  StandAloneMuonSmoother* theSmoother_;
  MuonServiceProxy* theService_;

  virtual void setSeedingLayers() {};
  virtual void makeTrajectorySeeds(const GEMRecHitCollection* gemHits) {};

  Trajectory makeTrajectory(TrajectorySeed& seed, const GEMRecHitCollection* gemHits);
  TrackingRecHit::ConstRecHitContainer findMissingHits(Trajectory& track);

  float alongDirection(const GlobalPoint&);

  double trackChi2_;
  bool doFit_;
  vector<double> direction_;


protected:
  edm::EDGetTokenT<GEMRecHitCollection> theGEMRecHitToken_;
  edm::ESGetToken<GEMGeometry, MuonGeometryRecord> gemg_;

  void makeSeedsFromHits(MuonTransientTrackingRecHit::MuonRecHitContainer frontHits,
                         MuonTransientTrackingRecHit::MuonRecHitContainer rearHits);

  int maxClusterSize_, minClusterSize_;
  double residualCut_;
  multimap<float,const GEMEtaPartition*> detLayerMap_;
  vector<TrajectorySeed> trajectorySeedCands_;
  map<GEMDetId, bool> excludingChambers_;
};

#endif

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')
options.register('include20x10',
                 False,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 "Include 20x10 chamber in the geometry")
options.register('excludeChambers',
                 [],
                 VarParsing.VarParsing.multiplicity.list,
                 VarParsing.VarParsing.varType.int,
                 "Exclude the chamber from track reconstruction (0 to 3)")
options.parseArguments()

process = cms.Process("GEMStreamSource")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)

process.options = cms.untracked.PSet(
    wantSummary=cms.untracked.bool(True),
    SkipEvent=cms.untracked.vstring('ProductNotFound'),
)

debug = False
#debug = True
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cout.threshold = cms.untracked.string('INFO')
process.MessageLogger.debugModules = cms.untracked.vstring('*')
if debug:
    process.MessageLogger.cerr.threshold = "DEBUG"
    process.MessageLogger.debugModules = ["source", "muonGEMDigis"]
    process.maxEvents.input = cms.untracked.int32(100)
else:
    process.MessageLogger.cerr.FwkReport.reportEvery = 5000

process.source = cms.Source("GEMStreamSource",
                            fileNames=cms.untracked.vstring(
                                options.inputFiles),
                            firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}))

print(options.inputFiles)

# this block ensures that the output collection is named rawDataCollector, not source
process.rawDataCollector = cms.EDAlias(source=cms.VPSet(
    cms.PSet(type=cms.string('FEDRawDataCollection'))))

process.load('EventFilter.GEMRawToDigi.muonGEMDigis_cfi')
process.muonGEMDigis.InputLabel = cms.InputTag("rawDataCollector")
process.muonGEMDigis.fedIdStart = cms.uint32(1477)
process.muonGEMDigis.fedIdEnd = cms.uint32(1478)
process.muonGEMDigis.skipBadStatus = cms.bool(True)
process.muonGEMDigis.useDBEMap = True

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.load('gemsw.Geometry.GeometryTestBeam_cff')
process.gemGeometry.applyAlignment = cms.bool(True)

if options.include20x10 :
    process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMeMapRcd"),
                                                 tag=cms.string("GEMeMapTestBeam"),
                                                 connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMeMap_TestBeam_with_20x10.db")),
                                        cms.PSet(
                                            record = cms.string('GEMAlignmentRcd'),
                                            tag = cms.string("TBGEMAlignment_test"),
                                            connect = cms.string("sqlite_fip:gemsw/Alignment/data/Alignment_Standalone_2021.db")),
                                        cms.PSet(
                                            record = cms.string('GEMAlignmentErrorExtendedRcd'),
                                            tag = cms.string("TBGEMAlignmentErrorExtended_test"),
                                            connect = cms.string("sqlite_fip:gemsw/Alignment/data/Alignment_Standalone_2021.db")),
                                        cms.PSet(record=cms.string('GlobalPositionRcd'), tag = cms.string('IdealGeometry'))
   )
else :
    process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMeMapRcd"),
                                                 tag=cms.string("GEMeMapTestBeam"),
                                                 connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMeMap_TestBeam.db")),
                                        cms.PSet(
                                            record = cms.string('GEMAlignmentRcd'),
                                            tag = cms.string("TBGEMAlignment_test"),
                                            connect = cms.string("sqlite_fip:gemsw/Alignment/data/Alignment_Standalone_2021.db")),
                                        cms.PSet(
                                            record = cms.string('GEMAlignmentErrorExtendedRcd'),
                                            tag = cms.string("TBGEMAlignmentErrorExtended_test"),
                                            connect = cms.string("sqlite_fip:gemsw/Alignment/data/Alignment_Standalone_2021.db")),
                                        cms.PSet(record=cms.string('GlobalPositionRcd'), tag = cms.string('IdealGeometry'))
    )
process.load('MagneticField.Engine.uniformMagneticField_cfi')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('RecoMuon.TrackingTools.MuonServiceProxy_cff')
process.MuonServiceProxy.ServiceParameters.Propagators.append('StraightLinePropagator')
process.load('TrackPropagation.SteppingHelixPropagator.SteppingHelixPropagatorAny_cfi')
process.SteppingHelixPropagatorAny.useMagVolumes = cms.bool(False)

process.GEMTrackFinder = cms.EDProducer("GEMTrackFinder",
                                        process.MuonServiceProxy,
                                        gemRecHitLabel = cms.InputTag("gemRecHits"),
                                        maxClusterSize = cms.int32(10),
                                        minClusterSize = cms.int32(1),
                                        trackChi2 = cms.double(1000.0),
                                        skipLargeChamber = cms.bool(True),
                                        use1DSeeds = cms.bool(False), 
                                        excludingChambers = cms.vint32(options.excludeChambers),
                                        MuonSmootherParameters = cms.PSet(
                                           PropagatorAlong = cms.string('SteppingHelixPropagatorAny'),
                                           PropagatorOpposite = cms.string('SteppingHelixPropagatorAny'),
                                           RescalingFactor = cms.double(5.0)
                                        ),
                                        )
process.GEMTrackFinder.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.GEMTrackFinder.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.GEMTrackFinder.ServiceParameters.RPCLayers = cms.bool(False)

process.load("CommonTools.UtilAlgos.TFileService_cfi")
process.TestBeamTrackAnalyzer = cms.EDAnalyzer("TestBeamTrackAnalyzer",
                                               process.MuonServiceProxy,
                                               gemRecHitLabel = cms.InputTag("gemRecHits"),
                                               tracks = cms.InputTag("GEMTrackFinder", "", "GEMStreamSource"),
                                               seeds = cms.InputTag("GEMTrackFinder", "", "GEMStreamSource"),
                                               )
process.TestBeamTrackAnalyzer.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.TestBeamTrackAnalyzer.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.TestBeamTrackAnalyzer.ServiceParameters.RPCLayers = cms.bool(False)

process.output = cms.OutputModule("PoolOutputModule",
                                  outputCommands=cms.untracked.vstring(
                                      "keep *",
                                      "drop FEDRawDataCollection_source_*_*"
                                  ),
                                  fileName=cms.untracked.string(
                                      'output_edm.root'),
)

process.load("DQM.Integration.config.environment_cfi")
process.load('DQM.GEM.GEMDQM_cff')

process.dqmEnv.subSystemFolder = "GEM"
process.dqmEnv.eventInfoFolder = "EventInfo"
process.dqmSaver.path = ""
process.dqmSaver.tag = "GEM"

process.unpack = cms.Path(process.muonGEMDigis)
process.reco = cms.Path(process.gemRecHits * process.GEMTrackFinder)
process.track_ana = cms.Path(process.TestBeamTrackAnalyzer)
process.dqm = cms.Path(process.GEMDQM)
process.dqm.remove(process.GEMDAQStatusSource)
process.dqmout = cms.EndPath(process.dqmEnv + process.dqmSaver)
process.outpath = cms.EndPath(process.output)

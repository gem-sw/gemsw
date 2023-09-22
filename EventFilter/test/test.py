import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')
options.parseArguments()

process = cms.Process("GEMStreamSource")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents),
    #input = cms.untracked.int32(10000),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)

process.options = cms.untracked.PSet(
    wantSummary=cms.untracked.bool(False),
    SkipEvent=cms.untracked.vstring('ProductNotFound'),
)

debug = False
process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cout.threshold = cms.untracked.string('INFO')
process.MessageLogger.debugModules = cms.untracked.vstring('*')
if debug:
    process.MessageLogger.cerr.threshold = "DEBUG"
    process.MessageLogger.debugModules = ["source", "muonGEMDigis"]
    process.maxEvents.input = cms.untracked.int32(100)
#else:
#    process.MessageLogger.cerr.FwkReport.reportEvery = 5000

process.source = cms.Source("GEMStreamSource",
                            fileNames=cms.untracked.vstring(
                            options.inputFiles),
                            firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}))
#process.source = cms.Source("NewEventStreamFileReader",
#                            fileNames=cms.untracked.vstring(
#                            options.inputFiles),)

print(options.inputFiles)

# this block ensures that the output collection is named rawDataCollector, not source
process.rawDataCollector = cms.EDAlias(source=cms.VPSet(
    cms.PSet(type=cms.string('FEDRawDataCollection'))))

process.load('EventFilter.GEMRawToDigi.muonGEMDigis_cfi')
process.muonGEMDigis.InputLabel = cms.InputTag("rawDataCollector")
process.muonGEMDigis.fedIdStart = cms.uint32(10)
process.muonGEMDigis.fedIdEnd = cms.uint32(11)
process.muonGEMDigis.isP5data = False
process.muonGEMDigis.useDBEMap = True
process.load('RecoLocalMuon.GEMRecHit.gemRecHits_cfi')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.load('gemsw.Geometry.GeometryQC8GE21_back_cff')
process.gemGeometry.applyAlignment = cms.bool(True)

process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMChMapRcd"),
                                             tag=cms.string("GEMChMapRcd"),
                                             connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMChMap_QC8.db")),
                                    cms.PSet(record = cms.string('GEMAlignmentRcd'),
                                             tag = cms.string("QC8GEMAlignment_test"),
                                             connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign.db")),
                                    cms.PSet(record = cms.string('GEMAlignmentErrorExtendedRcd'),
                                             tag = cms.string("QC8GEMAlignmentErrorExtended_test"),
                                             connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign.db")),
                                    cms.PSet(record=cms.string('GlobalPositionRcd'), tag = cms.string('IdealGeometry'))
)

process.load('Configuration.StandardSequences.Services_cff')
process.load('MagneticField.Engine.uniformMagneticField_cfi')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('RecoMuon.TrackingTools.MuonServiceProxy_cff')
process.MuonServiceProxy.ServiceParameters.Propagators.append('StraightLinePropagator')
process.load('TrackPropagation.SteppingHelixPropagator.SteppingHelixPropagatorAny_cfi')
process.SteppingHelixPropagatorAny.useMagVolumes = cms.bool(False)

process.GEMTrackFinder = cms.EDProducer("GEMTrackFinderQC8",
                                        process.MuonServiceProxy,
                                        gemRecHitLabel = cms.InputTag("gemRecHits"),
                                        maxClusterSize = cms.int32(10),
                                        minClusterSize = cms.int32(1),
                                        trackChi2 = cms.double(1.2),
                                        direction = cms.vdouble(0,0,1),
                                        topSeedingChamber = cms.int32(7),
                                        botSeedingChamber = cms.int32(2),
                                        useModuleColumns = cms.bool(True),
                                        doFit = cms.bool(True),
                                        MuonSmootherParameters = cms.PSet(
                                           #Propagator = cms.string('SteppingHelixPropagatorAny'),
                                           Propagator = cms.string('StraightLinePropagator'),
                                           ErrorRescalingFactor = cms.double(5.0),
                                           MaxChi2 = cms.double(1000.0),
                                           NumberOfSigma = cms.double(3),
                                        ),
                                        )
process.GEMTrackFinder.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.GEMTrackFinder.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.GEMTrackFinder.ServiceParameters.RPCLayers = cms.bool(False)

process.DQMDAQ = DQMEDAnalyzer("QC8DAQStatusSource")
process.DQMRecHit = DQMEDAnalyzer("QC8RecHitSource")

process.TrackValidation = DQMEDAnalyzer("QC8TrackValidation",
                                        process.MuonServiceProxy,
                                        gemRecHitLabel = cms.InputTag("gemRecHits"),
                                        tracks = cms.InputTag("GEMTrackFinder"),
                                        trajs = cms.InputTag("GEMTrackFinder"),
                                        )

process.load('Configuration.EventContent.EventContent_cff')
process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('DQMIO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:track_inDQM.root'),
    outputCommands = process.DQMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

process.output = cms.OutputModule("PoolOutputModule",
                                  dataset = cms.untracked.PSet(
                                      dataTier = cms.untracked.string('RECO'),
                                      filterName = cms.untracked.string('')
                                  ),
                                  outputCommands=cms.untracked.vstring(
                                      "keep *",
                                      "drop FEDRawDataCollection_*_*_*"
                                  ),
                                  fileName=cms.untracked.string('output_step1.root'),
                                  splitLevel = cms.untracked.int32(0)
)

process.load("DQM.Integration.config.environment_cfi")
process.dqmEnv.subSystemFolder = "GEM"
process.dqmEnv.eventInfoFolder = "EventInfo"
process.dqmSaver.path = ""
process.dqmSaver.tag = "GEM"

process.unpack = cms.Path(process.muonGEMDigis)
process.localreco = cms.Path(process.gemRecHits)
process.reco_step = cms.Path(process.GEMTrackFinder)
process.validation_step = cms.Path(process.TrackValidation)
process.dqmout_step = cms.EndPath(process.DQMoutput)
process.DQM_step = cms.Path(process.DQMDAQ*process.DQMRecHit)
process.dqmout = cms.EndPath(process.dqmEnv + process.dqmSaver)
process.outpath = cms.EndPath(process.output)

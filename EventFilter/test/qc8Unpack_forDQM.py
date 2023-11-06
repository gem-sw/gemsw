import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')
options.parseArguments()

process = cms.Process("GEMStreamSource")

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(options.maxEvents),
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
isBackTypeOnly = True

if isBackTypeOnly :
  process.load('gemsw.Geometry.GeometryQC8GE21_back_cff')
  process.gemGeometry.applyAlignment = cms.bool(True)
  
  process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMChMapRcd"),
                                               tag=cms.string("GEMChMapRcd"),
                                               connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMChMap_QC8.db")),
                                      cms.PSet(record = cms.string('GEMAlignmentRcd'),
                                               tag = cms.string("QC8GEMAlignment_test"),
                                               connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign_backTypeOnly.db")),
                                      cms.PSet(record = cms.string('GEMAlignmentErrorExtendedRcd'),
                                               tag = cms.string("QC8GEMAlignmentErrorExtended_test"),
                                               connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign_backTypeOnly.db")),
                                      cms.PSet(record=cms.string('GlobalPositionRcd'), tag = cms.string('IdealGeometry'))
  )
else :
  process.load('gemsw.Geometry.GeometryQC8GE21_front_cff')
  process.gemGeometry.applyAlignment = cms.bool(True)
  
  process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMChMapRcd"),
                                               tag=cms.string("GEMChMapRcd"),
                                               connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMChMap_QC8.db")),
                                      cms.PSet(record = cms.string('GEMAlignmentRcd'),
                                               tag = cms.string("QC8GEMAlignment_test"),
                                               connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign_frontTypeOnly.db")),
                                      cms.PSet(record = cms.string('GEMAlignmentErrorExtendedRcd'),
                                               tag = cms.string("QC8GEMAlignmentErrorExtended_test"),
                                               connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign_frontTypeOnly.db")),
                                      cms.PSet(record=cms.string('GlobalPositionRcd'), tag = cms.string('IdealGeometry'))
  )

process.DQMDAQ = DQMEDAnalyzer("QC8DAQStatusSource")
process.DQMRecHit = DQMEDAnalyzer("QC8RecHitSource")

process.load("DQM.Integration.config.environment_cfi")
process.dqmEnv.subSystemFolder = "GEM"
process.dqmEnv.eventInfoFolder = "EventInfo"
process.dqmSaver.path = ""
process.dqmSaver.tag = "GEM"

process.unpack = cms.Path(process.muonGEMDigis)
process.localreco = cms.Path(process.gemRecHits)
process.DQM_step = cms.Path(process.DQMDAQ*process.DQMRecHit)
process.dqmout = cms.EndPath(process.dqmEnv + process.dqmSaver)

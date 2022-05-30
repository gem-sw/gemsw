import FWCore.ParameterSet.Config as cms
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
                            fedId = cms.untracked.int32(12),
                            fedId2 = cms.untracked.int32(10),
                            fileNames=cms.untracked.vstring(options.inputFiles),
                            firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}))

print(options.inputFiles)

# this block ensures that the output collection is named rawDataCollector, not source
process.rawDataCollector = cms.EDAlias(source=cms.VPSet(
    cms.PSet(type=cms.string('FEDRawDataCollection'))))

process.load('EventFilter.GEMRawToDigi.muonGEMDigis_cfi')
process.muonGEMDigis.InputLabel = cms.InputTag("rawDataCollector")
process.muonGEMDigis.fedIdStart = cms.uint32(10)
process.muonGEMDigis.fedIdEnd = cms.uint32(12)
process.muonGEMDigis.skipBadStatus = cms.bool(False)
process.muonGEMDigis.useDBEMap = True

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.load('gemsw.Geometry.GeometryTestBeam2022_cff')
#process.gemGeometry.applyAlignment = cms.bool(True)

process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMeMapRcd"),
                                             tag=cms.string("GEMeMapTestBeam"),
                                             connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMeMap_TestBeam_2022.db")),
)

process.load('Configuration.StandardSequences.Reconstruction_cff')

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

process.load("CommonTools.UtilAlgos.TFileService_cfi")
process.TestBeamHitAnalyzer = cms.EDAnalyzer("TestBeamHitAnalyzer",
                                             gemRecHitLabel = cms.InputTag("gemRecHits"),
                                             gemDigiLabel = cms.InputTag("muonGEMDigis"),
                                             )

process.unpack = cms.Path(process.muonGEMDigis)
process.reco = cms.Path(process.gemRecHits)
process.hit_ana = cms.Path(process.TestBeamHitAnalyzer)
process.outpath = cms.EndPath(process.output)

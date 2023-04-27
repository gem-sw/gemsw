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
process.muonGEMDigis.fedIdStart = cms.uint32(1479)
process.muonGEMDigis.fedIdEnd = cms.uint32(1479)
# process.muonGEMDigis.skipBadStatus = cms.bool(True) # CMSSW_12_3_2 does not have skipBadStatus option 
process.muonGEMDigis.useDBEMap = True

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
from Configuration.AlCa.GlobalTag import GlobalTag
process.load('gemsw.Geometry.GeometryQC8GE21_cff')
process.gemGeometry.applyAlignment = cms.bool(True)

process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMChMapRcd"),
                                             tag=cms.string("GEMChMapQC8_v0"),
                                             connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMChMap_qc8.db")),
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

process.unpack = cms.Path(process.muonGEMDigis)
process.outpath = cms.EndPath(process.output)

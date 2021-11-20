import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')
options.parseArguments()

process = cms.Process("AMC13SpyReadout")

process.maxEvents = cms.untracked.PSet(
    #input = cms.untracked.int32(options.maxEvents)
    input=cms.untracked.int32(-1)
)

process.options = cms.untracked.PSet(
    wantSummary=cms.untracked.bool(True),
    SkipEvent=cms.untracked.vstring('ProductNotFound'),
)

process.load("FWCore.MessageLogger.MessageLogger_cfi")
process.MessageLogger.cout.threshold = cms.untracked.string('INFO')
process.MessageLogger.debugModules = cms.untracked.vstring('*')
process.MessageLogger.cerr.FwkReport.reportEvery = 5000
process.MessageLogger.cerr.threshold = "DEBUG"
process.MessageLogger.debugModules = ["muonGEMDigis"]

process.source = cms.Source("GEMStreamSource",
                            fileNames=cms.untracked.vstring(
                                options.inputFiles),
                            verifyAdler32=cms.untracked.bool(False),
                            verifyChecksum=cms.untracked.bool(False),
                            useL1EventID=cms.untracked.bool(False),
                            firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID(
                                *[cms.LuminosityBlockID(1, 0)]),
                            rawDataLabel=cms.untracked.string("GEM"))
# print the input file
print(options.inputFiles)

# this block ensures that the output collection is named rawDataCollector, not source
process.rawDataCollector = cms.EDAlias(source=cms.VPSet(
    cms.PSet(type=cms.string('FEDRawDataCollection'))))

process.load('EventFilter.GEMRawToDigi.muonGEMDigis_cfi')
process.muonGEMDigis.InputLabel = cms.InputTag("rawDataCollector")
process.muonGEMDigis.fedIdStart = cms.uint32(1477)
process.muonGEMDigis.fedIdEnd = cms.uint32(1478)
process.muonGEMDigis.skipBadStatus = cms.bool(False)
process.muonGEMDigis.useDBEMap = True

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMeMapRcd"),
                                             tag=cms.string("GEMeMapTestBeam"),
                                             connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMeMap_TestBeam_simple_me0.db")))

process.output = cms.OutputModule("PoolOutputModule",
                                  outputCommands=cms.untracked.vstring(
                                      "drop *",
                                      "drop FEDRawDataCollection_source_*_*"
                                  ),
                                  fileName=cms.untracked.string(
                                      'output_edm.root'),
                                  dataset=cms.untracked.PSet(
                                      dataTier=cms.untracked.string('GEN-SIM'),
                                      filterName=cms.untracked.string('')
                                  ),
                                  splitLevel=cms.untracked.int32(0))

process.p = cms.Path(process.muonGEMDigis)
process.outpath = cms.EndPath(process.output)

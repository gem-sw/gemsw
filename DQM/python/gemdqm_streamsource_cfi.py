import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')

options.register('runNumber',
                 210605,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Run number.')

options.register('runInputDir',
                 '/data',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 'Run input directory')

options.register('streamLabel',
                 'ge21qc8',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 'Stream label')

options.register('scanOnce',
                 False,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 'Do the file scan only once')

options.register('updir',
                 './',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 'upload directory')

options.parseArguments()

if not option.inputFiles:
    source = cms.Source('GEMStreamReader'
                        fedId = cms.untracked.int32(10),
                        fedId2 = cms.untracked.int32(11),
                        fileNames = cms.untracked.vstring(options.inputFiles),
                        firstRun =  cms.untracked.uint32(options.runNumber),
                        firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}),
                        runNumber = cms.untracked.uint32(options.runNumber),
                        runInputDir =  cms.untracked.string(options.runInputDir),
                        streamLabel =  cms.untracked.string(options.streamLabel),
                        minEventsPerFile = cms.untracked.uint32(10000),
                        scanOnce = cms.untracked.bool(options.scanOnce),
                        delayMillis= cms.untracked.uint32(500),
                        nextEntryTimeoutMillis = cms.untracked.int32(200000))

else:
    source = cms.Source("GEMStreamSource",
                        fedId = cms.untracked.int32(10),
                        fedId2 = cms.untracked.int32(11),
                        fileNames = cms.untracked.vstring(options.inputFiles),
                        firstRun =  cms.untracked.uint32(options.runNumber),
                        firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}))

print("Source settings", source)

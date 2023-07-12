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

options.register('secFile',
                 False,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.bool,
                 'Stream has secondary file')

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

options.register('minEventsPerFile',
                 10000,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 'Minimum number of events to process per file')

options.parseArguments()

if not options.inputFiles:
    source = cms.Source('GEMStreamReader',
                        runNumber = cms.untracked.uint32(options.runNumber),
                        runInputDir =  cms.untracked.string(options.runInputDir),
                        streamLabel =  cms.untracked.string(options.streamLabel),
                        secFile = cms.untracked.bool(options.secFile),
                        fedId = cms.untracked.int32(10),
                        fedId2 = cms.untracked.int32(11),
                        fileNames = cms.untracked.vstring(options.inputFiles),
                        firstRun =  cms.untracked.uint32(options.runNumber),
                        firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}),
                        minEventsPerFile = cms.untracked.uint32(options.minEventsPerFile),
                        scanOnce = cms.untracked.bool(options.scanOnce),
                        delayMillis= cms.untracked.uint32(500),
                        nextEntryTimeoutMillis = cms.untracked.int32(200000))

else:
    source = cms.Source('GEMStreamSource',
                        fedId = cms.untracked.int32(10),
                        fedId2 = cms.untracked.int32(11),
                        fileNames = cms.untracked.vstring(options.inputFiles),
                        firstRun =  cms.untracked.uint32(options.runNumber),
                        firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}))

print('Source settings', source)

import FWCore.ParameterSet.Config as cms
import FWCore.ParameterSet.VarParsing as VarParsing

options = VarParsing.VarParsing('analysis')

options.register('runNumber',
                 210605,
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.int,
                 "Run number.")

options.register('updir',
                 '',
                 VarParsing.VarParsing.multiplicity.singleton,
                 VarParsing.VarParsing.varType.string,
                 'upload directory')

options.parseArguments()

source = cms.Source("GEMStreamSource",
                    fedId = cms.untracked.int32(10),
                    fedId2 = cms.untracked.int32(11),
                    fileNames =cms.untracked.vstring(options.inputFiles),
                    firstRun =  cms.untracked.uint32(options.runNumber),
                    firstLuminosityBlockForEachRun=cms.untracked.VLuminosityBlockID({}))

# TODO
# Add GEMStreamReader if not options.inputFiles

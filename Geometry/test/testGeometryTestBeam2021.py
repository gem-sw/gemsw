import FWCore.ParameterSet.Config as cms

process = cms.Process('TestBeam')

# import of standard configurations
#process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
#process.load('Configuration.EventContent.EventContent_cff')
#process.load('Configuration.StandardSequences.GeometryRecoDB_cff')
process.load('gemsw.Geometry.GeometryTestBeam2021_cff')
#process.load('Configuration.StandardSequences.MagneticField_0T_cff')
#process.load('Configuration.StandardSequences.Reconstruction_cff')
#process.load('Configuration.StandardSequences.RecoSim_cff')
#process.load('Configuration.StandardSequences.EndOfProcess_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)
process.source = cms.Source("EmptySource")

process.test = cms.EDAnalyzer("GEMGeometryAnalyzer")

process.p = cms.Path(process.test)
process.MessageLogger.cerr.threshold = "DEBUG"
process.MessageLogger.debugModules = ["gemGeometry"]

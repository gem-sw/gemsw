import FWCore.ParameterSet.Config as cms

process = cms.Process("MyGEMAlignmentRcdRetrieveTest")

process.load("Geometry.MuonNumbering.muonNumberingInitialization_cfi")
process.load("Geometry.GEMGeometryBuilder.gemGeometry_cfi")
process.load('gemsw.Geometry.GeometryTestBeam_cff')
process.load('Configuration.StandardSequences.CondDBESSource_cff')

#process.load("Geometry.GEMGeometryBuilder.gemGeometry_cfi")

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

# Need to get global offsets
from Configuration.AlCa.GlobalTag import GlobalTag


process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.source = cms.Source("EmptySource")

process.test = cms.EDAnalyzer("GEMTestGeometryAnalyzer")

process.path = cms.Path(process.test)

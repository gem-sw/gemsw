import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process("ALTEST",eras.phase2_muon)


process.load('gemsw.Geometry.GeometryTestBeam_cff')

process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

from Configuration.AlCa.GlobalTag import GlobalTag
process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')

process.GlobalTag.toGet = cms.VPSet(
    cms.PSet(
        record = cms.string('GEMAlignmentRcd'),
        tag = cms.string("TBGEMAlignment_test"),
        #connect = cms.string("sqlite:MyAlignment.db"),
        connect = cms.string("sqlite_fip:gemsw/Alignment/data/MyAlignment.db"),
    ),
    cms.PSet(
        record = cms.string('GEMAlignmentErrorExtendedRcd'),
        tag = cms.string("TBGEMAlignmentErrorExtended_test"),
        connect = cms.string("sqlite_fip:gemsw/Alignment/data/MyAlignment.db"),
    )
)

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1)
)

process.source = cms.Source("EmptySource")

process.test = cms.EDAnalyzer("ReadAli")

process.path = cms.Path(process.test)

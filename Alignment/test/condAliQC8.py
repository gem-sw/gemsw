import FWCore.ParameterSet.Config as cms

process = cms.Process("TBGEMAlignmentRcdWriter")

# Load CondDB service                                                                                                                                    
process.load("CondCore.CondDB.CondDB_cfi")

process.load("Geometry.MuonNumbering.muonNumberingInitialization_cfi")
process.load("Geometry.GEMGeometryBuilder.gemGeometry_cfi")
process.load('gemsw.Geometry.GeometryQC8GE21_front_cff')
process.load('Configuration.StandardSequences.CondDBESSource_cff')

process.load("Geometry.GEMGeometryBuilder.gemGeometry_cfi")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#process.GEMGeometryESModule.applyAlignment = cms.bool(False)


# output database (in this case local sqlite file)                                                                                                       
process.OutDB = process.CondDB.clone()
process.OutDB.connect = 'sqlite_file:QC8_GE21_FakeAlign_frontTypeOnly.db'

# A data source must always be defined. We don't need it, so here's a dummy one.                                                                         
process.source = cms.Source("EmptyIOVSource",
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    lastValue = cms.uint64(1),
    interval = cms.uint64(1)
)


# We define the output service.                                                                                                                          
process.PoolDBOutputService = cms.Service("PoolDBOutputService",
    process.OutDB,
    timetype = cms.untracked.string('runnumber'),
    toPut = cms.VPSet(cms.PSet(
        record = cms.string('GEMAlignmentRcd'),
        tag = cms.string('QC8GEMAlignment_test')
    ),
    cms.PSet(
        record = cms.string('GEMAlignmentErrorExtendedRcd'),
        tag = cms.string('QC8GEMAlignmentErrorExtended_test')
    ))
)

process.tbgemrcd = cms.EDAnalyzer("QC8GEMRcdMaker",
    loggingOn= cms.untracked.bool(True),
    SinceAppendMode=cms.bool(True),
    Source=cms.PSet(
        IOVRun=cms.untracked.uint32(1)
    )
)

process.path = cms.Path(process.tbgemrcd)

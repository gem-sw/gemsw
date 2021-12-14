import FWCore.ParameterSet.Config as cms

process = cms.Process("TBGEMAlignmentRcdWriter")

# Load CondDB service                                                                                                                                    
process.load("CondCore.CondDB.CondDB_cfi")

process.load("Geometry.MuonNumbering.muonNumberingInitialization_cfi")
process.load("Geometry.GEMGeometryBuilder.gemGeometry_cfi")
process.load('gemsw.Geometry.GeometryTestBeam_cff')
process.load('Configuration.StandardSequences.CondDBESSource_cff')

process.load("Geometry.GEMGeometryBuilder.gemGeometry_cfi")
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')

#process.GEMGeometryESModule.applyAlignment = cms.bool(False)


# output database (in this case local sqlite file)                                                                                                       
process.OutDB = process.CondDB.clone()
process.OutDB.connect = 'sqlite_file:MyAlignment.db'

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
        tag = cms.string('TBGEMAlignment_test')
    ),
    # cms.PSet(                                                                                                                                          
    #     record = cms.string('GEMAlignmentErrorRcd'),                                                                                                   
    #     tag = cms.string('myGEMAlignment_test')                                                                                                        
    # ),                                                                                                                                                 
    cms.PSet(
        record = cms.string('GEMAlignmentErrorExtendedRcd'),
        tag = cms.string('TBGEMAlignmentErrorExtended_test')
    ))
)

process.tbgemrcd = cms.EDAnalyzer("TBGEMRcdMaker",
    xShift1=cms.double(+0.0269035),
    yShift1=cms.double(+0.206244),
    phiShift1=cms.double(0.),
    xShift2=cms.double(-0.0292939),
    yShift2=cms.double(+0.0207079),
    phiShift2=cms.double(0.),
    xShift3=cms.double(+0.0332708),
    yShift3=cms.double(-0.0577936),
    phiShift3=cms.double(0.),
    xShift4=cms.double(-0.00905448),
    yShift4=cms.double(-0.0108215),
    phiShift4=cms.double(0.),
    loggingOn= cms.untracked.bool(True),
    SinceAppendMode=cms.bool(True),
    Source=cms.PSet(
        IOVRun=cms.untracked.uint32(1)
    )
)

process.path = cms.Path(process.tbgemrcd)

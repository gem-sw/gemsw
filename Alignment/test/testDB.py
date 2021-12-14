import FWCore.ParameterSet.Config as cms

process = cms.Process("MyGEMAlignmentRcdRetrieveTest")

process.load("CondCore.CondDB.CondDB_cfi")
# input database (in this case the local sqlite file)
process.CondDB.connect = 'sqlite_fip:gemsw/CondAli/python/MyAlignment.db'

process.PoolDBESSource = cms.ESSource("PoolDBESSource",
    process.CondDB,
    DumpStat=cms.untracked.bool(True),
    toGet = cms.VPSet(cms.PSet(
        record = cms.string('GEMAlignmentRcd'),
        tag = cms.string("TBGEMAlignment_test"),
        data = cms.vstring('Alignments')
    ),
    # cms.PSet(
    #     record = cms.string('GEMAlignmentErrorRcd'),
    #     tag = cms.string("myGEMAlignment_test"),
    #     data = cms.vstring('AlignmentErrors')
    # ),
                      
    cms.PSet(
        record = cms.string('GEMAlignmentErrorExtendedRcd'),
        tag = cms.string("TBGEMAlignmentErrorExtended_test"),
        data = cms.vstring('AlignmentErrorsExtended')
    )
                      
    ),
)

process.get = cms.EDAnalyzer("EventSetupRecordDataGetter",
    toGet = cms.VPSet(
        cms.PSet(
            record = cms.string('GEMAlignmentRcd'),
            data = cms.vstring('Alignments')
        ),
        cms.PSet(
            record = cms.string('GEMAlignmentErrorExtendedRcd'),
            data = cms.vstring('AlignmentErrorsExtended')
        ),
    ),
    verbose = cms.untracked.bool(True)
)

# A data source must always be defined. We don't need it, so here's a dummy one.
process.source = cms.Source("EmptyIOVSource",
    timetype = cms.string('runnumber'),
    firstValue = cms.uint64(1),
    lastValue = cms.uint64(1),
    interval = cms.uint64(1)
)

process.path = cms.Path(process.get)

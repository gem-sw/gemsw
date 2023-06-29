import FWCore.ParameterSet.Config as cms

XMLIdealGeometryESSource = cms.ESSource("XMLIdealGeometryESSource",
    geomXMLFiles = cms.vstring(
        'gemsw/Geometry/data/materials.xml',
        'gemsw/Geometry/data/cms.xml',
        'gemsw/Geometry/data/cmsMother.xml',
        'gemsw/Geometry/data/muonBase.xml',
        'gemsw/Geometry/data/cmsMuon.xml',
        'gemsw/Geometry/data/QC8GE21/mf.xml',
        'gemsw/Geometry/data/gemf.xml',
        'gemsw/Geometry/data/QC8GE21/gem21.xml',
        'gemsw/Geometry/data/QC8GE21/muonNumbering.xml',
        'gemsw/Geometry/data/muonSens.xml',
        ## custom stand
        'gemsw/Geometry/data/GEMSpecs.xml',
        'gemsw/Geometry/data/GEMSpecsFilter.xml',
        'gemsw/Geometry/data/QC8GE21/mf.xml',
        'gemsw/Geometry/data/QC8GE21/backstand.xml',
    ),
    rootNodeName = cms.string('cms:OCMS')
)

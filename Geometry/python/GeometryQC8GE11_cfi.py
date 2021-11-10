import FWCore.ParameterSet.Config as cms

XMLIdealGeometryESSource = cms.ESSource("XMLIdealGeometryESSource",
    geomXMLFiles = cms.vstring(
        'gemsw/Geometry/data/materials.xml',
        'gemsw/Geometry/data/rotations.xml',
        'gemsw/Geometry/data/cms.xml',
        'gemsw/Geometry/data/cmsMother.xml',
        'gemsw/Geometry/data/etaMax.xml',
        'gemsw/Geometry/data/muonBase.xml',
        'gemsw/Geometry/data/cmsMuon.xml',
        'gemsw/Geometry/data/QC8GE11/mf.xml',
        'gemsw/Geometry/data/gemf.xml',
        'gemsw/Geometry/data/gem11.xml',
        'gemsw/Geometry/data/muonNumbering.xml',
        'gemsw/Geometry/data/muonSens.xml',
        'gemsw/Geometry/data/GEMSpecsFilter.xml',
        'gemsw/Geometry/data/GEMSpecs.xml',
        'gemsw/Geometry/data/QC8GE11/gem11S_c2_r1.xml',
        'gemsw/Geometry/data/QC8GE11/gem11S_c2_r3.xml',
        'gemsw/Geometry/data/QC8GE11/gem11S_c2_r5.xml',
    ),
    rootNodeName = cms.string('cms:OCMS')
)

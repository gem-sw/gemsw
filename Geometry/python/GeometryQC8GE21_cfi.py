import FWCore.ParameterSet.Config as cms

# This config was generated automatically using generate2026Geometry.py
# If you notice a mistake, please update the generating script, not just this config

XMLIdealGeometryESSource = cms.ESSource("XMLIdealGeometryESSource",
    geomXMLFiles = cms.vstring(
        'gemsw/Geometry/data/materials.xml',
        'gemsw/Geometry/data/rotations.xml',
        'gemsw/Geometry/data/cms.xml',
        'gemsw/Geometry/data/cmsMother.xml',
        'gemsw/Geometry/data/etaMax.xml',
        'gemsw/Geometry/data/muonBase.xml',
        'gemsw/Geometry/data/cmsMuon.xml',
        'gemsw/Geometry/data/mf.xml',
        'gemsw/Geometry/data/gemf.xml',
        'gemsw/Geometry/data/gem21.xml',
        'gemsw/Geometry/data/muonNumbering.xml',
        'gemsw/Geometry/data/muonSens.xml',
        'gemsw/Geometry/data/qc8/GEMSpecsFilter.xml',
        'gemsw/Geometry/data/qc8/GEMSpecs.xml',
        ## testbeam
        'gemsw/Geometry/data/qc8/qc8GE21.xml',
    ),
    rootNodeName = cms.string('cms:OCMS')
)

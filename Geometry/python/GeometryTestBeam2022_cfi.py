import FWCore.ParameterSet.Config as cms

XMLIdealGeometryESSource = cms.ESSource("XMLIdealGeometryESSource",
    geomXMLFiles = cms.vstring(
        'gemsw/Geometry/data/materials.xml',
        'gemsw/Geometry/data/cms.xml',
        'gemsw/Geometry/data/cmsMother.xml',
        'gemsw/Geometry/data/muonBase.xml',
        'gemsw/Geometry/data/cmsMuon.xml',
        'gemsw/Geometry/data/testbeam2022/mf.xml',
        'gemsw/Geometry/data/gemf.xml',
        'gemsw/Geometry/data/testbeam2022/gem11.xml',        
        'gemsw/Geometry/data/testbeam2022/ge0.xml',
        'gemsw/Geometry/data/testbeam2022/gem21.xml',
        ## testbeam2022
        'gemsw/Geometry/data/testbeam2022/tbGE11.xml',
        'gemsw/Geometry/data/testbeam2022/tbGE0.xml',
        'gemsw/Geometry/data/testbeam2022/tbGE21.xml',
        'gemsw/Geometry/data/testbeam2022/muonNumbering.xml',
        'gemsw/Geometry/data/muonSens.xml',
        'gemsw/Geometry/data/GEMSpecsFilter.xml',
        'gemsw/Geometry/data/testbeam2022/GEMSpecs.xml',
    ),
    rootNodeName = cms.string('cms:OCMS')
)

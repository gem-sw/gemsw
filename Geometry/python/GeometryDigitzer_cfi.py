import FWCore.ParameterSet.Config as cms

XMLIdealGeometryESSource = cms.ESSource("XMLIdealGeometryESSource",
    geomXMLFiles = cms.vstring(
        'gemsw/Geometry/data/materials.xml',
        'gemsw/Geometry/data/cms.xml',
        'gemsw/Geometry/data/cmsMother.xml',
        'gemsw/Geometry/data/muonBase.xml',
        'gemsw/Geometry/data/cmsMuon.xml',
        'gemsw/Geometry/data/digitzer/mf.xml',
        'gemsw/Geometry/data/gemf.xml',
        'gemsw/Geometry/data/digitzer/gem11.xml',
        'gemsw/Geometry/data/digitzer/gem21.xml',
        'gemsw/Geometry/data/digitzer/ge0.xml',
        'gemsw/Geometry/data/muonNumbering.xml',
        'gemsw/Geometry/data/muonSens.xml',
        'gemsw/Geometry/data/GEMSpecsFilter.xml',
        'gemsw/Geometry/data/GEMSpecs.xml',
        #'gemsw/Geometry/data/digitzer/ge21box.xml',
    ),
    rootNodeName = cms.string('cms:OCMS')
)

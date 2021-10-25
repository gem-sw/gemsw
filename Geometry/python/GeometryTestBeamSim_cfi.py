import FWCore.ParameterSet.Config as cms

# This config was generated automatically using generate2026Geometry.py
# If you notice a mistake, please update the generating script, not just this config

XMLIdealGeometryESSource = cms.ESSource("XMLIdealGeometryESSource",
    geomXMLFiles = cms.vstring(
        'Geometry/CMSCommonData/data/materials/2021/v1/materials.xml',
        'Geometry/CMSCommonData/data/rotations.xml',
        'Geometry/CMSCommonData/data/extend/v2/cmsextent.xml',
        'Geometry/CMSCommonData/data/cavernData/2021/v1/cavernData.xml',
        'Geometry/CMSCommonData/data/cms/2026/v5/cms.xml',
        'Geometry/CMSCommonData/data/cmsMother.xml',
        'Geometry/CMSCommonData/data/eta3/etaMax.xml',
        'gemsw/Geometry/data/muonBase.xml',
        'Geometry/CMSCommonData/data/cmsMuon.xml',
        'gemsw/Geometry/data/mf.xml',
        'Geometry/MuonCommonData/data/gemf/TDR_BaseLine/gemf.xml',
        'gemsw/Geometry/data/testbeam/gem11.xml',
        'gemsw/Geometry/data/gem21.xml',
        #'Geometry/MuonCommonData/data/mfshield/2026/v5/mfshield.xml',
        'gemsw/Geometry/data/ge0.xml',
    )+
    cms.vstring(
        'Geometry/MuonSimData/data/PhaseII/v2/muonSens.xml',
        'Geometry/GEMGeometryBuilder/data/v12/GEMSpecsFilter.xml',
        'Geometry/GEMGeometryBuilder/data/v12/GEMSpecs.xml',
        ## testbeam
        'gemsw/Geometry/data/testbeam/muonNumbering.xml',
        #'gemsw/Geometry/data/testbeam/tbGE11.xml',
        'gemsw/Geometry/data/testbeam/tbGE0.xml',
        'gemsw/Geometry/data/testbeam/tbGE21.xml',
    ),
    rootNodeName = cms.string('cms:OCMS')
)

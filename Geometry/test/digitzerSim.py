import FWCore.ParameterSet.Config as cms

process = cms.Process('test')

process.load('Configuration.StandardSequences.Services_cff')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.MagneticField_0T_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedGauss_cfi')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')

process.load('gemsw.Geometry.GeometryDigitzer_cff')
process.XMLIdealGeometryESSource.geomXMLFiles.append('gemsw/Geometry/data/digitzer/ge11box.xml')
#process.XMLIdealGeometryESSource.geomXMLFiles.append('gemsw/Geometry/data/digitzer/ge21box.xml')
#process.XMLIdealGeometryESSource.geomXMLFiles.append('gemsw/Geometry/data/digitzer/ge0box.xml')

process.source = cms.Source("EmptySource")

process.maxEvents = cms.untracked.PSet(input = cms.untracked.int32(1000))
# test beam detectors at y-axis
process.generator = cms.EDFilter("Pythia8EGun",
    PGunParameters = cms.PSet(
        AddAntiParticle = cms.bool(True),
        MaxE = cms.double(100.0),
        MinE = cms.double(0.1),
        MaxEta = cms.double(0.001),
        MinEta = cms.double(0.00),
        MaxPhi = cms.double(1.58),
        MinPhi = cms.double(1.57),
        ParticleID = cms.vint32(13)
    ),
    PythiaParameters = cms.PSet(
        parameterSets = cms.vstring()
    ),
    Verbosity = cms.untracked.int32(0),
    firstRun = cms.untracked.uint32(1),
    psethack = cms.string('Ten mu e 0 to 200')
)

process.g4SimHits.UseMagneticField = cms.bool(False)
process.g4SimHits.OnlySDs = cms.vstring('MuonSensitiveDetector')

process.mix = cms.EDProducer("MixingModule",
    digitizers = cms.PSet(),
    # LabelPlayback = cms.string(''),
    maxBunch = cms.int32(3),
    minBunch = cms.int32(-5), ## in terms of 25 ns
    bunchspace = cms.int32(450),
    mixProdStep1 = cms.bool(False),
    mixProdStep2 = cms.bool(False),
    # playback = cms.untracked.bool(False),
    # useCurrentProcessOnly = cms.bool(False),
    mixObjects = cms.PSet(
        mixSH = cms.PSet(
            crossingFrames = cms.untracked.vstring('MuonGEMHits',),
            input = cms.VInputTag(cms.InputTag("g4SimHits","MuonGEMHits")),
            type = cms.string('PSimHit'),
            subdets = cms.vstring('MuonGEMHits'),
        ),
    ),
)

process.FEVTDEBUGoutput = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('generation_step')
    ),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:step1.root'),
    outputCommands = cms.untracked.vstring( (
        'drop *',
        'keep *_simMuonGEMDigis_*_*',
        'keep *_genParticles_*_*',
        'keep *_g4SimHits_MuonGEMHits_*',
    )),
    splitLevel = cms.untracked.int32(0)
)

process.generation_step = cms.Path(process.generator+process.pgen)
process.simulation_step = cms.Path(process.psim)
process.digitisation_step = cms.Path(process.mix+process.simMuonGEMDigis)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGoutput_step = cms.EndPath(process.FEVTDEBUGoutput)

process.schedule = cms.Schedule(process.generation_step,process.simulation_step,
        process.digitisation_step,process.endjob_step,process.FEVTDEBUGoutput_step)

process.RandomNumberGeneratorService.simMuonGEMDigis = process.RandomNumberGeneratorService.generator

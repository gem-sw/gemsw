import FWCore.ParameterSet.Config as cms
from Configuration.Eras.Era_Phase2C11I13M9_cff import Phase2C11I13M9
process = cms.Process('SIM',Phase2C11I13M9)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('SimGeneral.MixingModule.mixNoPU_cfi')
process.load('Configuration.Geometry.GeometryExtended2026D76Reco_cff')
process.load('Configuration.Geometry.GeometryExtended2026D76_cff')
process.load('Configuration.StandardSequences.MagneticField_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedHLLHC_cfi')
process.load('GeneratorInterface.Core.genFilterSummary_cff')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.DigiToRaw_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')

process.load('gemsw.Geometry.GeometryTestBeam_cff')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100),
    output = cms.optional.untracked.allowed(cms.int32,cms.PSet)
)
process.source = cms.Source("EmptySource")

process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('TenMuExtendedE_0_200_pythia8_cfi nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
)

process.FEVTDEBUGEventContent.outputCommands = cms.untracked.vstring( (
        'drop *',
        'keep FEDRawDataCollection_rawDataCollector_*_*',
        'keep *_gemRecHits_*_*',
        'keep *_gemSegments_*_*',
        'keep *_muonGEMDigis_*_*',
        'keep *DigiSimLinkedmDetSetVector_simMuonGEMDigis_*_*',
        'keep *_simMuonGEMDigis_*_*',
        'keep *_genParticles_*_*',
))
process.FEVTDEBUGoutput = cms.OutputModule("PoolOutputModule",
    SelectEvents = cms.untracked.PSet(
        SelectEvents = cms.vstring('generation_step')
    ),
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('GEN-SIM'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:step1.root'),
    outputCommands = process.FEVTDEBUGEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)

process.genstepfilter.triggerConditions=cms.vstring("generation_step")

process.generator = cms.EDFilter("Pythia8EGun",
    PGunParameters = cms.PSet(
        AddAntiParticle = cms.bool(False),
        MaxE = cms.double(200.0),
        MaxEta = cms.double(0.0),
        MaxPhi = cms.double(1.48),
        MinE = cms.double(0.0),
        MinEta = cms.double(0.0),
        MinPhi = cms.double(1.47),
        ParticleID = cms.vint32(13)
    ),
    PythiaParameters = cms.PSet(
        parameterSets = cms.vstring()
    ),
    Verbosity = cms.untracked.int32(0),
    firstRun = cms.untracked.uint32(1),
    psethack = cms.string('Ten mu e 0 to 200')
)

process.mix = cms.EDProducer("MixingModule",
    LabelPlayback = cms.string(''),
    bunchspace = cms.int32(450),
    maxBunch = cms.int32(3),
    minBunch = cms.int32(-5),
    mixProdStep1 = cms.bool(False),
    mixProdStep2 = cms.bool(False),
    playback = cms.untracked.bool(False),
    useCurrentProcessOnly = cms.bool(False),
    digitizers = cms.PSet(),    
    mixObjects = cms.PSet(
        mixSH = cms.PSet(
            crossingFrames = cms.untracked.vstring('MuonGEMHits'),
            input = cms.VInputTag(cms.InputTag("g4SimHits","MuonGEMHits")),
            type = cms.string('PSimHit'),
            subdets = cms.vstring('MuonGEMHits'),            
            )
        ),
    mixTracks = cms.PSet(
        input = cms.VInputTag(cms.InputTag("g4SimHits")),
        makeCrossingFrame = cms.untracked.bool(True),
        type = cms.string('SimTrack')
    ),
)

process.g4SimHits.UseMagneticField = cms.bool(False)
process.g4SimHits.OnlySDs = cms.vstring('MuonSensitiveDetector')

process.gemPacker.useDBEMap = False
process.muonGEMDigis.readMultiBX = True
process.muonGEMDigis.useDBEMap = process.gemPacker.useDBEMap
process.muonGEMDigis.keepDAQStatus = True

#process.pgen = cms.Sequence(process.randomEngineStateProducer*process.generatorSmeared+process.genParticles)
process.generation_step = cms.Path(process.pgen)
process.simulation_step = cms.Path(process.psim)
process.genfiltersummary_step = cms.EndPath(process.genFilterSummary)
process.digitisation_step = cms.Path(process.mix+process.simMuonGEMDigis)
process.digi_step = cms.Path(process.gemPacker+process.rawDataCollector+process.muonGEMDigis)

process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGoutput_step = cms.EndPath(process.FEVTDEBUGoutput)

#process.schedule = cms.Schedule(process.generation_step,process.genfiltersummary_step,process.simulation_step,process.endjob_step,process.FEVTDEBUGoutput_step)
process.schedule = cms.Schedule(process.generation_step,process.simulation_step,#process.digitisation_step,
process.endjob_step,process.FEVTDEBUGoutput_step)

for path in process.paths:
	getattr(process,path).insert(0, process.generator)

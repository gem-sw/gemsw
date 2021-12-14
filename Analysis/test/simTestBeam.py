import FWCore.ParameterSet.Config as cms
from Configuration.StandardSequences.Eras import eras
process = cms.Process('tb',eras.phase2_muon)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedGauss_cfi')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.DigiToRaw_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
# test beam detectors at y-axis - GE21 at (0,110*cm,0), GE0 at (0,120*cm,0)
process.load('gemsw.Geometry.GeometryTestBeam_cff')

#process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
#from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:phase1_2021_realistic', '')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(100000),
)
process.source = cms.Source("EmptySource")
process.configurationMetadata = cms.untracked.PSet(
    annotation = cms.untracked.string('TenMuExtendedE_0_200_pythia8_cfi nevts:10'),
    name = cms.untracked.string('Applications'),
    version = cms.untracked.string('$Revision: 1.19 $')
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
        'keep FEDRawDataCollection_rawDataCollector_*_*',
        'keep *_gemRecHits_*_*',
        'keep *_gemSegments_*_*',
        'keep *_muonGEMDigis_*_*',
        'keep *DigiSimLinkedmDetSetVector_simMuonGEMDigis_*_*',
        'keep *_simMuonGEMDigis_*_*',
        'keep *_genParticles_*_*',
        'keep *_g4SimHits_MuonGEMHits_*',
        'keep *_g4SimHits__*',
        'keep *_GEMTrackFinder__*',
    )),
    splitLevel = cms.untracked.int32(0)
)

process.TFileService = cms.Service("TFileService",fileName = cms.string("histo.root"))

# test beam detectors at y-axis
process.generator = cms.EDFilter("Pythia8EGun",
    PGunParameters = cms.PSet(
        AddAntiParticle = cms.bool(False),
        MaxE = cms.double(20.0),
        MinE = cms.double(9.0),
        MaxEta = cms.double(0.002),
        MinEta = cms.double(0.00),        
        MaxPhi = cms.double(1.56),
        MinPhi = cms.double(1.58),
        ParticleID = cms.vint32(13)#211
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
#process.g4SimHits.Physics.CutsPerRegion = cms.bool(False)

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

process.g4SimHits.UseMagneticField = cms.bool(False)
process.gemPacker.useDBEMap = False

process.rawDataCollector.RawCollectionList = cms.VInputTag(cms.InputTag("gemPacker"))

process.load('MagneticField.Engine.uniformMagneticField_cfi')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('RecoMuon.TrackingTools.MuonServiceProxy_cff')
process.MuonServiceProxy.ServiceParameters.Propagators.append('StraightLinePropagator')
process.load('TrackPropagation.SteppingHelixPropagator.SteppingHelixPropagatorAny_cfi')
process.SteppingHelixPropagatorAny.useMagVolumes = cms.bool(False)

process.GEMTrackFinder = cms.EDProducer("GEMTrackFinder",
                                        process.MuonServiceProxy,
                                        gemRecHitLabel = cms.InputTag("gemRecHits"),
                                        maxClusterSize = cms.int32(10),
                                        minClusterSize = cms.int32(1),
                                        trackChi2 = cms.double(1000.0),
                                        skipLargeChamber = cms.bool(True),
                                        excludingChambers = cms.vint32(1),
                                        use1DSeeds = cms.bool(False),
                                        MuonSmootherParameters = cms.PSet(
                                           PropagatorAlong = cms.string('SteppingHelixPropagatorAny'),
                                           PropagatorOpposite = cms.string('SteppingHelixPropagatorAny'),
                                           RescalingFactor = cms.double(5.0)
                                        ),
                                        )
process.GEMTrackFinder.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.GEMTrackFinder.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.GEMTrackFinder.ServiceParameters.RPCLayers = cms.bool(False)

process.TestBeamTrackAnalyzer = cms.EDAnalyzer("TestBeamTrackAnalyzer",
                                               process.MuonServiceProxy,
                                               gemRecHitLabel = cms.InputTag("gemRecHits"),
                                               tracks = cms.InputTag("GEMTrackFinder"),
                                               )
                                                  

process.muonGEMDigis.readMultiBX = True
process.muonGEMDigis.useDBEMap = process.gemPacker.useDBEMap
process.muonGEMDigis.keepDAQStatus = True
process.gemRecHits.gemDigiLabel = cms.InputTag('simMuonGEMDigis')

process.generation_step = cms.Path(process.generator+process.pgen)
process.simulation_step = cms.Path(process.psim)
process.digitisation_step = cms.Path(process.mix+process.simMuonGEMDigis)
process.digi2raw_step = cms.Path(process.gemPacker+process.rawDataCollector+process.muonGEMDigis)
process.reconstruction_step = cms.Path(process.gemRecHits * process.GEMTrackFinder)
process.analyser_step = cms.Path(process.TestBeamTrackAnalyzer)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGoutput_step = cms.EndPath(process.FEVTDEBUGoutput)

process.schedule = cms.Schedule(process.generation_step,process.simulation_step,
process.digitisation_step,
#process.digi2raw_step,
process.reconstruction_step,
process.analyser_step,
process.endjob_step,process.FEVTDEBUGoutput_step)

process.RandomNumberGeneratorService.simMuonGEMDigis = process.RandomNumberGeneratorService.generator

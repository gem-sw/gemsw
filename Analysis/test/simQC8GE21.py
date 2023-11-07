import FWCore.ParameterSet.Config as cms
from DQMServices.Core.DQMEDAnalyzer import DQMEDAnalyzer
from DQMServices.Core.DQMEDHarvester import DQMEDHarvester
from Configuration.StandardSequences.Eras import eras
process = cms.Process('qc8',eras.phase2_muon)

# import of standard configurations
process.load('Configuration.StandardSequences.Services_cff')
process.load('SimGeneral.HepPDTESSource.pythiapdt_cfi')
process.load('FWCore.MessageService.MessageLogger_cfi')
process.load('Configuration.EventContent.EventContent_cff')
process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
process.load('Configuration.StandardSequences.Generator_cff')
process.load('IOMC.EventVertexGenerators.VtxSmearedGauss_cfi')
process.load('Configuration.StandardSequences.SimIdeal_cff')
process.load('Configuration.StandardSequences.EndOfProcess_cff')
process.load('Configuration.StandardSequences.Digi_cff')
process.load('Configuration.StandardSequences.DigiToRaw_cff')
process.load('Configuration.StandardSequences.RawToDigi_cff')
process.load("Configuration.StandardSequences.Reconstruction_cff")
#process.load("DQM.Integration.config.environment_cfi")
process.load('gemsw.Geometry.GeometryQC8GE21_back_cff')

#process.load('Configuration.StandardSequences.FrontierConditions_GlobalTag_cff')
#from Configuration.AlCa.GlobalTag import GlobalTag
#process.GlobalTag = GlobalTag(process.GlobalTag, 'auto:run2_mc', '')

process.maxEvents = cms.untracked.PSet(
    input = cms.untracked.int32(1000),
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

process.DQMoutput = cms.OutputModule("DQMRootOutputModule",
    dataset = cms.untracked.PSet(
        dataTier = cms.untracked.string('DQMIO'),
        filterName = cms.untracked.string('')
    ),
    fileName = cms.untracked.string('file:step1_inDQM.root'),
    outputCommands = process.DQMEventContent.outputCommands,
    splitLevel = cms.untracked.int32(0)
)


# Cosmic Muon generator
process.generator = cms.EDProducer("CosmicGun",
    AddAntiParticle = cms.bool(True),
    PGunParameters = cms.PSet(
        MinPt = cms.double(99.99),
        MaxPt = cms.double(100.01),
        MinPhi = cms.double(3.141592),
        MaxPhi = cms.double(-3.141592),
        MinTheta = cms.double(1.570796),
        MaxTheta = cms.double(3.141592),
        IsThetaFlat = cms.bool(False), # If 'True': theta distribution is flat. If 'False': theta distribution is a cos^2
        PartID = cms.vint32(-13)
    ),
    Verbosity = cms.untracked.int32(0),
    firstRun = cms.untracked.uint32(1),
    psethack = cms.string('single mu pt 100')
)

process.g4SimHits.UseMagneticField = cms.bool(False)
process.g4SimHits.OnlySDs = cms.vstring('MuonSensitiveDetector')
process.g4SimHits.Physics.CutsPerRegion = cms.bool(False)

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

process.gemPacker.useDBEMap = cms.bool(True)
process.gemPacker.fedIdStart = cms.uint32(1479)
process.gemPacker.fedIdEnd = cms.uint32(1479)

process.rawDataCollector.RawCollectionList = cms.VInputTag(cms.InputTag("gemPacker"))

from Configuration.AlCa.GlobalTag import GlobalTag
process.gemGeometry.applyAlignment = cms.bool(True)

process.GlobalTag.toGet = cms.VPSet(cms.PSet(record=cms.string("GEMChMapRcd"),
                                             tag=cms.string("GEMChMapRcd"),
                                             connect=cms.string("sqlite_fip:gemsw/EventFilter/data/GEMChMap_QC8_dummy.db")),
                                    cms.PSet(record = cms.string('GEMAlignmentRcd'),
                                             tag = cms.string("QC8GEMAlignment_test"),
                                             connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign.db")),
                                    cms.PSet(record = cms.string('GEMAlignmentErrorExtendedRcd'),
                                             tag = cms.string("QC8GEMAlignmentErrorExtended_test"),
                                             connect = cms.string("sqlite_fip:gemsw/Geometry/data/QC8GE21/QC8_GE21_FakeAlign.db")),
                                    cms.PSet(record=cms.string('GlobalPositionRcd'), tag = cms.string('IdealGeometry'))
)

process.muonGEMDigis.readMultiBX = True
process.muonGEMDigis.useDBEMap = process.gemPacker.useDBEMap
process.muonGEMDigis.fedIdStart = process.gemPacker.fedIdStart
process.muonGEMDigis.fedIdEnd = process.gemPacker.fedIdEnd
process.muonGEMDigis.keepDAQStatus = True
process.muonGEMDigis.isP5data = False
process.gemRecHits.gemDigiLabel = cms.InputTag('muonGEMDigis')

process.load('MagneticField.Engine.uniformMagneticField_cfi')
process.load('Configuration.StandardSequences.Reconstruction_cff')
process.load('RecoMuon.TrackingTools.MuonServiceProxy_cff')
process.MuonServiceProxy.ServiceParameters.Propagators.append('StraightLinePropagator')
process.load('TrackPropagation.SteppingHelixPropagator.SteppingHelixPropagatorAny_cfi')
process.SteppingHelixPropagatorAny.useMagVolumes = cms.bool(False)

process.GEMTrackFinder = cms.EDProducer("GEMTrackFinderQC8",
                                        process.MuonServiceProxy,
                                        gemRecHitLabel = cms.InputTag("gemRecHits"),
                                        maxClusterSize = cms.int32(10),
                                        minClusterSize = cms.int32(1),
                                        trackChi2 = cms.double(1000.0),
                                        residual = cms.double(100.),
                                        direction = cms.vdouble(0,0,1),
                                        topSeedingChamber = cms.int32(7),
                                        botSeedingChamber = cms.int32(1),
                                        useModuleColumns = cms.bool(True),
                                        doFit = cms.bool(True),
                                        MuonSmootherParameters = cms.PSet(
                                           #Propagator = cms.string('SteppingHelixPropagatorAny'),
                                           Propagator = cms.string('StraightLinePropagator'),
                                           ErrorRescalingFactor = cms.double(5.0),
                                           MaxChi2 = cms.double(1000.0),
                                           NumberOfSigma = cms.double(3),
                                        ),
                                        )
process.GEMTrackFinder.ServiceParameters.GEMLayers = cms.untracked.bool(True)
process.GEMTrackFinder.ServiceParameters.CSCLayers = cms.untracked.bool(False)
process.GEMTrackFinder.ServiceParameters.RPCLayers = cms.bool(False)

process.load("CommonTools.UtilAlgos.TFileService_cfi")
process.TrackAnalyzer = cms.EDAnalyzer("QC8TrackAnalyzer",
                                       process.MuonServiceProxy,
                                       gemRecHitLabel = cms.InputTag("gemRecHits"),
                                       tracks = cms.InputTag("GEMTrackFinder"),
                                       trajs = cms.InputTag("GEMTrackFinder"),
                                       )
#process.HitValidation = DQMEDAnalyzer("QC8HitValidation",
#                                      gemSimHitLabel = cms.InputTag("g4SimHits", "MuonGEMHits"),
#                                      gemRecHitLabel = cms.InputTag("gemRecHits"),
#                                      )
#process.TrackValidation = DQMEDAnalyzer("QC8TrackValidation",
#                                        process.MuonServiceProxy,
#                                        gemRecHitLabel = cms.InputTag("gemRecHits"),
#                                        tracks = cms.InputTag("GEMTrackFinder"),
#                                        trajs = cms.InputTag("GEMTrackFinder"),
#                                        )
process.DQMDAQ = DQMEDAnalyzer("QC8DAQStatusSource")
process.DQMRecHit = DQMEDAnalyzer("QC8RecHitSource")
process.DQMHarvestor = DQMEDHarvester("QC8DQMHarvestor")

#process.Harvester = DQMEDHarvester("QC8Harvestor")

#process.dqmEnv.subSystemFolder = "GEM"
#process.dqmEnv.eventInfoFolder = "EventInfo"
#process.dqmSaver.path = ""
#process.dqmSaver.tag = "GEM"

process.generation_step = cms.Path(process.generator+process.pgen)
process.simulation_step = cms.Path(process.psim)
process.digitisation_step = cms.Path(process.mix+process.simMuonGEMDigis)
process.digi2raw_step = cms.Path(process.gemPacker+process.rawDataCollector+process.muonGEMDigis)
process.localreco_step = cms.Path(process.gemRecHits)
process.reco_step = cms.Path(process.GEMTrackFinder)
process.analyzer_step = cms.Path(process.TrackAnalyzer)
#process.validation_step = cms.Path(process.HitValidation*process.TrackValidation+process.Harvester)
#process.DQM_step = cms.Path(process.DQMDAQ*process.DQMRecHit)
#process.DQMoutput_step = cms.EndPath(process.DQMoutput)
process.endjob_step = cms.EndPath(process.endOfProcess)
process.FEVTDEBUGoutput_step = cms.EndPath(process.FEVTDEBUGoutput)
#process.dqmout = cms.EndPath(process.dqmEnv + process.dqmSaver)

process.schedule = cms.Schedule(process.generation_step,
process.simulation_step,
process.digitisation_step,
process.digi2raw_step,
process.localreco_step,
process.reco_step,
process.analyzer_step,
#process.DQM_step,
#process.DQMoutput_step,
#process.dqmout,)
process.endjob_step,
process.FEVTDEBUGoutput_step)

process.RandomNumberGeneratorService.simMuonGEMDigis = process.RandomNumberGeneratorService.generator


#process.MessageLogger.G4cout=dict()
#process.MessageLogger.G4cerr=dict()
#process.MessageLogger.SimG4CoreApplication=dict()
#process.MessageLogger.SimG4CoreGeometry=dict()

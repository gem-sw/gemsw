# gemsw
QC8 and testbeam 

instructions for setting up
```bash
cmsrel CMSSW_13_0_9
cd CMSSW_13_0_9/src
cmsenv
git cms-init -q
git clone git@github.com:gem-sw/gemsw.git
scram b -j10
```

# GE21 QC8 simulation
instruction for GE21 QC8 simulation
```bash
cmsrel CMSSW_13_0_9
cd CMSSW_13_0_9/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:QC8Unpacker_13_0_X -q
git clone git@github.com:gem-sw/gemsw.git
scram b -j10
cd gemsw/Analysis/test
cmsRun simQC8GE21.py
cmsRun qc8_HARVESTING.py
```

# GE21 QC8 unpacking
instruction for GE21 QC8 data unpacking (allowed to unpack the compressed data)
```bash
cmsrel CMSSW_13_0_9
cd CMSSW_13_0_9/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:QC8Unpacker_13_0_X -q
git clone git@github.com:gem-sw/gemsw.git
scram b -j10
cd gemsw/EventFilter/test
cmsRun qc8Unpack.py inputFiles=file:<file path a>,file:<file path b> maxEvents=<number of events to analyze>
cmsRun qc8_HARVESTING.py
```

# GE21 QC8 unpacking with data directory
instruction for GE21 QC8 data unpacking (allowed to unpack the compressed data)
```bash
cmsrel CMSSW_13_0_9
cd CMSSW_13_0_9/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:QC8Unpacker_13_0_X -q
git clone git@github.com:gem-sw/gemsw.git
scram b -j10
cd gemsw/EventFilter/test
python3 runQC8Unapck_onDir.py -d <path to the dierectory with data>
cmsRun qc8_HARVESTING.py
```
`DQM_V0001_R000000001__Global__CMSSW_X_Y_Z__RECO.root` will contain the efficiency and residual, occupancies.

# The below instructions are out of date

# testbeam unpacking
instructions for unpacking the data from testbeam set up
```bash
cmsrel CMSSW_12_3_5_patch1
cd CMSSW_12_3_5_patch1/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:testBeam_safety_12_3_X # Once this feature is merged we don't need to do that
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/EventFilter/test
cmsRun gemTestBeam.py inputFiles=file:/store/data/testbeam/run_20211103_0159-0-0.raw include20x10=<True or False>
```
# reading in dual raw files
```bash
cmsrel CMSSW_12_3_5_patch1
cd CMSSW_12_3_5_patch1/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:testBeam_safety_12_3_X # Once this feature is merged we don't need to do that
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/EventFilter/test
cmsRun gemTestBeam.py inputFiles=file:/store/data/testbeam/run_20211103_0159-0-0.raw,file:/store/data/testbeam/run_20211103_0159-1-0.raw include20x10=<True or False>
```

# Unpacking 2022 TestBeam data
```bash
cmsrel CMSSW_12_3_5_patch1
cd CMSSW_12_3_5_patch1/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:testBeam_safety_12_3_X # Once this feature is merged we don't need to do that
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/EventFilter/test
cmsRun gemTestBeam2022_step1.py inputFiles=file:<file1 path>,<file2 path>
```

# fireworks
```bash
cmsrel CMSSW_12_3_5_patch1
cd CMSSW_12_3_5_patch1/src
git cms-merge-topic jshlee:fireworks-GEMOnly-CMSSW_12_2_X
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/Analysis/test

cmsRun dumpSimGeometry_cfg.py tag=GEMTB
cmsRun dumpRecoGeometry_cfg.py tag=GEMTB calo=0 tracker=0 muon=0 gem=1 tgeo=0
cmsRun dumpRecoGeometry_cfg.py tag=GEMTB calo=0 tracker=0 muon=0 gem=1 tgeo=1
```

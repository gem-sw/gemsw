# gemsw
QC8 and testbeam 

instructions for setting up
```bash
cmsrel CMSSW_12_2_0_pre1
cd CMSSW_12_2_0_pre1/src
git clone git@github.com:gem-sw/gemsw.git
scram b -j10
```

# testbeam unpacking
instructions for unpacking the data from testbeam set up
```bash
cmsrel CMSSW_12_2_0_pre1
cd CMSSW_12_2_0_pre1/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:testbeam_2022 # Will be updated
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/EventFilter/test
cmsRun gemTestBeam.py inputFiles=file:/store/data/testbeam/run_20211103_0159-0-0.raw include20x10=<True or False>
```
# reading in dual raw files
```bash
cmsrel CMSSW_12_2_0_pre1
cd CMSSW_12_2_0_pre1/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:testbeam_2022 # Will be updated
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/EventFilter/test
cmsRun gemTestBeam.py inputFiles=file:/store/data/testbeam/run_20211103_0159-0-0.raw,file:/store/data/testbeam/run_20211103_0159-1-0.raw include20x10=<True or False>
```

# Unpacking 2022 TestBeam data
```bash
cmsrel CMSSW_12_2_0_pre1
cd CMSSW_12_2_0_pre1/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:testbeam_2022 # Will be updated
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/EventFilter/test
cmsRun gemTestBeam2022_step1.py inputFiles=file:<file1 path>,<file2 path>
```

# fireworks
```bash
cmsrel CMSSW_12_2_0_pre1
cd CMSSW_12_2_0_pre1/src
git cms-merge-topic jshlee:fireworks-GEMOnly-CMSSW_12_2_X
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/Analysis/test

cmsRun dumpSimGeometry_cfg.py tag=GEMTB
cmsRun dumpRecoGeometry_cfg.py tag=GEMTB calo=0 tracker=0 muon=0 gem=1 tgeo=0
cmsRun dumpRecoGeometry_cfg.py tag=GEMTB calo=0 tracker=0 muon=0 gem=1 tgeo=1
```

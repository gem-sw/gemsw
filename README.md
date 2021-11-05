# gemsw
QC8 and testbeam 

instructions for setting up
```bash
cmsrel CMSSW_12_1_0_pre3
cd CMSSW_12_1_0_pre3/src
git clone git@github.com:gem-sw/gemsw.git
scram b -j10
```

# testbeam unpacking
instructions for unpacking the data from testbeam set up
```bash
cmsrel CMSSW_12_1_0_pre4
cd CMSSW_12_1_0_pre4/src
cmsenv
git cms-init -q
git cms-merge-topic yeckang:mapping_update_v0.5 # Will be updated
git clone git@github.com:gem-sw/gemsw.git
scram b -j 4
cd gemsw/EventFilter/test
cmsRun testbeamReadout.py inputFiles=<file path> useB904Data=True skipBadDigi=False dqm=True reconstruct=True isME0data=<True or False>
python3 make_profile.py <DQM root file> <root file to save profile>
```

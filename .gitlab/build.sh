#!/bin/bash

# exit when any command fails; be verbose
set -ex

shopt -s expand_aliases
export MY_BUILD_DIR=${PWD}

sudo ln -s /usr/lib64/libssl.so.10 /usr/lib/libssl.so
sudo ln -s /usr/lib64/libcrypto.so.10 /usr/lib/libcrypto.so

CMS_PATH=/cvmfs/cms.cern.ch
CMSSW_RELEASE=CMSSW_13_0_9
SCRAM_ARCH=slc7_amd64_gcc10

cd /home/cmsusr
source ${CMS_PATH}/cmsset_default.sh
cmsrel ${CMSSW_RELEASE}
cd ${CMSSW_RELEASE}/src
cmsenv

export CMSSW_MIRROR=https://:@git.cern.ch/kerberos/CMSSW.git
export CMSSW_GIT_REFERENCE=/cvmfs/cms.cern.ch/cmssw.git.daily

git cms-init --upstream-only
git config --global user.name 'Raul Rabadan'
git config --global user.email 'raul.iraq.rabadan.trejo@cern.ch'
git config --global user.github rrabadan
git cms-merge-topic yeckang:QC8Unpacker_13_0_X

mkdir gemsw

cp -r ${MY_BUILD_DIR}/Alignment gemsw 
cp -r ${MY_BUILD_DIR}/Analysis gemsw 
cp -r ${MY_BUILD_DIR}/DQM gemsw 
cp -r ${MY_BUILD_DIR}/EventFilter gemsw
cp -r ${MY_BUILD_DIR}/Geometry gemsw 
cp -r ${MY_BUILD_DIR}/RecoMuon gemsw 
cp -r ${MY_BUILD_DIR}/Simulation gemsw
cp -r ${MY_BUILD_DIR}/Validation gemsw

scram build -j 1

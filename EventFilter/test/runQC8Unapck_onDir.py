import sys, os
from glob import glob
import subprocess

def runCommand(cmd) :
  subprocess.run(['echo'] + cmd)
  re = subprocess.run(cmd)
  if re.returncode != 0 : exit(re)

def runQC8Analysis(dataDir, outName) :
  flist = glob(f'{dataDir}/*')
  
  runData = {}
  for fullPath in sorted(flist) :
    fName = fullPath.split('/')[-1]
    run = fName.split('run_')[-1].split('-')[0]
    if run not in runData.keys() : runData[run] = {}
    index = fName.split('index')[-1].split('.')[0]
    if index not in runData[run].keys() : runData[run][index] = []
    runData[run][index].append(fullPath)
  
  for run in runData.keys() :
    tmpOutDir = f'run_{run}'
    runCommand(['mkdir', '-p', tmpOutDir])
    for index in runData[run].keys() :
      inputArg = 'inputFiles='
      for fullPath in runData[run][index] :
        inputArg += f'file:{fullPath},'
      inputArg = inputArg.rstrip(',')
      outArg = f'DQMoutName=file:{tmpOutDir}/{outName}_{index}.root'
      runCommand(['cmsRun', 'qc8Unpack.py', inputArg, 'maxEvents=-1', outArg])
    outList = glob(f'{tmpOutDir}/*')
    runCommand(['hadd', f'{outName}.root'] + outList)
    runCommand(['rm', '-rf', tmpOutDir])

if __name__ == '__main__' :
  import argparse
  parser = argparse.ArgumentParser(description='Run the qc8Unapck.py for the runs with multiple file')
  parser.add_argument('--output', '-o', default='track_inDQM.root', help='filename for output root file (at the moment it only supports DQM type output)')
  parser.add_argument('--dataDir', '-d', default=None, help='region to draw histogram')
  args = parser.parse_args()
  runQC8Analysis(args.dataDir, args.output.rstrip('.root'))

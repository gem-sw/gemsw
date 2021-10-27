import ROOT as r
import sys, os

f = r.TFile.Open(sys.argv[1], 'read')

dir_name = 'DQMData/Run 1/GEM/Run summary/Digis/'
track1_chambers = f.Get(dir_name + 'occ_GE11-M-01L1-S')
track2_chambers = f.Get(dir_name + 'occ_GE11-M-03L1-S')

GE21M1 = f.Get(dir_name + 'occ_GE21-M-01L1-S')

saveHist = []

saveHist.append(track1_chambers.ProjectionX('chamber1_X', 1, 1))
saveHist.append(track1_chambers.ProjectionX('chamber1_Y', 2, 2))
saveHist.append(track1_chambers.ProjectionX('chamber2_X', 3, 3))
saveHist.append(track1_chambers.ProjectionX('chamber2_Y', 4, 4))

saveHist.append(track2_chambers.ProjectionX('chamber3_X', 1, 1))
saveHist.append(track2_chambers.ProjectionX('chamber3_Y', 2, 2))
saveHist.append(track2_chambers.ProjectionX('chamber4_X', 3, 3))
saveHist.append(track2_chambers.ProjectionX('chamber4_Y', 4, 4))

saveHist.append(GE21M1.ProjectionX('ge21_ieta16', 16, 16))
saveHist.append(GE21M1.ProjectionX('ge21_ieta15', 15, 15))
saveHist.append(GE21M1.ProjectionX('ge21_ieta14', 14, 14))
saveHist.append(GE21M1.ProjectionX('ge21_ieta13', 13, 13))

fout = r.TFile.Open(sys.argv[2], 'recreate')
for hist in saveHist : 
    hist.SetTitle(hist.GetName())
    hist.Write()
fout.Close()

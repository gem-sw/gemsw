import ROOT
import numpy

"""
strip           = 0
ieta            = 0
chamber         = 0
"""

#Getting the ROOT files
fname = "/afs/cern.ch/user/k/kmartine/qc8/CMSSW_13_0_9/src/gemsw/Analysis/test/MyTest.root"
f = ROOT.TFile(fname)
events = f.Get("/TrackAnalyzer/MyTree")

#Defining the Canvas and Histograms
def define_canvas_hist(canvas, hist, name1, name2, xnbins, xbin_low, xbin_high, ynbins, ybin_low, ybin_high):
	canvas = ROOT.TCanvas(name1, name2, 100, 100, 600, 600)
	hist = ROOT.TH2D(name1, name2, xnbins, xbin_low, xbin_high, ynbins, ybin_low, ybin_high)
	return canvas, hist

StripEta_canvas = ROOT.TCanvas("Strip_vs_Eta", "Strip_vs_Eta", 100, 100, 600, 600)
StripEta_hist = ROOT.TH2D("StripEta", "StripEta", 64, 0, 384, 15, 0, 15)

StripEtaChamber1_canvas = ROOT.TCanvas("Strip_vs_EtaChamber1", "Strip_vs_EtaChamber1", 100, 100, 600, 600)
StripEtaChamber1_hist = ROOT.TH2D("StripEtaChamber1", "StripEtaChamber1", 64, 0, 384, 15, 0, 15)

StripEtaChamber2_canvas = ROOT.TCanvas("Strip_vs_EtaChamber2", "Strip_vs_EtaChamber2", 100, 100, 600, 600)
StripEtaChamber2_hist = ROOT.TH2D("StripEtaChamber2", "StripEtaChamber2", 64, 0, 384, 15, 0, 15)

StripEtaChamber3_canvas = ROOT.TCanvas("Strip_vs_EtaChamber3", "Strip_vs_EtaChamber3", 100, 100, 600, 600)
StripEtaChamber3_hist = ROOT.TH2D("StripEtaChamber3", "StripEtaChamber3", 64, 0, 384, 15, 0, 15)

StripEtaChamber4_canvas = ROOT.TCanvas("Strip_vs_EtaChamber4", "Strip_vs_EtaChamber4", 100, 100, 600, 600)
StripEtaChamber4_hist = ROOT.TH2D("StripEtaChamber4", "StripEtaChamber4", 64, 0, 384, 15, 0, 15)

StripEtaChamber5_canvas = ROOT.TCanvas("Strip_vs_EtaChamber5", "Strip_vs_EtaChamber5", 100, 100, 600, 600)
StripEtaChamber5_hist = ROOT.TH2D("StripEtaChamber5", "StripEtaChamber5", 64, 0, 384, 15, 0, 15)

StripEtaChamber6_canvas = ROOT.TCanvas("Strip_vs_EtaChamber6", "Strip_vs_EtaChamber6", 100, 100, 600, 600)
StripEtaChamber6_hist = ROOT.TH2D("StripEtaChamber6", "StripEtaChamber6", 64, 0, 384, 15, 0, 15)

StripEtaChamber7_canvas = ROOT.TCanvas("Strip_vs_EtaChamber7", "Strip_vs_EtaChamber7", 100, 100, 600, 600)
StripEtaChamber7_hist = ROOT.TH2D("StripEtaChamber7", "StripEtaChamber7", 64, 0, 384, 15, 0, 15)

Strip1D_canvas = ROOT.TCanvas("Strip_hist", "Strip_hist", 100, 100, 600, 600)
Strip1D_hist = ROOT.TH1D("Strips", "Strips", 64, 0, 384)

#Saving Figures
def saving_canvas(canvas, hist, eta):
	canvas.cd()
	x_axis = hist.GetXaxis()
	y_axis = hist.GetYaxis()
	x_axis.SetTitle("Strip")
	y_axis.SetTitle("Eta")
	hist.Draw("colz")
	canvas.SaveAs("/afs/cern.ch/user/k/kmartine/qc8/CMSSW_13_0_9/src/gemsw/Analysis/python/Strip_Eta_%sChamber.png"%(eta))

#Looping through Events
max_number = 0
for index, event in enumerate(events):
	StripEta_canvas.cd()
	StripEta_canvas.Clear()
	StripEtaChamber1_canvas.cd()
	StripEtaChamber1_canvas.Clear()
	StripEtaChamber2_canvas.cd()
	StripEtaChamber2_canvas.Clear()
	StripEtaChamber3_canvas.cd()
	StripEtaChamber3_canvas.Clear()
	StripEtaChamber4_canvas.cd()
	StripEtaChamber4_canvas.Clear()
	StripEtaChamber5_canvas.cd()
	StripEtaChamber5_canvas.Clear()
	StripEtaChamber6_canvas.cd()
	StripEtaChamber6_canvas.Clear()
	StripEtaChamber7_canvas.cd()
	StripEtaChamber7_canvas.Clear()
	Strip1D_canvas.cd()
	Strip1D_canvas.Clear()

	#print("\nEvent:", index)

	#print("chamber =", event.chamber)
	#print("strip = ", event.strip)
	#print("eta =", event.ieta)

	if event.strip == 0: 
		print("Strip 0 not plotted")
	else: StripEta_hist.Fill(event.strip, event.ieta)	
	Strip1D_hist.Fill(event.strip)
	if event.chamber == 1: 
		StripEtaChamber1_hist.Fill(event.strip, event.ieta)
	if event.chamber == 2: 
		StripEtaChamber2_hist.Fill(event.strip, event.ieta)
	if event.chamber == 3: 
		StripEtaChamber3_hist.Fill(event.strip, event.ieta)
	if event.chamber == 4: 
		StripEtaChamber4_hist.Fill(event.strip, event.ieta)
	if event.chamber == 5: 
		StripEtaChamber5_hist.Fill(event.strip, event.ieta)
	if event.chamber == 6: 
		StripEtaChamber6_hist.Fill(event.strip, event.ieta)
	if event.chamber == 7: 
		StripEtaChamber7_hist.Fill(event.strip, event.ieta)
	#if index > 5: break
	if event.strip > max_number: 
		max_number = event.strip

print("Max strip number: ", max_number)

saving_canvas(StripEta_canvas, StripEta_hist, "All") 
saving_canvas(StripEtaChamber1_canvas, StripEtaChamber1_hist, "1") 
saving_canvas(StripEtaChamber2_canvas, StripEtaChamber2_hist, "2") 
saving_canvas(StripEtaChamber3_canvas, StripEtaChamber3_hist, "3") 
saving_canvas(StripEtaChamber4_canvas, StripEtaChamber4_hist, "4") 
saving_canvas(StripEtaChamber5_canvas, StripEtaChamber5_hist, "5") 
saving_canvas(StripEtaChamber6_canvas, StripEtaChamber6_hist, "6") 
saving_canvas(StripEtaChamber7_canvas, StripEtaChamber7_hist, "7") 
Strip1D_canvas.cd()
x_axis = Strip1D_hist.GetXaxis()
y_axis = Strip1D_hist.GetYaxis()
x_axis.SetTitle("Strip")
y_axis.SetTitle("Events")
Strip1D_hist.Draw()
Strip1D_canvas.SaveAs("/afs/cern.ch/user/k/kmartine/qc8/CMSSW_13_0_9/src/gemsw/Analysis/python/Strip1D_hist.png")


//root macro to take ntuple generated by plot_triggers and generate AN-ready plots of correlation between efficiency and analysis variables
//macro usage: root -l scripts/trig_postprocess_plots.cxx

#include <iostream>
#include <string>
#include <vector>

#include "TAxis.h"
#include "TCanvas.h"
#include "TFile.h"
#include "TGraphAsymmErrors.h"
#include "TH1.h"
#include "TLatex.h"

//helper functions
void make_efficiency_plot(TH1D* hist_num, TH1D* hist_den, TGraphAsymmErrors* hist_ratio, std::string lumi_string, std::string title, std::string xaxis, std::string yaxis, std::string units, std::string out_filename, bool var_width_bins=false, bool is_simulation=false);

int trig_postprocess_plots() {
	//settings
	TFile* f = TFile::Open("ntuples/trig_eff_old_variables_2018_met120.root");
	int year = 2018;
	bool do_controlregions = false;

	//other stuff
	std::string string_lumi = "35.9 fb^{-1} (13 TeV)";
	std::string string_mc = "Summer16 t#bar{t} 1l";
	std::string det_trigger = "#it{Ele27}";
	if (year == 2017) {
		string_lumi = "41.5 fb^{-1} (13 TeV)";
		string_mc = "Fall17 t#bar{t} 1l";
	}
	else if (year == 2018) {
		string_lumi = "60 fb^{-1} (13 TeV)";
		string_mc = "Autumn18 t#bar{t} 1l";
		det_trigger = "#it{Ele35}";
	}
	//real met
	TH1D* realmet_num = static_cast<TH1D*>((f->Get("hist_realmet_numerator"))->Clone());
	TH1D* realmet_den = static_cast<TH1D*>((f->Get("hist_realmet_denominator"))->Clone());
	TGraphAsymmErrors* realmet_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_realmet_ratio"))->Clone());
	make_efficiency_plot(realmet_num,realmet_den,realmet_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1","Offline E_{T}^{miss}","Efficiency MET120","GeV","realmet");
	//fake met
	TH1D* fakemet_num = static_cast<TH1D*>((f->Get("hist_fakemet_numerator"))->Clone());
	TH1D* fakemet_den = static_cast<TH1D*>((f->Get("hist_fakemet_denominator"))->Clone());
	TGraphAsymmErrors* fakemet_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_fakemet_ratio"))->Clone());
	make_efficiency_plot(fakemet_num,fakemet_den,fakemet_ratio,string_lumi,"Denom: #it{Jet500}, low #Delta #phi, N_{l veto}=0","Offline E_{T}^{miss}","Efficiency MET120","GeV","fakemet");
	//HT low met
	TH1D* htlowmet_num = static_cast<TH1D*>((f->Get("hist_htlowmet_numerator"))->Clone());
	TH1D* htlowmet_den = static_cast<TH1D*>((f->Get("hist_htlowmet_denominator"))->Clone());
	TGraphAsymmErrors* htlowmet_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_htlowmet_ratio"))->Clone());
	make_efficiency_plot(htlowmet_num,htlowmet_den,htlowmet_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, 150 GeV<MET#leq 200 GeV","Offline H_{T}","Efficiency MET120","GeV","htlowmet");
	//HT high met
	TH1D* hthighmet_num = static_cast<TH1D*>((f->Get("hist_hthighmet_numerator"))->Clone());
	TH1D* hthighmet_den = static_cast<TH1D*>((f->Get("hist_hthighmet_denominator"))->Clone());
	TGraphAsymmErrors* hthighmet_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_hthighmet_ratio"))->Clone());
	make_efficiency_plot(hthighmet_num,hthighmet_den,hthighmet_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, 200 GeV<MET#leq 300 GeV","Offline H_{T}","Efficiency MET120","GeV","hthighmet");
	//nb
	TH1D* nb_num = static_cast<TH1D*>((f->Get("hist_nb_numerator"))->Clone());
	TH1D* nb_den = static_cast<TH1D*>((f->Get("hist_nb_denominator"))->Clone());
	TGraphAsymmErrors* nb_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_nb_ratio"))->Clone());
	make_efficiency_plot(nb_num,nb_den,nb_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 200 GeV","Offline N_{b}","Efficiency MET120","","nb");
	//nj
	TH1D* nj_num = static_cast<TH1D*>((f->Get("hist_nj_numerator"))->Clone());
	TH1D* nj_den = static_cast<TH1D*>((f->Get("hist_nj_denominator"))->Clone());
	TGraphAsymmErrors* nj_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_nj_ratio"))->Clone());
	make_efficiency_plot(nj_num,nj_den,nj_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 200 GeV","Offline N_{j}","Efficiency MET120","","nj");
	//<m> for Higgs candidates
	TH1D* higcandam_num = static_cast<TH1D*>((f->Get("hist_higcandam_numerator"))->Clone());
	TH1D* higcandam_den = static_cast<TH1D*>((f->Get("hist_higcandam_denominator"))->Clone());
	TGraphAsymmErrors* higcandam_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_higcandam_ratio"))->Clone());
	make_efficiency_plot(higcandam_num,higcandam_den,higcandam_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 200 GeV","Offline #LT m#GT","Efficiency MET120","GeV","higcandam");
	//<m> for Higgs candidates - 200<HT<300 (temp)
	TH1D* higcandamht200300_num = static_cast<TH1D*>((f->Get("hist_higcandamht200300_numerator"))->Clone());
	TH1D* higcandamht200300_den = static_cast<TH1D*>((f->Get("hist_higcandamht200300_denominator"))->Clone());
	TGraphAsymmErrors* higcandamht200300_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_higcandamht200300_ratio"))->Clone());
	make_efficiency_plot(higcandamht200300_num,higcandamht200300_den,higcandamht200300_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 150 GeV, 200<HT<300 GeV","Offline #LT m#GT","Efficiency MET120","GeV","higcandamht200300");
	//<m> for Higgs candidates - 300<HT<400 (temp)
	TH1D* higcandamht300400_num = static_cast<TH1D*>((f->Get("hist_higcandamht300400_numerator"))->Clone());
	TH1D* higcandamht300400_den = static_cast<TH1D*>((f->Get("hist_higcandamht300400_denominator"))->Clone());
	TGraphAsymmErrors* higcandamht300400_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_higcandamht300400_ratio"))->Clone());
	make_efficiency_plot(higcandamht300400_num,higcandamht300400_den,higcandamht300400_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 150 GeV, 300<HT<400 GeV","Offline #LT m#GT","Efficiency MET120","GeV","higcandamht300400");
	//<m> for Higgs candidates - 400<HT<500 (temp)
	TH1D* higcandamht400500_num = static_cast<TH1D*>((f->Get("hist_higcandamht400500_numerator"))->Clone());
	TH1D* higcandamht400500_den = static_cast<TH1D*>((f->Get("hist_higcandamht400500_denominator"))->Clone());
	TGraphAsymmErrors* higcandamht400500_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_higcandamht400500_ratio"))->Clone());
	make_efficiency_plot(higcandamht400500_num,higcandamht400500_den,higcandamht400500_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 150 GeV, 400<HT<500 GeV","Offline #LT m#GT","Efficiency MET120","GeV","higcandamht400500");
	//<m> for Higgs candidates - 500<HT<600 (temp)
	TH1D* higcandamht500600_num = static_cast<TH1D*>((f->Get("hist_higcandamht500600_numerator"))->Clone());
	TH1D* higcandamht500600_den = static_cast<TH1D*>((f->Get("hist_higcandamht500600_denominator"))->Clone());
	TGraphAsymmErrors* higcandamht500600_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_higcandamht500600_ratio"))->Clone());
	make_efficiency_plot(higcandamht500600_num,higcandamht500600_den,higcandamht500600_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 150 GeV, 500<HT<600 GeV","Offline #LT m#GT","Efficiency MET120","GeV","higcandamht500600");
	//Delta R max
	TH1D* higcanddrmax_num = static_cast<TH1D*>((f->Get("hist_higcanddrmax_numerator"))->Clone());
	TH1D* higcanddrmax_den = static_cast<TH1D*>((f->Get("hist_higcanddrmax_denominator"))->Clone());
	TGraphAsymmErrors* higcanddrmax_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_higcanddrmax_ratio"))->Clone());
	make_efficiency_plot(higcanddrmax_num,higcanddrmax_den,higcanddrmax_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET > 200 GeV","Offline #Delta R_{max}","Efficiency MET120","","higcanddrmax");
	if (do_controlregions) {
		//1e MET
		TH1D* elmet_num = static_cast<TH1D*>((f->Get("hist_elmet_numerator"))->Clone());
		TH1D* elmet_den = static_cast<TH1D*>((f->Get("hist_elmet_denominator"))->Clone());
		TGraphAsymmErrors* elmet_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_elmet_ratio"))->Clone());
		make_efficiency_plot(elmet_num,elmet_den,elmet_ratio,string_lumi,"Denom: #it{Jet500}, N_{j}#geq 2, N_{e}=1","Offline E_{T}^{miss}","Efficiency [MET[NoMu][110|120|120_ht60]|Ele[Iso27|Iso35|115]]","GeV","elmet");
		//1e pt
		TH1D* elpt_num = static_cast<TH1D*>((f->Get("hist_elpt_numerator"))->Clone());
		TH1D* elpt_den = static_cast<TH1D*>((f->Get("hist_elpt_denominator"))->Clone());
		TGraphAsymmErrors* elpt_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_elpt_ratio"))->Clone());
		make_efficiency_plot(elpt_num,elpt_den,elpt_ratio,string_lumi,"Denom: #it{Jet500}, N_{j}#geq 2, N_{e}=1, 150 GeV<MET #leq 200 GeV","Offline Electron p_{T}","Efficiency [MET[NoMu][110|120|120_ht60]|Ele[Iso27|Iso35|115]]","GeV","elpt");
		//1mu MET
		TH1D* mumet_num = static_cast<TH1D*>((f->Get("hist_mumet_numerator"))->Clone());
		TH1D* mumet_den = static_cast<TH1D*>((f->Get("hist_mumet_denominator"))->Clone());
		TGraphAsymmErrors* mumet_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_mumet_ratio"))->Clone());
		make_efficiency_plot(mumet_num,mumet_den,mumet_ratio,string_lumi,"Denom: #it{Jet500}, N_{j}#geq 2, N_{#mu}=1","Offline E_{T}^{miss}","Efficiency [MET[NoMu][110|120|120_ht60]|Mu[Iso24|Iso27|50]]","GeV","mumet");
		//1mu pt
		TH1D* mupt_num = static_cast<TH1D*>((f->Get("hist_mupt_numerator"))->Clone());
		TH1D* mupt_den = static_cast<TH1D*>((f->Get("hist_mupt_denominator"))->Clone());
		TGraphAsymmErrors* mupt_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_mupt_ratio"))->Clone());
		make_efficiency_plot(mupt_num,mupt_den,mupt_ratio,string_lumi,"Denom: #it{Jet500}, N_{j}#geq 2, N_{#mu}=1, 150 GeV<MET #leq 200 GeV","Offline Muon p_{T}","Efficiency [MET[NoMu][110|120|120_ht60]|Mu[Iso24|Iso27|50]]","GeV","mupt");
		//2e
		TH1D* elel_show_num = static_cast<TH1D*>((f->Get("hist_elel_show_numerator"))->Clone());
		TH1D* elel_show_den = static_cast<TH1D*>((f->Get("hist_elel_show_denominator"))->Clone());
		TGraphAsymmErrors* elel_show_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_elel_show_ratio"))->Clone());
		make_efficiency_plot(elel_show_num,elel_show_den,elel_show_ratio,string_lumi,"Denom: #it{MET[NoMu][110|120|120_ht60]|Jet500}, N_{j}#geq 2, N_{e}=2, 80 GeV<m_{ee}<100 GeV","Max Offline Electron p_{T}","Efficiency Ele[Iso27|Iso35|115]","GeV","elel_show");
		//2mu
		TH1D* mumu_show_num = static_cast<TH1D*>((f->Get("hist_mumu_show_numerator"))->Clone());
		TH1D* mumu_show_den = static_cast<TH1D*>((f->Get("hist_mumu_show_denominator"))->Clone());
		TGraphAsymmErrors* mumu_show_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_mumu_show_ratio"))->Clone());
		make_efficiency_plot(mumu_show_num,mumu_show_den,mumu_show_ratio,string_lumi,"Denom: #it{MET[NoMu][110|120|120_ht60]|Jet500}, N_{j}#geq 2, N_{#mu}=2, 80 GeV<m_{#mu#mu}< 100 GeV","Max Offline Muon p_{T}","Efficiency Mu[Iso24|Iso27|50]","GeV","mumu_show");
	}
    	//data (nominal)
    	TH1D* datamet120_num = static_cast<TH1D*>((f->Get("hist_datamet120_numerator"))->Clone());
    	TH1D* datamet120_den = static_cast<TH1D*>((f->Get("hist_datamet120_denominator"))->Clone());
    	TGraphAsymmErrors* datamet120_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_datamet120_ratio"))->Clone());
    	make_efficiency_plot(datamet120_num,datamet120_den,datamet120_ratio,string_lumi,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1","Offline E_{T}^{miss}","Efficiency MET[NoMu]120","GeV","datamet120");
    	//mc
    	TH1D* mcmet120_num = static_cast<TH1D*>((f->Get("hist_mcmet120_numerator"))->Clone());
    	TH1D* mcmet120_den = static_cast<TH1D*>((f->Get("hist_mcmet120_denominator"))->Clone());
    	TGraphAsymmErrors* mcmet120_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_mcmet120_ratio"))->Clone());
    	make_efficiency_plot(mcmet120_num,mcmet120_den,mcmet120_ratio,string_mc,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1","Offline E_{T}^{miss}","Efficiency MET[NoMu]120","GeV","mcmet120",false,true);
    	//mc - <m> temporary
    	TH1D* mcmet120higcandam_num = static_cast<TH1D*>((f->Get("hist_mcmet120higcandam_numerator"))->Clone());
    	TH1D* mcmet120higcandam_den = static_cast<TH1D*>((f->Get("hist_mcmet120higcandam_denominator"))->Clone());
    	TGraphAsymmErrors* mcmet120higcandam_ratio = static_cast<TGraphAsymmErrors*>((f->Get("hist_mcmet120higcandam_ratio"))->Clone());
    	make_efficiency_plot(mcmet120higcandam_num,mcmet120higcandam_den,mcmet120higcandam_ratio,string_mc,"Denom: "+det_trigger+", N_{j}#geq 3, high #Delta #phi, N_{e}=1, MET>200 GeV","Offline #LT m#GT","Efficiency MET[NoMu]120","GeV","mcmet120higcandam",false,true);
    	return 0;
}

void make_efficiency_plot(TH1D* hist_num, TH1D* hist_den, TGraphAsymmErrors* hist_ratio, std::string lumi_string, std::string title, std::string xaxis, std::string yaxis, std::string units, std::string out_filename, bool var_width_bins, bool is_simulation) {
	//draw ratio and axis labels
	gStyle->SetOptStat(0);
	TCanvas* can = new TCanvas(("can_"+out_filename).c_str(),"can",1024,1024);
	can->cd();
	TPad* pad = new TPad(("pad_"+out_filename).c_str(),"pad",0.,0.,1.0,1.0);
	pad->Draw();
	pad->cd();
	pad->SetGrid();
	gPad->SetMargin(0.15,0.15,0.15,0.15);

	double xlow = hist_den->GetBinLowEdge(1);
	double xhigh = hist_den->GetBinLowEdge(hist_den->GetNbinsX())+hist_den->GetBinWidth(hist_den->GetNbinsX());
	double bin_size = (xhigh-xlow)/hist_den->GetNbinsX();
	hist_ratio->GetXaxis()->SetRangeUser(xlow, xhigh);
	if (yaxis.size() > 50) {
		hist_ratio->GetYaxis()->SetTitleSize(0.025);
		hist_ratio->GetYaxis()->SetTitleOffset(1.6);
	}
	else {
		hist_ratio->GetYaxis()->SetTitleOffset(1.4);
	}
	hist_ratio->GetYaxis()->SetRangeUser(0, 1.4);
	if (units == "") {
		hist_ratio->SetTitle((";"+xaxis+";"+yaxis).c_str());
	}
	else {
		hist_ratio->SetTitle((";"+xaxis+"("+units+");"+yaxis).c_str());
	}
	hist_ratio->SetLineWidth(2);
	hist_ratio->Draw("AP");

	//draw overlay text
	TLatex t;
	t.SetTextColor(kBlack);
	t.SetTextSize(0.04);
	if (!is_simulation) {
		t.DrawLatexNDC(0.155,0.87,"#font[62]{CMS} #scale[0.8]{#font[52]{Preliminary}}");
	}
	else {
		t.DrawLatexNDC(0.155,0.87,"#font[62]{CMS} #scale[0.8]{#font[52]{Simulation}}");
	}
	t.SetTextAlign(31);
	t.DrawLatexNDC(0.845,0.87,("#font[42]{"+lumi_string+"}").c_str());
	t.SetTextAlign(33);
	t.SetTextSize(0.03);
	if (title.size() < 76) {
		t.DrawLatexNDC(0.825,0.83,("#font[42]{"+title+"}").c_str());
	}
	else {
		//title too long, draw on separate lines
		unsigned int right_split_comma_pos = 999;
		for (unsigned int string_pos = 75; string_pos > 0; string_pos --) {
			if (title[string_pos]==',') {
				//split title at this comma
				right_split_comma_pos = string_pos;
				break;
			}
		}
		if (right_split_comma_pos == 999) {
			//unable to find splitting comma, just let it draw off histogram
			t.DrawLatexNDC(0.825,0.83,("#font[42]{"+title+"}").c_str());
		}
		else {
			t.DrawLatexNDC(0.825,0.83,("#font[42]{"+title.substr(0,right_split_comma_pos+1)+"}").c_str());
			t.DrawLatexNDC(0.825,0.78,("#font[42]{"+title.substr(right_split_comma_pos+1,title.size()-right_split_comma_pos-1)+"}").c_str());
		}
	}
	
	//draw numerator and denominator histograms
	double hist_den_max = hist_den->GetBinContent(hist_den->GetMaximumBin());
	hist_den->Scale(0.5/hist_den_max);
	hist_den->SetLineStyle(2);
	hist_den->SetLineColor(kBlue);
	hist_den->SetLineWidth(2);
	hist_den->SetFillStyle(0);
	hist_den->Draw("same hist");

	hist_num->Scale(0.5/hist_den_max);
	hist_num->SetLineColor(kBlue);
	hist_num->SetLineWidth(2);
	hist_num->SetFillStyle(0);
	//hist_num->SetFillColor(kBlue);
	hist_num->Draw("same hist");

	hist_ratio->Draw("P");

	//draw right axis
	pad->Update();
	TGaxis *norm_axis = new TGaxis(gPad->GetUxmax(), gPad->GetUymin(), gPad->GetUxmax(), gPad->GetUymax(), 0, 2.8*hist_den_max, 505, "+L");
	norm_axis->SetTickLength(0.3);
	norm_axis->SetLabelSize(0.03);
	if (units != "" && !var_width_bins) {
		norm_axis->SetTitle(("Events/("+std::to_string(static_cast<int>(bin_size))+" "+units+")").c_str());
	}
	else {
		norm_axis->SetTitle("Events/bin");
	}
	norm_axis->SetTitleColor(kBlue);
	norm_axis->SetTitleFont(42);
	norm_axis->SetTitleOffset(1.6);
	norm_axis->SetLineColor(kBlue);
	norm_axis->SetLabelColor(kBlue);
	norm_axis->Draw();

	//save
	can->Update();
	can->SaveAs(("plots/trig_eff_resolved_"+out_filename+".png").c_str());
}


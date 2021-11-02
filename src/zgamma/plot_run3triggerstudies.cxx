//This script generates some tables for run3 H->ZGamma trigger studies
//
//Arguments
// --single_thread (-s) to run single thread for debugging
// --unblind (-u) to unblind (not done for AN plots)
// --year (-y) yearname to select data year (2016, 2017, 2018, run2)
// --tag (-t) to add a tag to produced plots
// --string_options (-o) to specify what to plot
//
//possible string options: 
//

#include "core/test.hpp"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include <unistd.h>
#include <getopt.h>

#include "Math/Vector4D.h"
#include "TCanvas.h"
#include "TColor.h"
#include "TError.h"
#include "TFile.h"
#include "TLatex.h"
#include "TMath.h"
#include "TPad.h"
#include "TPie.h"
#include "TVector2.h"

#include "core/baby.hpp"
#include "core/cross_sections.hpp"
#include "core/efficiency_plot.hpp"
#include "core/event_scan.hpp"
#include "core/functions.hpp"
#include "core/gamma_params.hpp"
#include "core/hist1d.hpp"
#include "core/hist2d.hpp"
#include "core/named_func.hpp"
#include "core/palette.hpp"
#include "core/plot_maker.hpp"
#include "core/plot_opt.hpp"
#include "core/process.hpp"
#include "core/table.hpp"
#include "core/utilities.hpp"
#include "zgamma/script_utilities.hpp"

using namespace std;
using namespace PlotOptTypes;

int main(int argc, char *argv[]){
  //------------------------------------------------------------------------------------
  //                                    initialization
  //------------------------------------------------------------------------------------
  gErrorIgnoreLevel = 6000;
  time_t begtime, endtime;
  time(&begtime);
  Palette colors("txt/colors.txt","default");
  script_utilities::ArgStruct options = script_utilities::get_options(
      argc, argv, "");
  std::vector<PlotOpt> plt_lin = script_utilities::plot_lin(options.unblind);
  std::vector<PlotOpt> plt_log = script_utilities::plot_log(options.unblind);
  std::vector<PlotOpt> plt_shapes = script_utilities::plot_shapes();
  std::vector<PlotOpt> plt_log_shapes = script_utilities::plot_log_shapes();

  //set<int> years;
  //HigUtilities::parseYears(options.year_string, years);
  //int lumi_precision = 0;
  //string total_luminosity_string = HigUtilities::getLuminosityString(options.year_string, lumi_precision);
  std::string total_luminosity_string = "41.5";

  std::string base_folder = "/net/cms17/cms17r0/pico/NanoAODv7/nano/2017/signal/";

  std::vector<std::shared_ptr<Process>> procs;
  procs.push_back(Process::MakeShared<Baby_nano>("HToZG", 
      Process::Type::signal, kBlack,
      {base_folder+"*GluGluHToZG_ZToLL*"},"1"));

  //------------------------------------------------------------------------------------
  //                                     named funcs
  //------------------------------------------------------------------------------------
  
  //2017 triggers
  NamedFunc hlt_el_trigger = "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL||HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ";
  NamedFunc hlt_mu_trigger = "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8||HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8";
  NamedFunc hlt_el_trigger_plus = "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL||HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ||HLT_Ele35_WPTight_Gsf||HLT_Ele32_WPTight_Gsf_L1DoubleEG";
  NamedFunc hlt_mu_trigger_plus = "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8||HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8||HLT_IsoMu27||HLT_IsoMu24";
  NamedFunc hlt_mu_trigger_plusplus = "HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass3p8||HLT_Mu17_TrkIsoVVL_Mu8_TrkIsoVVL_DZ_Mass8||HLT_IsoMu27||HLT_IsoMu24||HLT_Mu17_Photon30_IsoCaloId";
  NamedFunc l1_el_trigger = "L1_SingleEG24||L1_SingleEG34er2p1||L1_SingleIsoEG24er2p1||L1_SingleIsoEG24||L1_DoubleEG_18_17||L1_DoubleEG_22_10||L1_DoubleEG_LooseIso23_10||L1_DoubleEG_LooseIso24_10";
  NamedFunc l1_mu_trigger = "L1_DoubleMu_12_5||L1_DoubleMu_15_7_SQ||L1_DoubleMu_15_7_SQ_Mass_Min4";

  const NamedFunc weight("weight",[](const Baby &b) -> NamedFunc::ScalarType{
    //extra factor for weighting is xs/nevts_effective where effective events are calculated including negative weights
    //0.007519850838359999 GluGluH->ZG->llG xs (pb)
    //967054
    float w_lumi = 1.0;
    float w_year = 1.0;
    if (b.Generator_weight()<0) w_lumi = -1.0;
    if (b.SampleType()==2016) w_year = 36.32264; 
    else if (b.SampleType()==2017) w_year = 41.52756;
    else if (b.SampleType()==2018) w_year = 59.67377;
    float corr_factor = 112.0/104.0; //temp, just to match scale with AN
    return w_lumi*w_year*7.7760403e-09*1000.0*corr_factor;
  });

  const NamedFunc z_decay_pdgid("z_decay_pdgid",[](const Baby &b) -> NamedFunc::ScalarType{
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==11 || abs_pdgid == 13 || abs_pdgid == 15) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            return abs_pdgid;
          }
        }
      }
    }
    return 0;
  });

  //number of electrons out of eta acceptance
  const NamedFunc nel_ooeta("nel_ooeta",[](const Baby &b) -> NamedFunc::ScalarType{
    int nel_ooa_ = 0;
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==11) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            if (abs(b.GenPart_eta()->at(imc))>2.5)
              nel_ooa_ += 1;
          }
        }
      }
    }
    return nel_ooa_;
  });

  //true if electrons out of level 1 pt acceptance
  const NamedFunc el_ool1pt("el_ool1pt",[](const Baby &b) -> NamedFunc::ScalarType{
    float el1_pt = 0;
    float el2_pt = 0;
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==11) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            if (b.GenPart_pt()->at(imc) > 24.0) return false;
            if (el1_pt <= 0)
              el1_pt = b.GenPart_pt()->at(imc);
            else
              el2_pt = b.GenPart_pt()->at(imc);
          }
        }
      }
    }
    if ((el1_pt > 18 && el2_pt > 17) || (el1_pt > 17 && el2_pt > 18)) return false;
    if ((el1_pt > 22 && el2_pt > 10) || (el1_pt > 10 && el2_pt > 22)) return false;
    return true;
  });

  //true if muons out of level 1 pt acceptance
  const NamedFunc mu_ool1pt("mu_ool1pt",[](const Baby &b) -> NamedFunc::ScalarType{
    float mu1_pt = 0;
    float mu2_pt = 0;
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==13) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            if (mu1_pt <= 0)
              mu1_pt = b.GenPart_pt()->at(imc);
            else
              mu2_pt = b.GenPart_pt()->at(imc);
          }
        }
      }
    }
    if ((mu1_pt > 12 && mu2_pt > 5) || (mu1_pt > 5 && mu2_pt > 12)) return false;
    return true;
  });

  //true if electrons out of HLT pt acceptance
  const NamedFunc el_oohltpt("el_oohltpt",[](const Baby &b) -> NamedFunc::ScalarType{
    float el1_pt = 0;
    float el2_pt = 0;
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==11) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            if (el1_pt <= 0)
              el1_pt = b.GenPart_pt()->at(imc);
            else
              el2_pt = b.GenPart_pt()->at(imc);
          }
        }
      }
    }
    if ((el1_pt > 23 && el2_pt > 12) || (el1_pt > 12 && el2_pt > 23)) return false;
    return true;
  });

  //true if electrons nearly out of HLT pt acceptance (2 GeV)
  const NamedFunc el_noohltpt("el_noohltpt",[](const Baby &b) -> NamedFunc::ScalarType{
    float el1_pt = 0;
    float el2_pt = 0;
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==11) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            if (el1_pt <= 0)
              el1_pt = b.GenPart_pt()->at(imc);
            else
              el2_pt = b.GenPart_pt()->at(imc);
          }
        }
      }
    }
    if ((el1_pt > 25 && el2_pt > 14) || (el1_pt > 14 && el2_pt > 25)) return false;
    return true;
  });

  const NamedFunc el_hlt_fail_reason("el_hlt_fail_reason",[](const Baby &b) -> NamedFunc::ScalarType{
    int n_pf_el = 0;
    std::vector<float> hlt_el_pt;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        n_pf_el++;
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          hlt_el_pt.push_back(b.TrigObj_pt()->at(itrig));
        }
      }
    }
    if (n_pf_el < 2) return 1; //electron not reconstructed at HLT
    if (hlt_el_pt.size() < 2) return 2; //electron failed CaloIdL_TrackIdL_IsoVL
    std::sort(hlt_el_pt.begin(), hlt_el_pt.end());
    if (hlt_el_pt[hlt_el_pt.size()-1] < 23.0) return 3; //top leg failed pt cut
    if (hlt_el_pt[hlt_el_pt.size()-2] < 12.0) return 4; //bottom leg failed pt cut
    return 0; //unknown
  });

  const NamedFunc nel_id_hlt("nel_id_hlt",[](const Baby &b) -> NamedFunc::ScalarType{
    int nel_hlt = 0;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          nel_hlt++;
        }
      }
    }
    return nel_hlt;
  });

  const NamedFunc nel_noid_hlt("nel_noid_hlt",[](const Baby &b) -> NamedFunc::ScalarType{
    int nel_hlt = 0;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        nel_hlt++;
      }
    }
    return nel_hlt;
  });

  const NamedFunc hlt_max_el_pt("hlt_max_el_pt",[](const Baby &b) -> NamedFunc::ScalarType{
    float hlt_max_el_pt_ = 0;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > hlt_max_el_pt_)
            hlt_max_el_pt_ = b.TrigObj_pt()->at(itrig);
        }
      }
    }
    return hlt_max_el_pt_;
  });

  const NamedFunc HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon30("HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon30",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    bool found_good_ph = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 27) {
            found_good_el = true;
          }
        }
      }
      if (b.TrigObj_id()->at(itrig) == 22) {
        if (b.TrigObj_pt()->at(itrig) > 30) {
          found_good_ph = true;
        }
      }
    }
    return (found_good_el && found_good_ph);
  });

  const NamedFunc HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon50("HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon50",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 27) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon50();
  });

  const NamedFunc HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon33("HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon33",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 27) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon33();
  });

  const NamedFunc HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon25("HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon25",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 27) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon25();
  });
  
  const NamedFunc HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose("HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 27) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon20_HoverELoose();
  });

  const NamedFunc HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon50("HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon50",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 22) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon50();
  });

  const NamedFunc HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon33("HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon33",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 22) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon33();
  });

  const NamedFunc HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon25("HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon25",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 22) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon25();
  });

  const NamedFunc HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose("HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose",[](const Baby &b) -> NamedFunc::ScalarType{
    bool found_good_el = false;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > 22) {
            found_good_el = true;
          }
        }
      }
    }
    return found_good_el && b.HLT_Photon20_HoverELoose();
  });

  const NamedFunc HLT_Ele17_Ele8_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose("HLT_Ele17_Ele8_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose",[](const Baby &b) -> NamedFunc::ScalarType{
    float max_el_pt = 0;
    float subl_el_pt = 0;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > max_el_pt) {
            subl_el_pt = max_el_pt;
            max_el_pt = b.TrigObj_pt()->at(itrig);
          }
          else if (b.TrigObj_pt()->at(itrig) > subl_el_pt) {
            subl_el_pt = b.TrigObj_pt()->at(itrig);
          }
        }
      }
    }
    return (max_el_pt > 17) && (subl_el_pt > 8) && b.HLT_Photon20_HoverELoose();
  });

  const NamedFunc HLT_Ele17_Ele8_CaloIdL_TrackIdL_IsoVL_Photon25("HLT_Ele17_Ele8_CaloIdL_TrackIdL_IsoVL_Photon25",[](const Baby &b) -> NamedFunc::ScalarType{
    float max_el_pt = 0;
    float subl_el_pt = 0;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 11) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //CaloIdL_TrackIdL_IsoVL
          if (b.TrigObj_pt()->at(itrig) > max_el_pt) {
            subl_el_pt = max_el_pt;
            max_el_pt = b.TrigObj_pt()->at(itrig);
          }
          else if (b.TrigObj_pt()->at(itrig) > subl_el_pt) {
            subl_el_pt = b.TrigObj_pt()->at(itrig);
          }
        }
      }
    }
    return (max_el_pt > 17) && (subl_el_pt > 8) && b.HLT_Photon25();
  });

  NamedFunc hlt_el_trigger_plusplus = "HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL||HLT_Ele23_Ele12_CaloIdL_TrackIdL_IsoVL_DZ||HLT_Ele35_WPTight_Gsf||HLT_Ele32_WPTight_Gsf_L1DoubleEG" || HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon33;

  const NamedFunc mu_hlt_fail_reason("mu_hlt_fail_reason",[](const Baby &b) -> NamedFunc::ScalarType{
    int n_pf_mu = 0;
    std::vector<float> hlt_mu_pt;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 13) {
        n_pf_mu++;
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //TkIsoVVL
          hlt_mu_pt.push_back(b.TrigObj_pt()->at(itrig));
        }
      }
    }
    if (n_pf_mu < 2) return 1; //muon not reconstructed at HLT
    if (hlt_mu_pt.size() < 2) return 2; //muon fails Iso
    std::sort(hlt_mu_pt.begin(), hlt_mu_pt.end());
    if (hlt_mu_pt[hlt_mu_pt.size()-1] < 23.0) return 3; //top leg failed pt cut
    if (hlt_mu_pt[hlt_mu_pt.size()-2] < 12.0) return 4; //bottom leg failed pt cut
    return 0; //unknown
  });

  const NamedFunc nmu_id_hlt("nmu_id_hlt",[](const Baby &b) -> NamedFunc::ScalarType{
    int n_pf_mu = 0;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 13) {
        if ((b.TrigObj_filterBits()->at(itrig) & 0x1)==1) { //TkIsoVVL
          n_pf_mu++;
        }
      }
    }
    return n_pf_mu;
  });

  const NamedFunc nmu_noid_hlt("nmu_noid_hlt",[](const Baby &b) -> NamedFunc::ScalarType{
    int n_pf_mu = 0;
    for (unsigned itrig = 0; itrig < b.TrigObj_pt()->size(); itrig++) {
      if (b.TrigObj_id()->at(itrig) == 13) {
        n_pf_mu++;
      }
    }
    return n_pf_mu;
  });

  //weights from data that should account for why things fail HLT
  const NamedFunc hlt_fail_weights("hlt_fail_weights",[](const Baby &b) -> NamedFunc::ScalarType{
    float el1_pt = 0;
    float el1_eta = 0;
    float el2_pt = 0;
    float el2_eta = 0;
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==11) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            if (el1_pt <= 0) {
              el1_pt = b.GenPart_pt()->at(imc);
              el1_eta = abs(b.GenPart_eta()->at(imc));
            }
            else {
              el2_pt = b.GenPart_pt()->at(imc);
              el2_eta = abs(b.GenPart_eta()->at(imc));
            }
          }
        }
      }
    }
    float leg1_w = 0.0, leg2_w = 0.0;
    if (el1_pt > 20 && el1_pt < 23 && el1_eta > 0.00 && el1_eta < 0.80) leg1_w = 0.25;
    else if (el1_pt > 20 && el1_pt < 23 && el1_eta > 0.80 && el1_eta < 1.44) leg1_w = 0.18;
    else if (el1_pt > 20 && el1_pt < 23 && el1_eta > 1.44 && el1_eta < 1.56) leg1_w = 0.27;
    else if (el1_pt > 20 && el1_pt < 23 && el1_eta > 1.56 && el1_eta < 2.50) leg1_w = 0.15;
    else if (el1_pt > 23 && el1_pt < 25 && el1_eta > 0.00 && el1_eta < 0.80) leg1_w = 0.80;
    else if (el1_pt > 23 && el1_pt < 25 && el1_eta > 0.80 && el1_eta < 1.44) leg1_w = 0.75;
    else if (el1_pt > 23 && el1_pt < 25 && el1_eta > 1.44 && el1_eta < 1.56) leg1_w = 0.65;
    else if (el1_pt > 23 && el1_pt < 25 && el1_eta > 1.56 && el1_eta < 2.50) leg1_w = 0.68;
    else if (el1_pt > 25 && el1_pt < 30 && !(el1_eta > 1.44 && el1_eta < 1.56)) leg1_w = 0.83;
    else if (el1_pt > 25 && el1_pt < 30 && el1_eta > 1.44 && el1_eta < 1.56) leg1_w = 0.68;
    else if (el1_pt > 30 && el1_pt < 40 && !(el1_eta > 1.44 && el1_eta < 1.56)) leg1_w = 0.90;
    else if (el1_pt > 30 && el1_pt < 40 && el1_eta > 1.44 && el1_eta < 1.56) leg1_w = 0.78;
    else if (el1_pt > 40 && el1_pt < 50 && !(el1_eta > 1.44 && el1_eta < 1.56)) leg1_w = 0.92;
    else if (el1_pt > 40 && el1_pt < 50 && el1_eta > 1.44 && el1_eta < 1.56) leg1_w = 0.83;
    else if (el1_pt > 50 && el1_pt < 100 && !(el1_eta > 1.44 && el1_eta < 1.56)) leg1_w = 0.95;
    else if (el1_pt > 50 && el1_pt < 100 && el1_eta > 1.44 && el1_eta < 1.56) leg1_w = 0.83;
    else if (el1_pt > 100 && !(el1_eta > 1.44 && el1_eta < 1.56)) leg1_w = 0.97;
    else if (el1_pt > 100 && el1_eta > 1.44 && el1_eta < 1.56) leg1_w = 0.91;
    if (el2_pt > 12 && el2_pt < 14 && el2_eta > 0.00 && el2_eta < 0.80) leg2_w = 0.0;
    else if (el2_pt > 12 && el2_pt < 14 && el2_eta > 0.80 && el2_eta < 1.44) leg2_w = 0.0;
    else if (el2_pt > 12 && el2_pt < 14 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.0;
    else if (el2_pt > 12 && el2_pt < 14 && el2_eta > 1.56 && el2_eta < 2.50) leg2_w = 0.38;
    else if (el2_pt > 14 && el2_pt < 17 && el2_eta > 0.00 && el2_eta < 0.80) leg2_w = 0.78;
    else if (el2_pt > 14 && el2_pt < 17 && el2_eta > 0.80 && el2_eta < 1.44) leg2_w = 0.70;
    else if (el2_pt > 14 && el2_pt < 17 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.52;
    else if (el2_pt > 14 && el2_pt < 17 && el2_eta > 1.56 && el2_eta < 2.50) leg2_w = 0.57;
    else if (el2_pt > 17 && el2_pt < 20 && el2_eta > 0.00 && el2_eta < 0.80) leg2_w = 0.80;
    else if (el2_pt > 17 && el2_pt < 20 && el2_eta > 0.80 && el2_eta < 1.44) leg2_w = 0.75;
    else if (el2_pt > 17 && el2_pt < 20 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.54;
    else if (el2_pt > 17 && el2_pt < 20 && el2_eta > 1.56 && el2_eta < 2.50) leg2_w = 0.70;
    else if (el2_pt > 20 && el2_pt < 25 && !(el2_eta > 1.44 && el2_eta < 1.56)) leg2_w = 0.80;
    else if (el2_pt > 20 && el2_pt < 25 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.65;
    else if (el2_pt > 25 && el2_pt < 27 && !(el2_eta > 1.44 && el2_eta < 1.56)) leg2_w = 0.82;
    else if (el2_pt > 25 && el2_pt < 27 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.70;
    else if (el2_pt > 27 && el2_pt < 30 && !(el2_eta > 1.44 && el2_eta < 1.56)) leg2_w = 0.85;
    else if (el2_pt > 27 && el2_pt < 30 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.73;
    else if (el2_pt > 30 && el2_pt < 40 && !(el2_eta > 1.44 && el2_eta < 1.56)) leg2_w = 0.88;
    else if (el2_pt > 30 && el2_pt < 40 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.76;
    else if (el2_pt > 40 && el2_pt < 60 && !(el2_eta > 1.44 && el2_eta < 1.56)) leg2_w = 0.92;
    else if (el2_pt > 40 && el2_pt < 60 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.82;
    else if (el2_pt > 60 && !(el2_eta > 1.44 && el2_eta < 1.56)) leg2_w = 0.97;
    else if (el2_pt > 60 && el2_eta > 1.44 && el2_eta < 1.56) leg2_w = 0.91;
    return (1.0-leg1_w*leg2_w);
  });

  //true if muons out of hlt pt acceptance
  const NamedFunc mu_oohltpt("mu_oohltpt",[](const Baby &b) -> NamedFunc::ScalarType{
    float mu1_pt = 0;
    float mu2_pt = 0;
    for (unsigned imc = 0; imc < b.GenPart_pdgId()->size(); imc++) {
      int abs_pdgid = abs(b.GenPart_pdgId()->at(imc));
      if (abs_pdgid==13) {
        int mom_idx = b.GenPart_genPartIdxMother()->at(imc);
        if (mom_idx != -1) {
          if ((b.GenPart_pdgId()->at(mom_idx))==23) {
            if (mu1_pt <= 0)
              mu1_pt = b.GenPart_pt()->at(imc);
            else
              mu2_pt = b.GenPart_pt()->at(imc);
          }
        }
      }
    }
    if ((mu1_pt > 17 && mu2_pt > 8) || (mu1_pt > 8 && mu2_pt > 17)) return false;
    return true;
  });

  const NamedFunc mu_sig("mu_sig",[](const Baby &b) -> NamedFunc::VectorType{
    std::vector<double> mu_sig_;
    for (unsigned imu = 0; imu < b.Muon_pt()->size(); imu++) {
      bool is_sig = true;
      if (b.Muon_pt()->at(imu) < 5) is_sig = false;
      if (abs(b.Muon_eta()->at(imu)) > 2.4) is_sig = false;
      if (abs(b.Muon_dz()->at(imu))>1.0)  is_sig = false;
      if (abs(b.Muon_dxy()->at(imu))>0.5) is_sig = false; 
      if (!((b.Muon_looseId()->at(imu) || (b.Muon_pt()->at(imu) > 200 && b.Muon_highPtId()->at(imu))) && 
          b.Muon_pfRelIso03_all()->at(imu) < 0.35 &&
          b.Muon_sip3d()->at(imu) < 4)) is_sig = false;
      if (is_sig)
        mu_sig_.push_back(1.0);
      else
        mu_sig_.push_back(0.0);
    }
    return mu_sig_;
  });

  const NamedFunc offline_nmu("offline_nmu",[mu_sig](const Baby &b) -> NamedFunc::ScalarType{
    int nmu_sig = 0;
    std::vector<double> mu_sig_ = mu_sig.GetVector(b);
    for (double imu_sig_ : mu_sig_) {
      if (imu_sig_ > 0.5)
        nmu_sig++;
    }
    return nmu_sig;
  });

  const NamedFunc el_sig("el_sig",[](const Baby &b) -> NamedFunc::VectorType{
    std::vector<double> el_sig_;
    for (unsigned iel = 0; iel < b.Electron_pt()->size(); iel++) {
      bool is_sig = true;
      if (b.Electron_pt()->at(iel) < 7) is_sig = false;
      float etasc = b.Electron_deltaEtaSC()->at(iel) + b.Electron_eta()->at(iel);
      if (abs(etasc) > 2.5) is_sig = false;
      if (abs(b.Electron_dz()->at(iel))>1.0) is_sig = false;
      if (abs(b.Electron_dxy()->at(iel))>0.5) is_sig = false; 
      double wp[2][3] = {{-0.145237, -0.0315746, -0.032173},
                         { 0.604775,  0.628743,   0.896462}};
      int ipt(1), ieta(2);
      if(b.Electron_pt()->at(iel)>10) ipt = 0;
      if(fabs(etasc) < 0.8) ieta = 0;
      else if(fabs(etasc) < 1.479) ieta = 1;
      double mva = b.Electron_mvaFall17V2Iso()->at(iel);
      if (mva <= wp[ipt][ieta])
        is_sig = false;
      if (is_sig)
        el_sig_.push_back(1.0);
      else
        el_sig_.push_back(0.0);
    }
    return el_sig_;
  });

  const NamedFunc el_gap("el_gap",[el_sig](const Baby &b) -> NamedFunc::ScalarType{
    std::vector<double> el_sig_ = el_sig.GetVector(b);
    for (unsigned iel = 0; iel < b.Electron_pt()->size(); iel++) {
      if (el_sig_[iel] > 0.5) {
        float etasc = abs(b.Electron_deltaEtaSC()->at(iel) + b.Electron_eta()->at(iel));
        float eta = abs(b.Electron_eta()->at(iel));
        if (eta > 1.44 && eta < 1.56) return true;
        if (etasc > 1.44 && etasc < 1.56) return true;
      }
    }
    return false;
  });

  const NamedFunc offline_nel("offline_nel",[el_sig](const Baby &b) -> NamedFunc::ScalarType{
    int nel_sig = 0;
    std::vector<double> el_sig_ = el_sig.GetVector(b);
    for (double iel_sig_ : el_sig_) {
      if (iel_sig_ > 0.5)
        nel_sig++;
    }
    return nel_sig;
  });

  const NamedFunc offline_nph("offline_nph",[](const Baby &b) -> NamedFunc::ScalarType{
    int nph_sig = 0;
    for (unsigned iph = 0; iph < b.Photon_pt()->size(); iph++) {
      if (b.Photon_pt()->at(iph) < 15) continue;
      if (abs(b.Photon_eta()->at(iph)) > 2.5) continue;
      if (!(b.Photon_isScEtaEB()->at(iph) || b.Photon_isScEtaEE()->at(iph))) continue;
      // if (b.Photon_pfRelIso03_all()->at(iph)==PhotonRelIsoCut) continue; // no isolation cut in 2016...?
      if (b.Photon_mvaID()->at(iph) > 0.2 && 
          b.Photon_electronVeto()->at(iph))
      nph_sig++;
    }
    return nph_sig;
  });

  //------------------------------------------------------------------------------------
  //                                     make plots and pie charts
  //------------------------------------------------------------------------------------
  
  PlotMaker pm;

    pm.Push<Hist1D>(Axis(20, 0.0, 100.0, hlt_max_el_pt, "HLT Electron p_{T}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger_plus,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__hltelpt__failhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(27, 0.0, 90.0, "Electron_pt", "Electron p_{T}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__elpt__passhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(27, 0.0, 90.0, "Electron_pt", "Electron p_{T}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__elpt__failhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, -2.5, 2.5, "Electron_eta", "Electron #eta", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__eleta__passhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, -2.5, 2.5, "Electron_eta", "Electron #eta", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__eleta__failhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, -2.5, 2.5, "Electron_eta", "Electron #eta", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==1)&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__eleta__failhltreco")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Electron_pfRelIso03_all", "Electron I_{rel}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__elreliso__passhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Electron_pfRelIso03_all", "Electron I_{rel}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__elreliso__failhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Electron_pfRelIso03_all", "Electron I_{rel}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==2)&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__elreliso__failhltiso")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Electron_pfRelIso03_chg", "Electron I_{rel,chg}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__elrelisochg__passhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Electron_pfRelIso03_chg", "Electron I_{rel,chg}", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__elrelisochg__failhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Muon_pfRelIso03_all", "Muon I_{rel}", {}),
        (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&hlt_mu_trigger&&mu_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__mureliso__passhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Muon_pfRelIso03_all", "Muon I_{rel}", {}),
        (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger&&mu_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__mureliso__failhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist1D>(Axis(30, 0.0, 2.0, "Muon_pfRelIso03_all", "Muon I_{rel}", {}),
        (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==2)&&mu_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__mureliso__failhltiso")
        .LuminosityTag(total_luminosity_string);


    pm.Push<Hist2D>(Axis(15, -2.5, 2.5, "Electron_eta", "Electron #eta", {}), Axis(10, -3.14, 3.14, "Electron_phi", "Electron #phi", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__eletaphi__passhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist2D>(Axis(15, -2.5, 2.5, "Electron_eta", "Electron #eta", {}), Axis(10, -3.14, 3.14, "Electron_phi", "Electron #phi", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__eletaphi__failhlt")
        .LuminosityTag(total_luminosity_string);
    pm.Push<Hist2D>(Axis(15, -2.5, 2.5, "Electron_eta", "Electron #eta", {}), Axis(10, -3.14, 3.14, "Electron_phi", "Electron #phi", {}),
        (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==1)&&el_sig,
        procs, plt_lin).Weight(weight)
        .Tag("FixName:zghlt__eletaphi__failhltreco")
        .LuminosityTag(total_luminosity_string);

    //pm.Push<Hist1D>(Axis(60, -30.0, 30.0, "GenPart_pdgId", "PDGID", {}),
    //    "1",
    //    procs, plt_lin).Weight(weight)
    //    .Tag("FixName:nano__test")
    //    .LuminosityTag(total_luminosity_string);
    //pm.Push<Hist1D>(Axis(8, -0.5, 7.5, "nJet", "N_{j}", {}),
    //    "1",
    //    procs, plt_lin)
    //    .Tag("FixName:nano__test_1").Weight("1")
    //    .LuminosityTag(total_luminosity_string);
    //pm.Push<Hist1D>(Axis(30, 0, 300.0, "Jet_pt", "Jet p_{T}", {}),
    //    "1",
    //    procs, plt_lin)
    //    .Tag("FixName:nano__test_2").Weight("1")
    //    .LuminosityTag(total_luminosity_string);
    pm.Push<Table>("zgtrig_el_"+options.year_string, vector<TableRow>{
      //TableRow("L1 Triggers", 
      //    (z_decay_pdgid==11)&&l1_el_trigger,0,0,weight),
      //TableRow("HLT Triggers", 
      //    (z_decay_pdgid==11)&&l1_el_trigger&&hlt_el_trigger,0,0,weight),

      TableRow("$Z\\rightarrow e^{+}e^{-}$ decays", 
          (z_decay_pdgid==11),0,0,weight),
      TableRow("Offline electrons in acceptance", 
          (z_decay_pdgid==11)&&(offline_nel>=2),0,0,weight),
      TableRow("Offline photon in acceptance", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1),0,0,weight),
      TableRow("L1 Triggers", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger,0,0,weight),
      TableRow("HLT Triggers", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger,0,0,weight),
      TableRow("HLT Triggers (incl el)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger_plus,0,0,weight),
      TableRow("Events that fail HLT - $0\\gamma 0e$", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger_plus&&(nel_id_hlt<1)&&"!(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $0\\gamma 1e$", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger_plus&&(nel_id_hlt==1)&&"!(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $0\\gamma \\geq 1e$", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger_plus&&(nel_id_hlt>=2)&&"!(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $1\\gamma 0e$", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger_plus&&(nel_id_hlt<1)&&"(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $1\\gamma 1e$", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger_plus&&(nel_id_hlt==1)&&"(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $1\\gamma \\geq 1e$", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger_plus&&(nel_id_hlt>=2)&&"(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("HLT Triggers (incl Ele27Ph50)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon50),0,0,weight),
      TableRow("HLT Triggers (incl Ele27Ph33)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon33),0,0,weight),
      TableRow("HLT Triggers (incl Ele27Ph25)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon25),0,0,weight),
      TableRow("HLT Triggers (incl Ele27Ph20)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele27_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose),0,0,weight),
      TableRow("HLT Triggers (incl Ele22Ph50)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon50),0,0,weight),
      TableRow("HLT Triggers (incl Ele22Ph33)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon33),0,0,weight),
      TableRow("HLT Triggers (incl Ele22Ph25)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon25),0,0,weight),
      TableRow("HLT Triggers (incl Ele22Ph20)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele22_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose),0,0,weight),
      TableRow("HLT Triggers (incl Ele17Ele8Ph25)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele17_Ele8_CaloIdL_TrackIdL_IsoVL_Photon25),0,0,weight),
      TableRow("HLT Triggers (incl Ele17Ele8Ph20)", 
          (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&(hlt_el_trigger_plus||HLT_Ele17_Ele8_CaloIdL_TrackIdL_IsoVL_Photon20_HoverELoose),0,0,weight),
      //TableRow("HLT Triggers (incl el, el+ph)", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&hlt_el_trigger_plusplus,0,0,weight),
      //TableRow("Electrons out of HLT p_{T} acceptance", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&el_oohltpt,0,0,weight),
      //TableRow("Electrons nearly out of HLT p_{T} acceptance", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&el_noohltpt,0,0,weight),
      //TableRow("Expected events with electrons failing HLT", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&el_oohltpt,0,0,weight*hlt_fail_weights),
      //TableRow("HLT fail reason: unknown", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==0.0),0,0,weight),
      //TableRow("HLT fail reason: reconstruction", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==1),0,0,weight),
      //TableRow("HLT fail reason: reconstruction, e in gap", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==1)&&el_gap,0,0,weight),
      //TableRow("HLT fail reason: id/iso", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==2),0,0,weight),
      //TableRow("HLT fail reason: top leg", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==3),0,0,weight),
      //TableRow("HLT fail reason: bottom leg", 
      //    (z_decay_pdgid==11)&&(offline_nel>=2)&&(offline_nph>=1)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==4),0,0,weight),

      TableRow("$Z\\rightarrow \\mu^{+}\\mu^{-}$ decays", 
          (z_decay_pdgid==13),0,0,weight),
      TableRow("Offline muons in acceptance", 
          (z_decay_pdgid==13)&&(offline_nmu>=2),0,0,weight),
      TableRow("Offline photon in acceptance", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1),0,0,weight),
      TableRow("L1 Triggers", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger,0,0,weight),
      TableRow("HLT Triggers", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&hlt_mu_trigger,0,0,weight),
      TableRow("HLT Triggers (incl mu)", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&hlt_mu_trigger_plus,0,0,weight),
      TableRow("Events that fail HLT - $0\\gamma 0 \\mu$", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger_plus&&nmu_id_hlt<1&&"!(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $0\\gamma 1 \\mu$", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger_plus&&nmu_id_hlt==1&&"!(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $0\\gamma 2 \\mu$", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger_plus&&nmu_id_hlt>=2&&"!(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $1\\gamma 0 \\mu$", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger_plus&&nmu_id_hlt<1&&"(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $1\\gamma 1 \\mu$", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger_plus&&nmu_id_hlt==1&&"(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("Events that fail HLT - $1\\gamma 2 \\mu$", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger_plus&&nmu_id_hlt>=2&&"(HLT_Photon25||HLT_Photon20_HoverELoose)",0,0,weight),
      TableRow("HLT Triggers (incl mu, mu+ph)", 
          (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&hlt_mu_trigger_plusplus,0,0,weight),
      //TableRow("Muons out of HLT acceptance", 
      //    (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&hlt_mu_trigger&&mu_oohltpt,0,0,weight),
      //TableRow("HLT fail reason: unknown", 
      //    (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==0.0),0,0,weight),
      //TableRow("HLT fail reason: reconstruction", 
      //    (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==1),0,0,weight),
      //TableRow("HLT fail reason: iso", 
      //    (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==2),0,0,weight),
      //TableRow("HLT fail reason: top leg", 
      //    (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==3),0,0,weight),
      //TableRow("HLT fail reason: bottom leg", 
      //    (z_decay_pdgid==13)&&(offline_nmu>=2)&&(offline_nph>=1)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==4),0,0,weight),

      TableRow("$Z\\rightarrow e^{+}e^{-}$ decays", 
          (z_decay_pdgid==11),0,0,weight),
      TableRow("L1 Triggers", 
          (z_decay_pdgid==11)&&l1_el_trigger,0,0,weight),
      TableRow("HLT Triggers", 
          (z_decay_pdgid==11)&&l1_el_trigger&&hlt_el_trigger,0,0,weight),
      TableRow("HLT fail reason: unknown", 
          (z_decay_pdgid==11)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==0.0),0,0,weight),
      TableRow("HLT fail reason: reconstruction", 
          (z_decay_pdgid==11)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==1),0,0,weight),
      TableRow("HLT fail reason: reconstruction, e in gap", 
          (z_decay_pdgid==11)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==1)&&el_gap,0,0,weight),
      TableRow("HLT fail reason: id/iso", 
          (z_decay_pdgid==11)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==2),0,0,weight),
      TableRow("HLT fail reason: top leg", 
          (z_decay_pdgid==11)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==3),0,0,weight),
      TableRow("HLT fail reason: bottom leg", 
          (z_decay_pdgid==11)&&l1_el_trigger&&!hlt_el_trigger&&(el_hlt_fail_reason==4),0,0,weight),

      TableRow("$Z\\rightarrow \\mu^{+}\\mu^{-}$ decays", 
          (z_decay_pdgid==13),0,0,weight),
      TableRow("L1 Triggers", 
          (z_decay_pdgid==13)&&l1_mu_trigger,0,0,weight),
      TableRow("HLT Triggers", 
          (z_decay_pdgid==13)&&l1_mu_trigger&&hlt_mu_trigger,0,0,weight),
      TableRow("HLT fail reason: unknwon", 
          (z_decay_pdgid==13)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==0.0),0,0,weight),
      TableRow("HLT fail reason: reconstruction", 
          (z_decay_pdgid==13)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==1),0,0,weight),
      TableRow("HLT fail reason: iso", 
          (z_decay_pdgid==13)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==2),0,0,weight),
      TableRow("HLT fail reason: top leg", 
          (z_decay_pdgid==13)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==3),0,0,weight),
      TableRow("HLT fail reason: bottom leg", 
          (z_decay_pdgid==13)&&l1_mu_trigger&&!hlt_mu_trigger&&(mu_hlt_fail_reason==4),0,0,weight),

      //TableRow("$Z\\rightarrow e^{+}e^{-}$ decays, not in l1 acceptance", 
      //    (z_decay_pdgid==11)&&((nel_ooeta>=2)||(nel_ool1pt)),0,0,weight),
      //TableRow("$Z\\rightarrow e^{+}e^{-}$ decays, not eta acceptance", 
      //    (z_decay_pdgid==11)&&((nel_ooeta>=2)),0,0,weight),
      //TableRow("$Z\\rightarrow e^{+}e^{-}$ decays, not l1 pt acceptance", 
      //    (z_decay_pdgid==11)&&((nel_ool1pt)),0,0,weight),
      //TableRow("$Z\\rightarrow e^{+}e^{-}$ decays, offline+L1 but not in hlt pt acceptance", 
      //    (z_decay_pdgid==11)&&offline_selection&&l1_el_trigger&&nel_oohltpt,0,0,weight),
      //TableRow("HLT Triggers", 
      //    (z_decay_pdgid==11)&&l1_el_trigger&&hlt_el_trigger,0,0,weight),
    },procs,false,true,false,false,false,true).LuminosityTag(total_luminosity_string).Precision(2);
  
  pm.multithreaded_ = !options.single_thread;
  pm.min_print_ = true;
  pm.MakePlots(1.);

  //------------------------------------------------------------------------------------
  //                                     post processing
  //------------------------------------------------------------------------------------

  time(&endtime); 
  cout<<endl<<"Took "<<difftime(endtime, begtime)<<" seconds"<<endl<<endl;
}


// Copyright 2019-2020 CERN and copyright holders of ALICE O2.
// See https://alice-o2.web.cern.ch/copyright for details of the copyright holders.
// All rights not expressly granted are reserved.
//
// This software is distributed under the terms of the GNU General Public
// License v3 (GPL Version 3), copied verbatim in the file "COPYING".
//
// In applying this license CERN does not waive the privileges and immunities
// granted to it by virtue of its status as an Intergovernmental Organization
// or submit itself to any jurisdiction.

// O2 includes
#include "ReconstructionDataFormats/Track.h"
#include "Framework/AnalysisTask.h"
#include "Framework/AnalysisDataModel.h"
#include "Framework/ASoAHelpers.h"
#include "Common/Core/PID/PIDResponse.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "DataModel/LFDerived.h"

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;

void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  std::vector<ConfigParamSpec> options{
    {"add-tof-histos", VariantType::Int, 0, {"Generate TPC with TOF histograms"}}};
  std::swap(workflowOptions, options);
}

#include "Framework/runDataProcessing.h"

constexpr int Np = 9;
struct TPCSpectraAnalyserTask {

  static constexpr const char* pT[Np] = {"e", "#mu", "#pi", "K", "p", "d", "t", "^{3}He", "#alpha"};
  static constexpr std::string_view hp[Np] = {"p/El", "p/Mu", "p/Pi", "p/Ka", "p/Pr", "p/De", "p/Tr", "p/He", "p/Al"};
  static constexpr std::string_view hpt[Np] = {"pt/El", "pt/Mu", "pt/Pi", "pt/Ka", "pt/Pr", "pt/De", "pt/Tr", "pt/He", "pt/Al"};
  HistogramRegistry histos{"Histos", {}, OutputObjHandlingPolicy::AnalysisObject};

  void init(o2::framework::InitContext&)
  {
    histos.add("p/Unselected", "Unselected;#it{p} (GeV/#it{c})", kTH1F, {{100, 0, 20}});
    histos.add("pt/Unselected", "Unselected;#it{p}_{T} (GeV/#it{c})", kTH1F, {{100, 0, 20}});
    for (int i = 0; i < Np; i++) {
      histos.add(hp[i].data(), Form("%s;#it{p} (GeV/#it{c})", pT[i]), kTH1F, {{100, 0, 20}});
      histos.add(hpt[i].data(), Form("%s;#it{p}_{T} (GeV/#it{c})", pT[i]), kTH1F, {{100, 0, 20}});
    }
  }

  Configurable<float> nsigmacut{"nsigmacut", 3, "Value of the Nsigma cut"};
  Configurable<float> cfgCutVertex{"cfgCutVertex", 10.0f, "Accepted z-vertex range"};
  Configurable<float> cfgCutEta{"cfgCutEta", 0.8f, "Eta range for tracks"};

  template <std::size_t i, typename T>
  void fillParticleHistos(const T& track, const float nsigma[])
  {
    if (abs(nsigma[i]) > nsigmacut.value) {
      return;
    }
    histos.fill(HIST(hp[i]), track.p());
    histos.fill(HIST(hpt[i]), track.pt());
  }

  Filter collisionFilter = nabs(aod::collision::posZ) < cfgCutVertex; //collision filters not doing anything now?
  Filter trackFilter = nabs(aod::lftrack::eta) < cfgCutEta;

  void process(soa::Filtered<aod::LFTracks>::iterator const& track)
  {
    const float nsigma[Np] = {track.tpcNSigmaEl(), track.tpcNSigmaMu(), track.tpcNSigmaPi(),
                              track.tpcNSigmaKa(), track.tpcNSigmaPr(), track.tpcNSigmaDe(),
                              track.tpcNSigmaTr(), track.tpcNSigmaHe(), track.tpcNSigmaAl()};
    histos.fill(HIST("p/Unselected"), track.p());
    histos.fill(HIST("pt/Unselected"), track.pt());

    fillParticleHistos<0>(track, nsigma);
    fillParticleHistos<1>(track, nsigma);
    fillParticleHistos<2>(track, nsigma);
    fillParticleHistos<3>(track, nsigma);
    fillParticleHistos<4>(track, nsigma);
    fillParticleHistos<5>(track, nsigma);
    fillParticleHistos<6>(track, nsigma);
    fillParticleHistos<7>(track, nsigma);
    fillParticleHistos<8>(track, nsigma);
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  WorkflowSpec workflow{adaptAnalysisTask<TPCSpectraAnalyserTask>(cfgc, TaskName{"tpcspectra-task-skim-analyser"})};
  return workflow;
}

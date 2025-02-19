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

#include "Common/DataModel/EventSelection.h"
#include "Common/DataModel/TrackSelectionTables.h"
#include "Common/DataModel/Centrality.h"

#include "Framework/HistogramRegistry.h"

#include <TLorentzVector.h>
#include <TMath.h>
#include <TObjArray.h>

#include <cmath>

using namespace o2;
using namespace o2::framework;
using namespace o2::framework::expressions;

void customize(std::vector<o2::framework::ConfigParamSpec>& workflowOptions)
{
  std::vector<ConfigParamSpec> options{
    {"add-vertex", VariantType::Int, 1, {"Vertex plots"}},
    {"add-gen", VariantType::Int, 1, {"Generated plots"}},
    {"add-rec", VariantType::Int, 1, {"Reconstructed plots"}}};
  std::swap(workflowOptions, options);
}

#include "Framework/runDataProcessing.h" // important to declare after the options

struct NucleiSpectraEfficiencyLightVtx {
  OutputObj<TH1F> histVertexTrueZ{TH1F("histVertexTrueZ", "MC true z position of z-vertex; vertex z (cm)", 200, -20., 20.)};

  void process(aod::McCollision const& mcCollision)
  {
    histVertexTrueZ->Fill(mcCollision.posZ());
  }
};

struct NucleiSpectraEfficiencyLightGen {

  HistogramRegistry spectra{"spectraGen", {}, OutputObjHandlingPolicy::AnalysisObject, true, true};

  void init(o2::framework::InitContext&)
  {
    std::vector<double> ptBinning = {0.0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6,
                                     1.8, 2.0, 2.2, 2.4, 2.8, 3.2, 3.6, 4., 5., 6., 8., 10., 12., 14.};
    //
    AxisSpec ptAxis = {ptBinning, "#it{p}_{T} (GeV/#it{c})"};
    //
    spectra.add("histGenPtPion", "generated particles", HistType::kTH1F, {ptAxis});
    spectra.add("histGenPtProton", "generated particles", HistType::kTH1F, {ptAxis});
    spectra.add("histGenPtHe3", "generated particles", HistType::kTH1F, {ptAxis});
  }

  void process(aod::McCollision const& mcCollision, aod::McParticles_000& mcParticles)
  {
    //
    // loop over generated particles and fill generated particles
    //
    for (auto& mcParticleGen : mcParticles) {
      if (!mcParticleGen.isPhysicalPrimary()) {
        continue;
      }
      if (abs(mcParticleGen.y()) > 0.5) {
        continue;
      }
      //
      if (mcParticleGen.pdgCode() == 211) {
        spectra.fill(HIST("histGenPtPion"), mcParticleGen.pt());
      }
      if (mcParticleGen.pdgCode() == -2212) {
        spectra.fill(HIST("histGenPtProton"), mcParticleGen.pt());
      }
      if (mcParticleGen.pdgCode() == -1000020030) {
        spectra.fill(HIST("histGenPtHe3"), mcParticleGen.pt());
      }
    }
  }
};

struct NucleiSpectraEfficiencyLightRec {

  HistogramRegistry spectra{"spectraRec", {}, OutputObjHandlingPolicy::AnalysisObject, true, true};

  void init(o2::framework::InitContext&)
  {
    std::vector<double> ptBinning = {0.0, 0.05, 0.1, 0.15, 0.2, 0.3, 0.4, 0.6, 0.8, 1.0, 1.2, 1.4, 1.6,
                                     1.8, 2.0, 2.2, 2.4, 2.8, 3.2, 3.6, 4., 5., 6., 8., 10., 12., 14.};
    //
    AxisSpec ptAxis = {ptBinning, "#it{p}_{T} (GeV/#it{c})"};
    //
    spectra.add("histEvSel", "eventselection", HistType::kTH1D, {{10, -0.5, 9.5}});
    spectra.add("histRecVtxZ", "collision z position", HistType::kTH1F, {{200, -20., +20., "z position (cm)"}});
    spectra.add("histRecPtPion", "reconstructed particles", HistType::kTH1F, {ptAxis});
    spectra.add("histRecPtProton", "reconstructed particles", HistType::kTH1F, {ptAxis});
    spectra.add("histRecPtHe3", "reconstructed particles", HistType::kTH1F, {ptAxis});
    spectra.add("histTpcSignal", "Specific energy loss", HistType::kTH2F, {{600, -6., 6., "#it{p/z} (GeV/#it{c})"}, {1400, 0, 1400, "d#it{E} / d#it{X} (a. u.)"}});
    spectra.add("histTofSignalData", "TOF signal", HistType::kTH2F, {{600, -6., 6., "#it{p} (GeV/#it{c})"}, {500, 0.0, 1.2, "#beta (TOF)"}});
    spectra.add("histTpcNsigmaHe3", "n-sigmaHe3 TPC", HistType::kTH2F, {ptAxis, {200, -100., +100., "n#sigma_{He} (a. u.)"}});
    spectra.add("histTpcNsigmaPr", "n-sigmaPr TPC", HistType::kTH2F, {ptAxis, {200, -100., +100., "n#sigma_{Pr} (a. u.)"}});
    spectra.add("histTpcNsigmaPi", "n-sigmaPi TPC", HistType::kTH2F, {ptAxis, {200, -100., +100., "n#sigma_{Pi} (a. u.)"}});
    spectra.add("histItsClusters", "number of ITS clusters", HistType::kTH1F, {{10, -0.5, +9.5, "number of ITS clusters"}});
    spectra.add("histDcaXYprimary", "dca XY primary particles", HistType::kTH1F, {{200, -1., +1., "dca XY (cm)"}});
    spectra.add("histDcaXYsecondary", "dca XY secondary particles", HistType::kTH1F, {{200, -1., +1., "dca XY (cm)"}});
  }

  Configurable<float> cfgCutVertex{"cfgCutVertex", 10.0f, "Accepted z-vertex range"};
  Configurable<float> cfgCutEta{"cfgCutEta", 0.8f, "Eta range for tracks"};
  Configurable<float> nsigmacutLow{"nsigmacutLow", -20.0, "Value of the Nsigma cut"};
  Configurable<float> nsigmacutHigh{"nsigmacutHigh", +20.0, "Value of the Nsigma cut"};

  Filter collisionFilter = nabs(aod::collision::posZ) < cfgCutVertex;
  //Filter trackFilter = (nabs(aod::track::eta) < cfgCutEta) && (requireGlobalTrackInFilter());

  using TrackCandidates = soa::Join<aod::Tracks, aod::TracksExtra, aod::McTrackLabels, aod::pidTPCFullHe, aod::pidTPCFullPr, aod::pidTPCFullPi>;

  void process(soa::Filtered<soa::Join<aod::Collisions, aod::McCollisionLabels, aod::EvSels>>::iterator const& collision,
               TrackCandidates const& tracks, aod::McParticles_000& mcParticles, aod::McCollisions const& mcCollisions)
  {
    //
    // check event selection
    //
    spectra.get<TH1>(HIST("histEvSel"))->Fill("all", 1.f);
    if (collision.sel8()) {
      spectra.get<TH1>(HIST("histEvSel"))->Fill("sel8", 1.f);
    } else {
      return;
    }
    //
    // check the vertex-z distribution
    //
    spectra.fill(HIST("histRecVtxZ"), collision.posZ());
    //
    // loop over reconstructed particles and fill reconstructed tracks
    //
    for (auto track : tracks) {
      //
      // apply minimal cuts
      //
      if (track.itsNCls() == 0) {
        continue;
      }
      //
      // fill QA histograms
      //
      float nSigmaHe3 = track.tpcNSigmaHe();
      nSigmaHe3 += 94.222101 * TMath::Exp(-0.905203 * track.tpcInnerParam());
      float nSigmaPr = track.tpcNSigmaPr();
      float nSigmaPi = track.tpcNSigmaPi();
      //
      // TPC-QA
      //
      spectra.fill(HIST("histTpcSignal"), track.tpcInnerParam() * track.sign(), track.tpcSignal());
      //
      spectra.fill(HIST("histTpcNsigmaPi"), track.tpcInnerParam(), nSigmaPi);
      spectra.fill(HIST("histTpcNsigmaPr"), track.tpcInnerParam(), nSigmaPr);
      spectra.fill(HIST("histTpcNsigmaHe3"), track.tpcInnerParam(), nSigmaHe3);
      //
      // ITS-QA
      //
      spectra.fill(HIST("histItsClusters"), track.itsNCls());
      //
      /*
      // TOF-QA
      //
      if (track.hasTOF()) {
        Float_t tofTime = track.tofSignal();
        Float_t tofLength = track.length();
        Float_t beta = tofLength / (TMath::C() * 1e-10 * tofTime);
        spectra.fill(HIST("histTofSignalData"), track.tpcInnerParam() * track.sign(), beta);
      }
      */
      //
      // dca to primary vertex -- wait for tracksextented
      //
      //spectra.fill(HIST("histDcaXYprimary"), track.dcaXY());
      //
      // fill histograms for pions
      //
      if (nSigmaPi > nsigmacutLow && nSigmaPi < nsigmacutHigh) {
        // check on perfect PID
        if (track.mcParticle_as<aod::McParticles_000>().pdgCode() == 211 && track.mcParticle_as<aod::McParticles_000>().isPhysicalPrimary()) {
          TLorentzVector lorentzVector{};
          lorentzVector.SetPtEtaPhiM(track.pt(), track.eta(), track.phi(), constants::physics::MassPionCharged);
          if (lorentzVector.Rapidity() > -0.5 && lorentzVector.Rapidity() < 0.5) {
            spectra.fill(HIST("histRecPtPion"), track.pt());
          }
        }
      }
      //
      // fill histograms for protons
      //
      if (nSigmaPr > nsigmacutLow && nSigmaPr < nsigmacutHigh) {
        // check on perfect PID
        if (track.mcParticle_as<aod::McParticles_000>().pdgCode() == -2212 && track.mcParticle_as<aod::McParticles_000>().isPhysicalPrimary()) {
          TLorentzVector lorentzVector{};
          lorentzVector.SetPtEtaPhiM(track.pt(), track.eta(), track.phi(), constants::physics::MassProton);
          if (lorentzVector.Rapidity() > -0.5 && lorentzVector.Rapidity() < 0.5) {
            spectra.fill(HIST("histRecPtProton"), track.pt());
          }
        }
      }
      //
      // fill histograms for he3
      //
      if (nSigmaHe3 > nsigmacutLow && nSigmaHe3 < nsigmacutHigh) {
        // check on perfect PID
        if (track.mcParticle_as<aod::McParticles_000>().pdgCode() == -1000020030) {
          TLorentzVector lorentzVector{};
          lorentzVector.SetPtEtaPhiM(track.pt() * 2.0, track.eta(), track.phi(), constants::physics::MassHelium3);
          if (lorentzVector.Rapidity() > -0.5 && lorentzVector.Rapidity() < 0.5) {
            // fill reconstructed histogram
            spectra.fill(HIST("histRecPtHe3"), track.pt() * 2.0);
          }
        }
      }
    }
  }
};

WorkflowSpec defineDataProcessing(ConfigContext const& cfgc)
{
  const bool vertex = cfgc.options().get<int>("add-vertex");
  const bool gen = cfgc.options().get<int>("add-gen");
  const bool rec = cfgc.options().get<int>("add-rec");
  //
  WorkflowSpec workflow{};
  //
  if (vertex) {
    workflow.push_back(adaptAnalysisTask<NucleiSpectraEfficiencyLightVtx>(cfgc, TaskName{"nuclei-efficiency-vtx"}));
  }
  if (gen) {
    workflow.push_back(adaptAnalysisTask<NucleiSpectraEfficiencyLightGen>(cfgc, TaskName{"nuclei-efficiency-gen"}));
  }
  if (rec) {
    workflow.push_back(adaptAnalysisTask<NucleiSpectraEfficiencyLightRec>(cfgc, TaskName{"nuclei-efficiency-rec"}));
  }
  //
  return workflow;
}

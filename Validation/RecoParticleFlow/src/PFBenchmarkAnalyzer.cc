#include "Validation/RecoParticleFlow/interface/PFBenchmarkAnalyzer.h"
// author: Mike Schmitt, University of Florida
// first version 11/7/2007

#include "FWCore/MessageLogger/interface/MessageLogger.h"
#include "FWCore/ServiceRegistry/interface/Service.h"

#include "FWCore/Framework/interface/Event.h"
#include "DataFormats/Common/interface/Handle.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DQMServices/Core/interface/DaqMonitorBEInterface.h"
#include "DQMServices/Daemon/interface/MonitorDaemon.h"

#include "DataFormats/Candidate/interface/CandidateFwd.h"
#include "DataFormats/Candidate/interface/Candidate.h"

//#include "DataFormats/HepMCCandidate/interface/GenParticleCandidateFwd.h"
//#include "DataFormats/HepMCCandidate/interface/GenParticleCandidate.h"
//#include "DataFormats/METReco/interface/GenMET.h"
//#include "DataFormats/METReco/interface/GenMETCollection.h"
//#include "DataFormats/METReco/interface/CaloMET.h"
//#include "DataFormats/METReco/interface/CaloMETCollection.h"
//#include "DataFormats/TrackReco/interface/Track.h"
//#include "DataFormats/ParticleFlowReco/interface/PFBlock.h"
//#include "DataFormats/ParticleFlowReco/interface/PFBlockElement.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidateFwd.h"
#include "DataFormats/ParticleFlowCandidate/interface/PFCandidate.h"
//#include "DataFormats/Common/interface/RefToBase.h"

#include <vector>
#include <ostream>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <memory>

using namespace reco;
using namespace edm;
using namespace std;

PFBenchmarkAnalyzer::PFBenchmarkAnalyzer(const edm::ParameterSet& iConfig)
{

  inputTruthLabel_             = iConfig.getParameter<std::string>("InputTruthLabel");
  inputRecoLabel_              = iConfig.getParameter<std::string>("InputRecoLabel");
  outputFile_                  = iConfig.getUntrackedParameter<std::string>("OutputFile");
  benchmarkLabel_              = iConfig.getParameter<std::string>("BenchmarkLabel"); 
  plotAgainstRecoQuantities_   = iConfig.getParameter<bool>("PlotAgainstRecoQuantities");

  if (outputFile_.size() > 0)
    edm::LogInfo("OutputInfo") << " ParticleFLow Task histograms will be saved to '" << outputFile_.c_str() << "'";
  else edm::LogInfo("OutputInfo") << " ParticleFlow Task histograms will NOT be saved";

}

PFBenchmarkAnalyzer::~PFBenchmarkAnalyzer() { }

void PFBenchmarkAnalyzer::beginJob(const edm::EventSetup& iSetup)
{

  // get ahold of back-end interface
  dbe_ = edm::Service<DaqMonitorBEInterface>().operator->();
  
  if (dbe_) {

    string path = "PFTask/Benchmarks/" + benchmarkLabel_ + "/";
    if (plotAgainstRecoQuantities_) path += "Reco"; else path += "Gen";
    dbe_->setCurrentFolder(path.c_str());
    setup(dbe_);

  }

}

void PFBenchmarkAnalyzer::analyze(const edm::Event& iEvent, const edm::EventSetup& iSetup)
{

  // data to retrieve from the event
  const CandidateCollection *truth_candidates;
  const CandidateCollection *reco_candidates;

  // ==========================================================
  // Retrieve!
  // ==========================================================

  { 

    // Get Truth Candidates (GenCandidates, GenJets, etc.)
    Handle<CandidateCollection> truth_hnd;
    iEvent.getByLabel(inputTruthLabel_, truth_hnd);
    truth_candidates = truth_hnd.product();

    // Get Reco Candidates (PFlow, CaloJet, etc.)
    Handle<PFCandidateCollection> reco_hnd;
    iEvent.getByLabel(inputRecoLabel_, reco_hnd);
    const PFCandidateCollection *pf_candidates = reco_hnd.product();
    reco_candidates = algo_->makeCandidateCollection(pf_candidates);

  }

  if (!truth_candidates || !reco_candidates) {

    edm::LogInfo("OutputInfo") << " failed to retrieve data required by ParticleFlow Task";
    edm::LogInfo("OutputInfo") << " ParticleFlow Task cannot continue...!";
    return;

  }

  // ==========================================================
  // Analyze!
  // ==========================================================

  fill(reco_candidates,truth_candidates,plotAgainstRecoQuantities_);

}

void PFBenchmarkAnalyzer::endJob() 
{

  // Store the DAQ Histograms
  if (outputFile_.size() != 0)
    dbe_->save(outputFile_);

}

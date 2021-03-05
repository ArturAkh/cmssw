#include "FWCore/Framework/interface/global/EDProducer.h"

#include "FWCore/Framework/interface/Event.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/MakerMacros.h"
#include "FWCore/ParameterSet/interface/ParameterSet.h"

#include "DataFormats/Common/interface/ContainerMask.h"
#include "DataFormats/Common/interface/DetSetVectorNew.h"
#include "DataFormats/SiPixelCluster/interface/SiPixelCluster.h"
#include "DataFormats/SiStripCluster/interface/SiStripClusterfwd.h"

#include "RecoTracker/MkFit/interface/MkFitHitWrapper.h"
#include "RecoTracker/MkFit/interface/MkFitSeedWrapper.h"
#include "RecoTracker/MkFit/interface/MkFitOutputWrapper.h"
#include "RecoTracker/MkFit/interface/MkFitGeometry.h"
#include "RecoTracker/Record/interface/TrackerRecoGeometryRecord.h"

// mkFit includes
#include "ConfigWrapper.h"
#include "LayerNumberConverter.h"
#include "mkFit/buildtestMPlex.h"
#include "mkFit/IterationConfig.h"
#include "mkFit/MkBuilderWrapper.h"

// TBB includes
#include "tbb/task_arena.h"

// std includes
#include <functional>

class MkFitProducer : public edm::global::EDProducer<edm::StreamCache<mkfit::MkBuilderWrapper> > {
public:
  explicit MkFitProducer(edm::ParameterSet const& iConfig);
  ~MkFitProducer() override = default;

  static void fillDescriptions(edm::ConfigurationDescriptions& descriptions);

  std::unique_ptr<mkfit::MkBuilderWrapper> beginStream(edm::StreamID) const override;

private:
  void produce(edm::StreamID, edm::Event& iEvent, const edm::EventSetup& iSetup) const override;

  const edm::EDGetTokenT<MkFitHitWrapper> hitToken_;
  const edm::EDGetTokenT<MkFitSeedWrapper> seedToken_;
  edm::EDGetTokenT<edm::ContainerMask<edmNew::DetSetVector<SiPixelCluster> > > pixelMaskToken_;
  edm::EDGetTokenT<edm::ContainerMask<edmNew::DetSetVector<SiStripCluster> > > stripMaskToken_;
  const edm::ESGetToken<MkFitGeometry, TrackerRecoGeometryRecord> mkFitGeomToken_;
  edm::EDPutTokenT<MkFitOutputWrapper> putToken_;
  std::function<double(mkfit::Event&, mkfit::MkBuilder&)> buildFunction_;
  const int iterationNumber_;  // TODO: temporary solution
  const bool seedCleaning_;
  bool backwardFitInCMSSW_;
  const bool removeDuplicates_;
  bool mkFitSilent_;
};

MkFitProducer::MkFitProducer(edm::ParameterSet const& iConfig)
    : hitToken_{consumes<MkFitHitWrapper>(iConfig.getParameter<edm::InputTag>("hits"))},
      seedToken_{consumes<MkFitSeedWrapper>(iConfig.getParameter<edm::InputTag>("seeds"))},
      mkFitGeomToken_{esConsumes<MkFitGeometry, TrackerRecoGeometryRecord>()},
      putToken_{produces<MkFitOutputWrapper>()},
      iterationNumber_{iConfig.getParameter<int>("iterationNumber")},
      seedCleaning_{iConfig.getParameter<bool>("seedCleaning")},
      backwardFitInCMSSW_{iConfig.getParameter<bool>("backwardFitInCMSSW")},
      removeDuplicates_{iConfig.getParameter<bool>("removeDuplicates")},
      mkFitSilent_{iConfig.getUntrackedParameter<bool>("mkFitSilent")} {
  const auto clustersToSkip = iConfig.getParameter<edm::InputTag>("clustersToSkip");
  if (not clustersToSkip.label().empty()) {
    pixelMaskToken_ = consumes(clustersToSkip);
    stripMaskToken_ = consumes(clustersToSkip);
  }

  const auto build = iConfig.getParameter<std::string>("buildingRoutine");
  if (build == "bestHit") {
    //buildFunction_ = mkfit::runBuildingTestPlexBestHit;
    throw cms::Exception("Configuration") << "bestHit is temporarily disabled";
  } else if (build == "standard") {
    //buildFunction_ = mkfit::runBuildingTestPlexStandard;
    throw cms::Exception("Configuration") << "standard is temporarily disabled";
  } else if (build == "cloneEngine") {
    //buildFunction_ = mkfit::runBuildingTestPlexCloneEngine;
  } else {
    throw cms::Exception("Configuration")
        << "Invalid value for parameter 'buildingRoutine' " << build << ", allowed are bestHit, standard, cloneEngine";
  }

  // TODO: what to do when we have multiple instances of MkFitProducer in a job?
  mkfit::MkBuilderWrapper::populate();
  mkfit::ConfigWrapper::initializeForCMSSW(mkFitSilent_);
}

void MkFitProducer::fillDescriptions(edm::ConfigurationDescriptions& descriptions) {
  edm::ParameterSetDescription desc;

  desc.add("hits", edm::InputTag("mkFitHits"));
  desc.add("seeds", edm::InputTag("mkFitSeedConverter"));
  desc.add("clustersToSkip", edm::InputTag());
  desc.add<std::string>("buildingRoutine", "cloneEngine")
      ->setComment("Valid values are: 'bestHit', 'standard', 'cloneEngine'");
  desc.add<int>("iterationNumber", 0)->setComment("Iteration number (default: 0)");
  desc.add("seedCleaning", true)->setComment("Clean seeds within mkFit");
  desc.add("removeDuplicates", true)->setComment("Run duplicate removal within mkFit");
  desc.add("backwardFitInCMSSW", false)
      ->setComment("Do backward fit (to innermost hit) in CMSSW (true) or mkFit (false)");
  desc.addUntracked("mkFitSilent", true)->setComment("Allows to enables printouts from mkFit with 'False'");

  descriptions.add("mkFitProducer", desc);
}

std::unique_ptr<mkfit::MkBuilderWrapper> MkFitProducer::beginStream(edm::StreamID iID) const {
  return std::make_unique<mkfit::MkBuilderWrapper>();
}

namespace {
  std::once_flag geometryFlag;
}
void MkFitProducer::produce(edm::StreamID iID, edm::Event& iEvent, const edm::EventSetup& iSetup) const {
  const auto& hits = iEvent.get(hitToken_);
  const auto& seeds = iEvent.get(seedToken_);
  // This producer does not strictly speaking need the MkFitGeometry,
  // but the ESProducer sets global variables (yes, that "feature"
  // should be removed), so getting the MkFitGeometry makes it
  // sure that the ESProducer is called even if the input/output
  // converters
  const auto& mkFitGeom = iSetup.getData(mkFitGeomToken_);

  const std::vector<bool>* pixelMaskPtr = nullptr;
  const std::vector<bool>* stripMaskPtr = nullptr;
  std::vector<bool> pixelMask;
  std::vector<bool> stripMask;
  if (not pixelMaskToken_.isUninitialized()) {
    const auto& pixelContainerMask = iEvent.get(pixelMaskToken_);
    pixelMask.resize(pixelContainerMask.size(), false);
    pixelContainerMask.copyMaskTo(pixelMask);
    pixelMaskPtr = &pixelMask;

    const auto& stripContainerMask = iEvent.get(stripMaskToken_);
    stripMask.resize(stripContainerMask.size(), false);
    stripContainerMask.copyMaskTo(stripMask);
    stripMaskPtr = &stripMask;
  }
  // TODO: add strip cluster charge cut

  // Initialize the number of layers, has to be done exactly once in
  // the whole program.
  // TODO: the mechanism needs to be improved...
  std::call_once(geometryFlag, [nlayers = mkFitGeom.layerNumberConverter().nLayers()]() {
    mkfit::ConfigWrapper::setNTotalLayers(nlayers);
  });

  // seeds need to be mutable because of the possible cleaning
  auto seeds_mutable = seeds.seeds();
  mkfit::TrackVec tracks;

  tbb::this_task_arena::isolate([&]() {
    mkfit::run_OneIteration(mkFitGeom.trackerInfo(),
                            mkFitGeom.iterationsInfo()[iterationNumber_],
                            hits.eventOfHits(),
                            {pixelMaskPtr, stripMaskPtr},
                            streamCache(iID)->get(),
                            seeds_mutable,
                            tracks,
                            seedCleaning_,
                            not backwardFitInCMSSW_,
                            removeDuplicates_);
  });

  iEvent.emplace(putToken_, std::move(tracks), not backwardFitInCMSSW_);
}

DEFINE_FWK_MODULE(MkFitProducer);

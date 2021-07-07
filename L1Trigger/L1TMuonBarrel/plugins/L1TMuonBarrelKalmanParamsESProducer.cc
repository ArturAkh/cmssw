// -*- C++ -*-
//
// Class:      L1TMuonBarrelKalmanParamsESProducer
//
// Original Author (of the base file):  Giannis Flouris
// Kalman Mod & clean up:  Panos Katsoulis
//         Created:
//
//

// system include files
#include <memory>

// user include files
#include "FWCore/Framework/interface/ModuleFactory.h"
#include "FWCore/Framework/interface/ESProducer.h"
#include "FWCore/Framework/interface/ESHandle.h"
#include "FWCore/Framework/interface/ESProducts.h"

#include "CondFormats/L1TObjects/interface/L1TMuonBarrelKalmanParams.h" //changed
#include "CondFormats/DataRecord/interface/L1TMuonBarrelParamsRcd.h" //changed
#include "L1Trigger/L1TMuon/interface/MicroGMTLUTFactories.h"

#include "FWCore/ParameterSet/interface/FileInPath.h"

#include "L1Trigger/L1TCommon/interface/XmlConfigParser.h"
#include "L1Trigger/L1TCommon/interface/TriggerSystem.h"
#include "L1Trigger/L1TCommon/interface/Parameter.h"
#include "L1Trigger/L1TCommon/interface/Mask.h"

#include "L1Trigger/L1TMuonBarrel/interface/L1TMuonBarrelParamsHelper.h" //to check

// class declaration
//
// typedef std::map<short, short, std::less<short> > LUT;

class L1TMuonBarrelKalmanParamsESProducer : public edm::ESProducer {
public:
  L1TMuonBarrelKalmanParamsESProducer(const edm::ParameterSet&);
  ~L1TMuonBarrelKalmanParamsESProducer() override;

  using ReturnType = std::unique_ptr<L1TMuonBarrelKalmanParams>;
  ReturnType produce(const L1TMuonBarrelKalmanParamsRcd&);

private:
  L1TMuonBarrelKalmanParams kalman_params;
};

//
// constants, enums and typedefs
//

//
// static data member definitions
//

//
// constructors and destructor
//
L1TMuonBarrelKalmanParamsESProducer::L1TMuonBarrelKalmanParamsESProducer(const edm::ParameterSet& iConfig) {
  // the following line is needed to tell the framework what data is being produced
  setWhatProduced(this); //???

  // basic configurables needed (now set static)
  kalman_params.pnodes_[kalman_params.CONFIG].fwVersion_ = iConfig.getParameter<unsigned>("fwVersion");

  // the LUTs
  kalman_params.pnodes_[kalman_params.CONFIG].kalmanLUTsPath_ = iConfig.getParameter<std::string>("LUTsPath");
}

L1TMuonBarrelKalmanParamsESProducer::~L1TMuonBarrelKalmanParamsESProducer() {}

//
// member functions
//

// ------------ method called to produce the data  ------------
L1TMuonBarrelKalmanParamsESProducer::ReturnType L1TMuonBarrelKalmanParamsESProducer::produce(const L1TMuonBarrelKalmanParamsRcd& iRecord) {
  return std::make_unique<L1TMuonBarrelKalmanParams>(kalman_params);
}

//define this as a plug-in
DEFINE_FWK_EVENTSETUP_MODULE(L1TMuonBarrelKalmanParamsESProducer);

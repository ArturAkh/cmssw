#ifndef DataFormats_PortableTestObjects_interface_alpaka_HGCalSoAClustersExtraDeviceCollection_h
#define DataFormats_PortableTestObjects_interface_alpaka_HGCalSoAClustersExtraDeviceCollection_h

#include "DataFormats/Portable/interface/alpaka/PortableCollection.h"
#include "RecoLocalCalo/HGCalRecProducers/interface/HGCalSoAClustersExtra.h"
#include "HeterogeneousCore/AlpakaInterface/interface/config.h"

namespace ALPAKA_ACCELERATOR_NAMESPACE {

  using HGCalSoAClustersExtraDeviceCollection = PortableCollection<HGCalSoAClustersExtra>;

}  // namespace ALPAKA_ACCELERATOR_NAMESPACE

#endif  // DataFormats_PortableTestObjects_interface_alpaka_HGCalSoAClustersExtraDeviceCollection_h

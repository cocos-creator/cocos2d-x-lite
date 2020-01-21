#include "CoreStd.h"
#include "GFXSampler.h"

NS_CC_BEGIN

GFXSampler::GFXSampler(GFXDevice* device)
    : _device(device),
      min_filter_(GFXFilter::NONE),
      mag_filter_(GFXFilter::NONE),
      mip_filter_(GFXFilter::NONE),
      address_u_(GFXAddress::WRAP),
      address_v_(GFXAddress::WRAP),
      address_w_(GFXAddress::WRAP),
      max_anisotropy_(0),
      cmp_func_(GFXComparisonFunc::ALWAYS),
      min_lod_(0),
      max_lod_(0),
      mip_lod_bias_(0.0f) {
    border_color_.r = 0.0f;
    border_color_.g = 0.0f;
    border_color_.b = 0.0f;
    border_color_.a = 0.0f;
}

GFXSampler::~GFXSampler() {
}

NS_CC_END

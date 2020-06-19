#include "CoreStd.h"
#include "GFXSampler.h"

namespace cc {
namespace gfx {

GFXSampler::GFXSampler(Device *device)
: GFXObject(GFXObjectType::SAMPLER), _device(device) {
    _borderColor.r = 0.0f;
    _borderColor.g = 0.0f;
    _borderColor.b = 0.0f;
    _borderColor.a = 0.0f;
}

GFXSampler::~GFXSampler() {
}

} // namespace gfx
} // namespace cc

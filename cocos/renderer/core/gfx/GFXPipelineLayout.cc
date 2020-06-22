#include "CoreStd.h"
#include "GFXPipelineLayout.h"

namespace cc {
namespace gfx {

PipelineLayout::PipelineLayout(Device *device)
: GFXObject(GFXObjectType::PIPELINE_LAYOUT), _device(device) {
}

PipelineLayout::~PipelineLayout() {
}

} // namespace gfx
} // namespace cc

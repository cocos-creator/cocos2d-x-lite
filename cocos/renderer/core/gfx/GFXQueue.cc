#include "CoreStd.h"
#include "GFXQueue.h"

namespace cc {
namespace gfx {

GFXQueue::GFXQueue(Device *device)
: GFXObject(GFXObjectType::QUEUE), _device(device) {
}

GFXQueue::~GFXQueue() {
}

} // namespace gfx
} // namespace cc

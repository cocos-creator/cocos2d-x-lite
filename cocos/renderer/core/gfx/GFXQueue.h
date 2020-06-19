#ifndef CC_CORE_GFX_QUEUE_H_
#define CC_CORE_GFX_QUEUE_H_

#include "GFXDef.h"

namespace cc {
namespace gfx {

class CC_DLL GFXQueue : public GFXObject {
public:
    GFXQueue(Device *device);
    virtual ~GFXQueue();

public:
    virtual bool initialize(const GFXQueueInfo &info) = 0;
    virtual void destroy() = 0;
    virtual void submit(const vector<CommandBuffer *> &cmdBuffs, GFXFence *fence) = 0;

    CC_INLINE void submit(const vector<CommandBuffer *> &cmdBuffs) { submit(cmdBuffs, nullptr); }
    CC_INLINE Device *getDevice() const { return _device; }
    CC_INLINE GFXQueueType getType() const { return _type; }
    CC_INLINE bool isAsync() const { return _isAsync; }

protected:
    Device *_device = nullptr;
    GFXQueueType _type = GFXQueueType::GRAPHICS;
    bool _isAsync = false;
};

} // namespace gfx
} // namespace cc

#endif // CC_CORE_GFX_QUEUE_H_

#pragma once

#include "gfx/GFXCommandPool.h"
#include "MTLCommands.h"

namespace cc {
namespace gfx {

class CCMTLCommandPackage;

class CCMTLCommandAllocator : public GFXCommandAllocator {
public:
    CCMTLCommandAllocator(Device *device);
    ~CCMTLCommandAllocator();

    virtual bool initialize(const GFXCommandAllocatorInfo &info) override;
    virtual void destroy() override;

    void clearCommands(CCMTLCommandPackage *commandPackage);

    CC_INLINE void releaseCmds() {
        _beginRenderPassCmdPool.release();
        _bindStatesCmdPool.release();
        _drawCmdPool.release();
        _updateBufferCmdPool.release();
        _copyBufferToTextureCmdPool.release();
    }

private:
    friend class CCMTLCommandBuffer;
    GFXCommandPool<CCMTLCmdBeginRenderPass> _beginRenderPassCmdPool;
    GFXCommandPool<CCMTLCmdBindStates> _bindStatesCmdPool;
    GFXCommandPool<CCMTLCmdDraw> _drawCmdPool;
    GFXCommandPool<CCMTLCmdUpdateBuffer> _updateBufferCmdPool;
    GFXCommandPool<CCMTLCmdCopyBufferToTexture> _copyBufferToTextureCmdPool;
};

} // namespace gfx
} // namespace cc

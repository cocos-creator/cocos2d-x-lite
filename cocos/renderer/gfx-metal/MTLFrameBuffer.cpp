#include "MTLStd.h"
#include "MTLFrameBuffer.h"

NS_CC_BEGIN

CCMTLFrameBuffer::CCMTLFrameBuffer(GFXDevice* device) : GFXFramebuffer(device) {}
CCMTLFrameBuffer::~CCMTLFrameBuffer() { destroy(); }

bool CCMTLFrameBuffer::initialize(const GFXFramebufferInfo& info)
{
    _renderPass = info.render_pass;
    color_views_ = info.color_views;
    depth_stencil_view_ = info.depth_stencil_view;
    _isOffscreen = info.is_offscreen;
    
    return true;
}

void CCMTLFrameBuffer::destroy()
{
    
}

NS_CC_END

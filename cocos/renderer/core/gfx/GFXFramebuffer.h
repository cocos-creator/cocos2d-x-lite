#ifndef CC_CORE_GFX_FRAME_BUFFER_H_
#define CC_CORE_GFX_FRAME_BUFFER_H_

#include "GFXDef.h"

NS_CC_BEGIN

class CC_CORE_API GFXFramebuffer : public Object {
 public:
  GFXFramebuffer(GFXDevice* device);
  virtual ~GFXFramebuffer();
  
 public:
  virtual bool initialize(const GFXFramebufferInfo& info) = 0;
  virtual void destroy() = 0;
  
  CC_INLINE GFXDevice* device() const { return _device; }
  CC_INLINE GFXRenderPass* render_pass() const { return render_pass_; }
  CC_INLINE const GFXTextureViewList& color_views() const { return color_views_; }
  CC_INLINE GFXTextureView* depth_stencil_view() const { return depth_stencil_view_; }
  CC_INLINE bool is_offscreen() const { return is_offscreen_; }
  
 protected:
  GFXDevice* _device;
  GFXRenderPass* render_pass_;
  GFXTextureViewList color_views_;
  GFXTextureView* depth_stencil_view_;
  bool is_offscreen_;
};

NS_CC_END

#endif // CC_CORE_GFX_FRAME_BUFFER_H_

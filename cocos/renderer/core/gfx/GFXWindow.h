#ifndef CC_CORE_GFX_WINDOW_H_
#define CC_CORE_GFX_WINDOW_H_

#include "GFXDef.h"

NS_CC_BEGIN

class CC_CORE_API GFXWindow : public Object {
 public:
  GFXWindow(GFXDevice* device);
  virtual ~GFXWindow();
  
 public:
  virtual bool initialize(const GFXWindowInfo& info) = 0;
  virtual void destroy() = 0;
  virtual void resize(uint width, uint height) = 0;
  
  CC_INLINE GFXDevice* device() const { return _device; }
  CC_INLINE const String& title() const { return _title; }
  CC_INLINE int left() const { return _left; }
  CC_INLINE int top() const { return _top; }
  CC_INLINE uint width() const { return _width; }
  CC_INLINE uint height() const { return _height; }
  CC_INLINE uint native_width() const { return _nativeWidth; }
  CC_INLINE uint native_height() const { return _nativeHeight; }
  CC_INLINE GFXFormat color_fmt() const { return color_fmt_; }
  CC_INLINE GFXFormat depth_stencil_fmt() const { return depth_stencil_fmt_; }
  CC_INLINE bool is_offscreen() const { return is_offscreen_; }
  CC_INLINE GFXRenderPass* render_pass() const { return render_pass_; }
  CC_INLINE GFXTexture* color_texture() const { return color_texture_; }
  CC_INLINE GFXTextureView* color_tex_view() const { return color_tex_view_; }
  CC_INLINE GFXTexture* depth_stencil_texture() const { return depth_stencil_texture_; }
  CC_INLINE GFXTextureView* depth_stencil_tex_view() const { return depth_stencil_tex_view_; }
  CC_INLINE GFXFramebuffer* framebuffer() const { return framebuffer_; }
  
 protected:
  GFXDevice* _device;
  String _title;
  int _left;
  int _top;
  uint _width;
  uint _height;
  uint _nativeWidth;
  uint _nativeHeight;
  GFXFormat color_fmt_;
  GFXFormat depth_stencil_fmt_;
  bool is_offscreen_;
  bool is_fullscreen_;
  GFXRenderPass* render_pass_;
  GFXTexture* color_texture_;
  GFXTextureView* color_tex_view_;
  GFXTexture* depth_stencil_texture_;
  GFXTextureView* depth_stencil_tex_view_;
  GFXFramebuffer* framebuffer_;
};

NS_CC_END

#endif // CC_CORE_GFX_TEXTURE_VIEW_H_

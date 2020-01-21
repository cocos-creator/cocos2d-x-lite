#include "GLES3Std.h"
#include "GLES3TextureView.h"
#include "GLES3Texture.h"
#include "GLES3Commands.h"

NS_CC_BEGIN

GLES3TextureView::GLES3TextureView(GFXDevice* device)
    : GFXTextureView(device),
      gpu_tex_view_(nullptr) {
}

GLES3TextureView::~GLES3TextureView() {
}

bool GLES3TextureView::initialize(const GFXTextureViewInfo &info) {
  
  texture_ = info.texture;
  _type = info.type;
  format_ = info.format;
  base_level_ = info.base_level;
  level_count_ = info.level_count;
  base_layer_ = info.base_layer;
  layer_count_ = info.layer_count;
  
  gpu_tex_view_ = CC_NEW(GLES3GPUTextureView);
  gpu_tex_view_->gpu_texture = static_cast<GLES3Texture*>(texture_)->gpu_texture();
  gpu_tex_view_->type = _type;
  gpu_tex_view_->format = format_;
  gpu_tex_view_->base_level = info.base_level;
  gpu_tex_view_->level_count = info.level_count;
  
  return true;
}

void GLES3TextureView::destroy() {
  if (gpu_tex_view_) {
    CC_DELETE(gpu_tex_view_);
    gpu_tex_view_ = nullptr;
  }
  texture_ = nullptr;
}

NS_CC_END

#include "GLES3Std.h"
#include "GLES3Texture.h"
#include "GLES3Commands.h"

NS_CC_BEGIN

GLES3Texture::GLES3Texture(GFXDevice* device)
    : GFXTexture(device),
      gpu_texture_(nullptr) {
}

GLES3Texture::~GLES3Texture() {
}

bool GLES3Texture::initialize(const GFXTextureInfo &info) {
  
  type_ = info.type;
  usage_ = info.usage;
  format_ = info.format;
  _width = info.width;
  height_ = info.height;
  depth_ = info.depth;
  array_layer_ = info.array_layer;
  mip_level_ = info.mip_level;
  samples_ = info.samples;
  flags_ = info.flags;
  size_ = GFXFormatSize(format_, _width, height_, depth_);
  
  if (flags_ & GFXTextureFlags::BAKUP_BUFFER) {
    buffer_ = (uint8_t*)CC_MALLOC(size_);
    _device->memoryStatus().texture_size += size_;
  }
  
  gpu_texture_ = CC_NEW(GLES3GPUTexture);
  gpu_texture_->type = type_;
  
  switch (type_) {
    case GFXTextureType::TEX1D: {
      if (array_layer_) {
        gpu_texture_->view_type = array_layer_ <= 1 ? GFXTextureViewType::TV1D : GFXTextureViewType::TV1D_ARRAY;
      } else {
        gpu_texture_->view_type = GFXTextureViewType::TV1D;
      }
      break;
    }
    case GFXTextureType::TEX2D: {
      if (array_layer_) {
        if (array_layer_ <= 1) {
          gpu_texture_->view_type = GFXTextureViewType::TV2D;
        } else if (flags_ & GFXTextureFlagBit::CUBEMAP) {
          gpu_texture_->view_type = GFXTextureViewType::CUBE;
        } else {
          gpu_texture_->view_type = GFXTextureViewType::TV2D_ARRAY;
        }
      } else {
        gpu_texture_->view_type = GFXTextureViewType::TV2D;
      }
      break;
    }
    case GFXTextureType::TEX3D: {
      gpu_texture_->view_type = GFXTextureViewType::TV3D;
      break;
    }
    default: {
      gpu_texture_->view_type = GFXTextureViewType::TV2D;
    }
  }
  
  gpu_texture_->format = format_;
  gpu_texture_->usage = usage_;
  gpu_texture_->width = _width;
  gpu_texture_->height = height_;
  gpu_texture_->depth = depth_;
  gpu_texture_->size = size_;
  gpu_texture_->array_layer = array_layer_;
  gpu_texture_->mip_level = mip_level_;
  gpu_texture_->samples = samples_;
  gpu_texture_->flags = flags_;
    gpu_texture_->is_pot = math::IsPowerOfTwo(_width) && math::IsPowerOfTwo(height_);
  
  GLES3CmdFuncCreateTexture((GLES3Device*)_device, gpu_texture_);
  _device->memoryStatus().texture_size += size_;
  
  return true;
}

void GLES3Texture::destroy() {
  if (gpu_texture_) {
    GLES3CmdFuncDestroyTexture((GLES3Device*)_device, gpu_texture_);
    _device->memoryStatus().texture_size -= size_;
    CC_DELETE(gpu_texture_);
    gpu_texture_ = nullptr;
  }
  
  if (buffer_) {
    CC_FREE(buffer_);
    _device->memoryStatus().texture_size -= size_;
    buffer_ = nullptr;
  }
}

void GLES3Texture::resize(uint width, uint height) {
  uint size = GFXFormatSize(format_, width, height, depth_);
  if (size_ != size) {
    const uint old_size = size_;
    _width = width;
    height_ = height;
    size_ = size;
    
    GFXMemoryStatus& status = _device->memoryStatus();
    gpu_texture_->width = _width;
    gpu_texture_->height = height_;
    gpu_texture_->size = size_;
    GLES3CmdFuncResizeTexture((GLES3Device*)_device, gpu_texture_);
    status.buffer_size -= old_size;
    status.buffer_size += size_;
    
    if (buffer_) {
      const uint8_t* old_buff = buffer_;
      buffer_ = (uint8_t*)CC_MALLOC(size_);
      memcpy(buffer_, old_buff, old_size);
      CC_FREE(buffer_);
      status.buffer_size -= old_size;
      status.buffer_size += size_;
    }
  }
}

NS_CC_END

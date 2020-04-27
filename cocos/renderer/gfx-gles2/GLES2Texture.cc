#include "GLES2Std.h"
#include "GLES2Texture.h"
#include "GLES2Commands.h"

NS_CC_BEGIN

GLES2Texture::GLES2Texture(GFXDevice* device)
    : GFXTexture(device) {
}

GLES2Texture::~GLES2Texture() {
}

bool GLES2Texture::initialize(const GFXTextureInfo &info) {
  
  _type = info.type;
  _usage = info.usage;
  _format = info.format;
  _width = info.width;
  _height = info.height;
  _depth = info.depth;
  _arrayLayer = info.arrayLayer;
  _mipLevel = info.mipLevel;
  _samples = info.samples;
  _flags = info.flags;
  _size = GFXFormatSize(_format, _width, _height, _depth);
  
  if (_flags & GFXTextureFlags::BAKUP_BUFFER) {
    _buffer = (uint8_t*)CC_MALLOC(_size);
    if(!_buffer)
    {
      _status = GFXStatus::FAILED;
      CC_LOG_ERROR("GLES2Texture: CC_MALLOC backup buffer failed.");
      return false;
    }
    _device->getMemoryStatus().textureSize += _size;
  }
  
  _gpuTexture = CC_NEW(GLES2GPUTexture);
  if(!_gpuTexture)
  {
    _status = GFXStatus::FAILED;
    CC_LOG_ERROR("GLES2Texture: CC_NEW GLES2GPUTexture failed.");
    return false;
  }
  _gpuTexture->type = _type;
  
  switch (_type) {
    case GFXTextureType::TEX1D: {
      if (_arrayLayer) {
        _gpuTexture->viewType = _arrayLayer <= 1 ? GFXTextureViewType::TV1D : GFXTextureViewType::TV1D_ARRAY;
      } else {
        _gpuTexture->viewType = GFXTextureViewType::TV1D;
      }
      break;
    }
    case GFXTextureType::TEX2D: {
      if (_arrayLayer) {
        if (_arrayLayer <= 1) {
          _gpuTexture->viewType = GFXTextureViewType::TV2D;
        } else if (_flags & GFXTextureFlagBit::CUBEMAP) {
          _gpuTexture->viewType = GFXTextureViewType::CUBE;
        } else {
          _gpuTexture->viewType = GFXTextureViewType::TV2D_ARRAY;
        }
      } else {
        _gpuTexture->viewType = GFXTextureViewType::TV2D;
      }
      break;
    }
    case GFXTextureType::TEX3D: {
      _gpuTexture->viewType = GFXTextureViewType::TV3D;
      break;
    }
    default: {
      _gpuTexture->viewType = GFXTextureViewType::TV2D;
    }
  }
  
  _gpuTexture->format = _format;
  _gpuTexture->usage = _usage;
  _gpuTexture->width = _width;
  _gpuTexture->height = _height;
  _gpuTexture->depth = _depth;
  _gpuTexture->size = _size;
  _gpuTexture->arrayLayer = _arrayLayer;
  _gpuTexture->mipLevel = _mipLevel;
  _gpuTexture->samples = _samples;
  _gpuTexture->flags = _flags;
  _gpuTexture->isPowerOf2 = math::IsPowerOfTwo(_width) && math::IsPowerOfTwo(_height);
  
  GLES2CmdFuncCreateTexture((GLES2Device*)_device, _gpuTexture);
  _device->getMemoryStatus().textureSize += _size;  
  _status = GFXStatus::SUCCESS;
  return true;
}

void GLES2Texture::destroy() {
  if (_gpuTexture) {
    GLES2CmdFuncDestroyTexture((GLES2Device*)_device, _gpuTexture);
    _device->getMemoryStatus().textureSize -= _size;
    CC_DELETE(_gpuTexture);
    _gpuTexture = nullptr;
  }
  
  if (_buffer) {
    CC_FREE(_buffer);
    _device->getMemoryStatus().textureSize -= _size;
    _buffer = nullptr;
  }
    
    _status = GFXStatus::UNREADY;
}

void GLES2Texture::resize(uint width, uint height) {
  uint size = GFXFormatSize(_format, width, height, _depth);
  if (_size != size) {
    const uint oldSize = _size;
    _width = width;
    _height = height;
    _size = size;
    
    GFXMemoryStatus& status = _device->getMemoryStatus();
    _gpuTexture->width = _width;
    _gpuTexture->height = _height;
    _gpuTexture->size = _size;
    GLES2CmdFuncResizeTexture((GLES2Device*)_device, _gpuTexture);
    status.bufferSize -= oldSize;
    status.bufferSize += _size;
    
    if (_buffer) {
      const uint8_t* oldBuffer = _buffer;
      uint8_t* buffer = (uint8_t*)CC_MALLOC(_size);
      if(!buffer)
      {
        _status = GFXStatus::FAILED;
        CC_LOG_ERROR("GLES2Texture: CC_MALLOC backup buffer failed when resize the texture.");
        return;
      }
      memcpy(buffer, oldBuffer, std::min(oldSize, size));
      _buffer = buffer;
      CC_FREE(oldBuffer);
      status.bufferSize -= oldSize;
      status.bufferSize += _size;
    }
    _status = GFXStatus::SUCCESS;
  }
}

NS_CC_END

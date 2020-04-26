#include "GLES3Std.h"
#include "GLES3Buffer.h"
#include "GLES3Commands.h"

NS_CC_BEGIN

GLES3Buffer::GLES3Buffer(GFXDevice* device)
    : GFXBuffer(device) {
}

GLES3Buffer::~GLES3Buffer() {
}

bool GLES3Buffer::initialize(const GFXBufferInfo& info) {
  
  _usage = info.usage;
  _memUsage = info.memUsage;
  _size = info.size;
  _stride = std::max(info.stride, 1U);
  _count = _size / _stride;
  _flags = info.flags;
  
  if ((_flags & GFXBufferFlagBit::BAKUP_BUFFER) && _size > 0) {
    _buffer = (uint8_t*)CC_MALLOC(_size);
    if(!_buffer)
    {
      _status = GFXStatus::FAILED;
      CC_LOG_ERROR("GLES3Buffer: CC_MALLOC backup buffer failed.");
      return false;
    }
    _device->getMemoryStatus().bufferSize += _size;
  }
  
  _gpuBuffer = CC_NEW(GLES3GPUBuffer);
  if(!_gpuBuffer)
  {
    _status = GFXStatus::FAILED;
    CC_LOG_ERROR("GLES3Buffer: CC_NEW GLES3GPUBuffer failed.");
    return false;
  }
  _gpuBuffer->usage = _usage;
  _gpuBuffer->memUsage = _memUsage;
  _gpuBuffer->size = _size;
  _gpuBuffer->stride = _stride;
  _gpuBuffer->count = _count;
  
  if (_usage & GFXBufferUsageBit::INDIRECT) 
  {
    _gpuBuffer->indirects.resize(_count);
  }
  else 
  {
    _gpuBuffer->buffer = _buffer;
  }
  
  GLES3CmdFuncCreateBuffer((GLES3Device*)_device, _gpuBuffer);
  _device->getMemoryStatus().bufferSize += _size;
  _status = GFXStatus::SUCCESS;
  return true;
}

void GLES3Buffer::destroy() {
  if (_gpuBuffer) {
    GLES3CmdFuncDestroyBuffer((GLES3Device*)_device, _gpuBuffer);
    _device->getMemoryStatus().bufferSize -= _size;
    CC_DELETE(_gpuBuffer);
    _gpuBuffer = nullptr;
  }
  
  if (_buffer) {
    CC_FREE(_buffer);
    _device->getMemoryStatus().bufferSize -= _size;
    _buffer = nullptr;
  }
  _status = GFXStatus::UNREADY;
}

void GLES3Buffer::resize(uint size) {
  if (_size != size) {
    const uint oldSize = _size;
    _size = size;
    _count = _size / _stride;
    
    GFXMemoryStatus& status = _device->getMemoryStatus();
    _gpuBuffer->size = _size;
    _gpuBuffer->count = _count;
    GLES3CmdFuncResizeBuffer((GLES3Device*)_device, _gpuBuffer);
    status.bufferSize -= oldSize;
    status.bufferSize += _size;

    if (_buffer) {
      const uint8_t* oldBuffer = _buffer;
      uint8_t* buffer = (uint8_t*)CC_MALLOC(_size);
      if(!buffer)
      {
        _status = GFXStatus::FAILED;
        CC_LOG_ERROR("GLES3Buffer: CC_MALLOC resize backup buffer failed.");
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

void GLES3Buffer::update(void* buffer, uint offset, uint size) {
  if (_buffer) {
    memcpy(_buffer + offset, buffer, size);
  }
  GLES3CmdFuncUpdateBuffer((GLES3Device*)_device, _gpuBuffer, buffer, offset, size);
}

NS_CC_END

#include "MTLStd.h"
#include "MTLBuffer.h"
#include "MTLDevice.h"
#import <Metal/Metal.h>

NS_CC_BEGIN

namespace
{
    //FIXME: not sure if it is correct to map like this
    MTLResourceOptions toMTLResourseOption(GFXMemoryUsage usage)
    {
        if (usage == GFXMemoryUsage::DEVICE)
            return MTLResourceStorageModePrivate;
        else
            return MTLResourceStorageModeShared;
    }
}

CCMTLBuffer::CCMTLBuffer(GFXDevice* device) : GFXBuffer(device) {}

CCMTLBuffer::~CCMTLBuffer()
{
    destroy();
}

bool CCMTLBuffer::initialize(const GFXBufferInfo& info)
{
    _usage = info.usage;
    _memUsage = info.memUsage;
    _size = info.size;
    _stride = std::max(info.stride, 1U);
    _count = _size / _stride;
    _flags = info.flags;
    
    if (_usage & GFXBufferUsage::INDEX)
    {
        switch (_stride) {
            case 4: _indexType = MTLIndexTypeUInt32; break;
            case 2: _indexType = MTLIndexTypeUInt16; break;
            default:
                CC_LOG_ERROR("Illegal index buffer stride.");
                break;
        }
    }
    
    if ((_flags & GFXBufferFlagBit::BAKUP_BUFFER) && _size > 0)
    {
        _buffer = (uint8_t*)CC_MALLOC(_size);
        if (_buffer)
            _device->getMemoryStatus().bufferSize += _size;
        else
        {
            _status = GFXStatus::FAILED;
            CCASSERT(false, "CCMTLBuffer: Failed to create backup buffer.");
            return false;
        }
    }
    
    if (_usage & GFXBufferUsage::VERTEX ||
        _usage & GFXBufferUsage::INDEX ||
        _usage & GFXBufferUsage::UNIFORM)
    {
        _mtlResourceOptions = toMTLResourseOption(info.memUsage);
        if (_size > 0)
        {
            _mtlBuffer = [id<MTLDevice>(((CCMTLDevice*)_device)->getMTLDevice() ) newBufferWithLength:_size
                                                                                              options:_mtlResourceOptions];
            if (_mtlBuffer == nil)
            {
                _status = GFXStatus::FAILED;
                CCASSERT(false, "CCMTLBuffer: Failed to create MTLBuffer.");
                return false;
            }
        }
    }
    else if (_usage & GFXBufferUsageBit::INDIRECT)
    {
        _indirects.resize(_count);
    }
    else if (_usage & GFXBufferUsageBit::TRANSFER_SRC ||
             _usage & GFXBufferUsageBit::TRANSFER_DST)
    {
        _transferBuffer = (uint8_t*)CC_MALLOC(_size);
        if (!_transferBuffer)
        {
            _status = GFXStatus::FAILED;
            CCASSERT(false, "CCMTLBuffer: failed to create memory for transfer buffer.");
            return false;
        }
        _device->getMemoryStatus().bufferSize += _size;
    }
    else
    {
        _status = GFXStatus::FAILED;
        CCASSERT(false, "Unsupported GFXBufferType, create buffer failed.");
        return false;
    }
    
    _status = GFXStatus::SUCCESS;
    return true;
}

void CCMTLBuffer::destroy()
{
    if (_mtlBuffer)
    {
        [_mtlBuffer release];
        _mtlBuffer = nil;
    }
    
    if (_transferBuffer)
    {
        CC_FREE(_transferBuffer);
        _transferBuffer = nullptr;
        _device->getMemoryStatus().bufferSize -= _size;
    }
    
    if (_buffer)
    {
        CC_FREE(_buffer);
        _device->getMemoryStatus().bufferSize -= _size;
        _buffer = nullptr;
    }
    _status = GFXStatus::UNREADY;
}

void CCMTLBuffer::resize(uint size)
{
    if (size <= _size)
        return;
    
    const uint oldSize = _size;
    const uint count = size / _stride;
    uint8_t* tempBuffer = nullptr;
    
    //store buffer contents in case of backup buffer malloc faild.
    tempBuffer = static_cast<uint8_t*>(CC_MALLOC(oldSize));
    if(!tempBuffer)
    {
        _status = GFXStatus::FAILED;
        CCASSERT(false, "CCMTLBuffer: malloc temp buffer failed.");
        return;
    }
    
    auto succeed = resizeBuffer(size, count, oldSize, tempBuffer, true);
    if(!succeed)
    {
        _status = GFXStatus::FAILED;
        CC_SAFE_FREE(tempBuffer);
        CCASSERT(false, "CCMTLBuffer: resize buffer failed.");
        return;
    }
    
    if(_flags & GFXBufferFlagBit::BAKUP_BUFFER)
    {
        GFXMemoryStatus& status = _device->getMemoryStatus();
        uint8_t* buffer = static_cast<uint8_t*>(CC_MALLOC(size));
        if(!buffer)
        {
            _status = GFXStatus::FAILED;
            //recovery the reiszed buffer to keey it the same size as backup buffer.
            const uint oldCount = oldSize / _stride;
            resizeBuffer(oldSize, oldCount, size, tempBuffer, false);
            CC_SAFE_FREE(tempBuffer);
            CCASSERT(false, "CCMTLBuffer: malloc backup buffer failed.");
            return;
        }
        
        memcpy(buffer, _buffer, oldSize);
        CC_SAFE_FREE(_buffer);
        _transferBuffer = buffer;
        status.bufferSize += size;
        status.bufferSize -= oldSize;
    }
    CC_SAFE_FREE(tempBuffer);

    _size = size;
    _count = _size / _stride;
    _status = GFXStatus::SUCCESS;
}

bool CCMTLBuffer::resizeBuffer(uint size, uint count, uint oldSize, uint8_t* backupBuffer, bool needBackup)
{
    if(!backupBuffer)
        return false;
    
    if(_usage & GFXBufferUsage::VERTEX ||
       _usage & GFXBufferUsage::INDEX ||
       _usage & GFXBufferUsage::UNIFORM)
    {
        id<MTLBuffer> mtlBuffer = [id<MTLDevice>(((CCMTLDevice*)_device)->getMTLDevice() ) newBufferWithLength:size
                                                                                                       options:_mtlResourceOptions];
        if (mtlBuffer == nil)
        {
            CCASSERT(false, "CCMTLBuffer: resize metal buffer failed.");
            return false;
        }
        if(_mtlBuffer)
        {
            if(needBackup)
            {
                memcpy(backupBuffer, static_cast<uint8_t*>(_mtlBuffer.contents), oldSize);
            }
            else
            {
                memcpy(mtlBuffer.contents, backupBuffer, size);
            }
            [_mtlBuffer release];
        }
        
        _mtlBuffer = mtlBuffer;
    }
    else if(_usage & GFXBufferUsageBit::INDIRECT)
    {
        if(needBackup)
        {
            memcpy(backupBuffer, _indirects.data(), oldSize);
        }
        _indirects.resize(count);
        memcpy(_indirects.data(), static_cast<uint8_t*>(backupBuffer), needBackup ? oldSize : size);
    }
    else if(_usage & GFXBufferUsageBit::TRANSFER_SRC ||
            _usage & GFXBufferUsageBit::TRANSFER_DST)
    {
        GFXMemoryStatus& status = _device->getMemoryStatus();
        uint8_t* buffer = static_cast<uint8_t*>(CC_MALLOC(size));
        if(!buffer)
        {
            CCASSERT(false, "CCMTLBuffer: malloc buffer failed.");
            return false;
        }
        if(needBackup)
        {
            memcpy(backupBuffer, static_cast<uint8_t*>(_transferBuffer), oldSize);
        }
        
        CC_SAFE_FREE(_transferBuffer);
        _transferBuffer = buffer;
        memcpy(_transferBuffer, backupBuffer, needBackup ? oldSize : size);
        status.bufferSize += size;
        status.bufferSize -= oldSize;
    }
   else
   {
       return false;
   }
    
    return true;
}

void CCMTLBuffer::update(void* buffer, uint offset, uint size)
{
    if(_size < offset + size)
    {
        CC_LOG_INFO("CCMTLBuffer: the size of buffer is smaller than specified, try resize it.");
        resize(size);
        if(_status == GFXStatus::FAILED)
        {
            CC_LOG_INFO("CCMTLBuffer: buffer resize failed.");
            return;
        }
        CC_LOG_INFO("CCMTLBuffer: buffer resize succeeded.");
    }
    if (_usage & GFXBufferUsageBit::INDIRECT)
        memcpy((uint8_t*)_indirects.data() + offset, buffer, size);
    
    if(_transferBuffer)
        memcpy(_transferBuffer + offset, buffer, size);
    
    if(_mtlBuffer)
    {
        uint8_t* dst = (uint8_t*)(_mtlBuffer.contents);
        memcpy(dst + offset, buffer, size);
    }
    
    if (_buffer)
        memcpy(_buffer + offset, buffer, size);
}

NS_CC_END

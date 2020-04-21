#include "MTLStd.h"
#include "MTLTexture.h"
#include "MTLUtils.h"
#include "MTLDevice.h"

NS_CC_BEGIN

namespace
{
    uint8_t* convertData(uint8_t* source, uint length, GFXFormat type)
    {
        switch (type) {
            case GFXFormat::RGB8:   return mu::convertRGB8ToRGBA8(source, length);
            case GFXFormat::RGB32F: return mu::convertRGB32FToRGBA32F(source, length);
            default: return source;
        }
    }
}

CCMTLTexture::CCMTLTexture(GFXDevice* device) : GFXTexture(device) {}
CCMTLTexture::~CCMTLTexture() { destroy(); }

bool CCMTLTexture::initialize(const GFXTextureInfo& info)
{
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
    
    if (_flags & GFXTextureFlags::BAKUP_BUFFER)
    {
        _buffer = (uint8_t*)CC_MALLOC(_size);
        _device->getMemoryStatus().textureSize += _size;
    }
    
    auto isArray = _arrayLayer > 1;
    switch (_type) {
        case GFXTextureType::TEX1D:
            _viewType = isArray ? GFXTextureViewType::TV1D_ARRAY : GFXTextureViewType::TV1D;
            break;
        case GFXTextureType::TEX2D:
            if(_flags & GFXTextureFlagBit::CUBEMAP)
                _viewType = GFXTextureViewType::CUBE;
            else
                _viewType = isArray ? GFXTextureViewType::TV2D_ARRAY : GFXTextureViewType::TV2D;
            break;
        case GFXTextureType::TEX3D:
            _viewType = GFXTextureViewType::TV3D;
            break;
        default:
            break;
    }
    
    if (!createMTLTexture())
    {
        _status = GFXStatus::FAILED;
        return false;
    }
    
    _device->getMemoryStatus().textureSize += _size;
    _status = GFXStatus::SUCCESS;
    return true;
}

bool CCMTLTexture::createMTLTexture()
{
    _convertedFormat = mu::convertGFXPixelFormat(_format);
    MTLPixelFormat mtlFormat = mu::toMTLPixelFormat(_convertedFormat);
    if (mtlFormat == MTLPixelFormatInvalid)
        return false;
    
    MTLTextureDescriptor* descriptor = nullptr;
    switch (_viewType) {
        case GFXTextureViewType::TV2D:
        case GFXTextureViewType::TV2D_ARRAY:
            descriptor = [MTLTextureDescriptor texture2DDescriptorWithPixelFormat:mtlFormat
                                                                            width:_width
                                                                           height:_height
                                                                        mipmapped:_flags & GFXTextureFlags::GEN_MIPMAP];
            break;
        case GFXTextureViewType::CUBE:
            descriptor = [MTLTextureDescriptor textureCubeDescriptorWithPixelFormat:mtlFormat
                                                                               size:_width
                                                                          mipmapped:_flags & GFXTextureFlags::GEN_MIPMAP];
            break;
        default:
            CCASSERT(false, "Unsupported GFXTextureViewType, create MTLTextureDescriptor failed.");
            break;
    }
    
    if(descriptor == nullptr)
        return false;
    
    descriptor.usage = mu::toMTLTextureUsage(_usage);
    descriptor.textureType = mu::toMTLTextureType(_type, _arrayLayer, _flags);
    descriptor.sampleCount = mu::toMTLSampleCount(_samples);
    descriptor.mipmapLevelCount = _mipLevel;
    descriptor.arrayLength = _flags & GFXTextureFlagBit::CUBEMAP ? 1 : _arrayLayer;
    
    //FIXME: should change to MTLStorageModeManaged if texture usage is GFXTextureFlags::BAKUP_BUFFER?
    if (_usage & GFXTextureUsage::COLOR_ATTACHMENT ||
        _usage & GFXTextureUsage::DEPTH_STENCIL_ATTACHMENT ||
        _usage & GFXTextureUsage::INPUT_ATTACHMENT ||
        _usage & GFXTextureUsage::TRANSIENT_ATTACHMENT)
    {
        descriptor.resourceOptions = MTLResourceStorageModePrivate;
    }
    
    id<MTLDevice> mtlDevice = id<MTLDevice>(static_cast<CCMTLDevice*>(_device)->getMTLDevice() );
    _mtlTexture = [mtlDevice newTextureWithDescriptor:descriptor];
    
    return _mtlTexture != nil;
}

void CCMTLTexture::destroy()
{
    if (_buffer)
    {
        CC_FREE(_buffer);
        _device->getMemoryStatus().textureSize -= _size;
        _buffer = nullptr;
    }
    
    if (_mtlTexture)
    {
        [_mtlTexture release];
        _mtlTexture = nil;
    }
    
    _status = GFXStatus::UNREADY;
}

void CCMTLTexture::resize(uint width, uint height)
{
    if(_width == width && _height == height)
        return;
    
    auto oldSize = _size;
    auto oldWidth = _width;
    auto oldHeight = _height;
    id<MTLTexture> oldMTLTexture = _mtlTexture;
    
    _width = width;
    _height = height;
    _size = GFXFormatSize(_format, _width, _height, _depth);
    if(!createMTLTexture())
    {
        _width = oldWidth;
        _height = oldHeight;
        _size = oldSize;
        _mtlTexture = oldMTLTexture;
        return;
    }
    
    if(oldMTLTexture)
    {
        [oldMTLTexture release];
    }
    
    _device->getMemoryStatus().textureSize -= oldSize;
    if (_flags & GFXTextureFlags::BAKUP_BUFFER)
    {
        _device->getMemoryStatus().textureSize -= oldSize;
        CC_FREE(_buffer);
        _buffer = (uint8_t*)CC_MALLOC(_size);
        _device->getMemoryStatus().textureSize += _size;
    }
    
    _device->getMemoryStatus().textureSize += _size;
    _status = GFXStatus::SUCCESS;
}

void CCMTLTexture::update(uint8_t* const* datas, const GFXBufferTextureCopyList& regions)
{
    if (!_mtlTexture)
        return;

    uint n = 0;
    uint w = 0;
    uint h = 0;
    switch (_viewType) {
        case GFXTextureViewType::TV2D:
            for (size_t i = 0; i < regions.size(); i++) {
                const auto& region = regions[i];
                w = region.texExtent.width;
                h = region.texExtent.height;
                auto level = region.texSubres.baseMipLevel;
                auto levelCount = level + region.texSubres.levelCount;
                for (; level <  levelCount ; level++) {
                    w = w >> level;
                    h = h >> level;
                    uint8_t* buffer = region.buffOffset + region.buffTexHeight * region.buffStride + datas[n];
                    uint8_t* convertedData = convertData(buffer, w * h, _format);
                    MTLRegion mtlRegion = { {(uint)region.texOffset.x, (uint)region.texOffset.y, (uint)region.texOffset.z}, {w, h, 1} };
                    [_mtlTexture replaceRegion:mtlRegion
                                   mipmapLevel:level
                                     withBytes:convertedData
                                   bytesPerRow:GFX_FORMAT_INFOS[(uint)_convertedFormat].size * w];
                    if (convertedData != datas[n])
                        CC_FREE(convertedData);
                    n++;
                }
            }
            break;
        case GFXTextureViewType::TV2D_ARRAY:
        case GFXTextureViewType::CUBE:
            for (size_t i = 0; i < regions.size(); i++) {
                const auto& region = regions[i];
                auto layer = region.texSubres.baseArrayLayer;
                auto layerCount = layer + region.texSubres.layerCount;
                for (; layer < layerCount; layer++) {
                    w = region.texExtent.width;
                    h = region.texExtent.height;
                    auto level = region.texSubres.baseMipLevel;
                    auto levelCount = level + region.texSubres.levelCount;
                    for (; level < levelCount; level++) {
                        w = w >> level;
                        h = w >> level;
                        uint8_t* buffer = region.buffOffset + region.buffTexHeight * region.buffStride + datas[n];
                        uint8_t* convertedData = convertData(buffer, w * h, _format);
                        MTLRegion mtlRegion = { {(uint)region.texOffset.x, (uint)region.texOffset.y, (uint)region.texOffset.z}, {w, h, 1} };
                        [_mtlTexture replaceRegion:mtlRegion
                                       mipmapLevel:level
                                             slice:layer
                                         withBytes:convertedData
                                       bytesPerRow:GFX_FORMAT_INFOS[(uint)_convertedFormat].size * w
                                     bytesPerImage:0];
                        if (convertedData != datas[n++])
                            CC_FREE(convertedData);
                    }
                }
            }
            break;
        default:
            CCASSERT(false, "Unsupported GFXTextureViewType, metal texture update failed.");
            break;
    }
}

NS_CC_END

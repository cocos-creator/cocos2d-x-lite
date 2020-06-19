#ifndef CC_CORE_GFX_TEXTURE_H_
#define CC_CORE_GFX_TEXTURE_H_

#include "GFXDef.h"

namespace cc {
namespace gfx {

class CC_DLL Texture : public GFXObject {
public:
    Texture(Device *device);
    virtual ~Texture();

public:
    virtual bool initialize(const TextureInfo &info) = 0;
    virtual bool initialize(const TextureViewInfo &info) = 0;
    virtual void destroy() = 0;
    virtual void resize(uint width, uint height) = 0;

    CC_INLINE TextureType getType() const { return _type; }
    CC_INLINE TextureUsage getUsage() const { return _usage; }
    CC_INLINE GFXFormat getFormat() const { return _format; }
    CC_INLINE uint getWidth() const { return _width; }
    CC_INLINE uint getHeight() const { return _height; }
    CC_INLINE uint getDepth() const { return _depth; }
    CC_INLINE uint getArrayLayer() const { return _arrayLayer; }
    CC_INLINE uint getMipLevel() const { return _mipLevel; }
    CC_INLINE uint getSize() const { return _size; }
    CC_INLINE GFXSampleCount getSamples() const { return _samples; }
    CC_INLINE TextureFlags getFlags() const { return _flags; }
    CC_INLINE uint8_t *getBuffer() const { return _buffer; }

protected:
    Device *_device = nullptr;
    TextureType _type = TextureType::TEX2D;
    TextureUsage _usage = TextureUsageBit::NONE;
    GFXFormat _format = GFXFormat::UNKNOWN;
    uint _width = 0;
    uint _height = 0;
    uint _depth = 1;
    uint _arrayLayer = 1;
    uint _mipLevel = 1;
    uint _size = 0;
    GFXSampleCount _samples = GFXSampleCount::X1;
    TextureFlags _flags = TextureFlagBit::NONE;
    uint8_t *_buffer = nullptr;
};

} // namespace gfx
} // namespace cc

#endif // CC_CORE_GFX_TEXTURE_H_

#pragma once

#import <Metal/MTLBuffer.h>
#import <Metal/MTLRenderCommandEncoder.h>
#import <Metal/MTLStageInputOutputDescriptor.h>

namespace cc {
namespace gfx {

class CCMTLBuffer : public GFXBuffer {
public:
    CCMTLBuffer(GFXDevice *device);
    ~CCMTLBuffer();

    virtual bool initialize(const GFXBufferInfo &info) override;
    virtual void destroy() override;
    virtual void resize(uint size) override;
    virtual void update(void *buffer, uint offset, uint size) override;

    CC_INLINE id<MTLBuffer> getMTLBuffer() const { return _mtlBuffer; }
    CC_INLINE uint8_t *getTransferBuffer() const { return _transferBuffer; }
    CC_INLINE MTLIndexType getIndexType() const { return _indexType; }
    CC_INLINE bool isIndexIndirectCommand() const { return _isIndexIndirectCommand; }
    void encodeBuffer(id<MTLRenderCommandEncoder> encoder, uint offset, uint binding, GFXShaderType stages);

private:
    void resizeBuffer(uint8_t **, uint, uint);
    bool createMTLBuffer(uint size, GFXMemoryUsage usage);

    id<MTLBuffer> _mtlBuffer = nullptr;
    uint8_t *_transferBuffer = nullptr;
    MTLIndexType _indexType = MTLIndexTypeUInt16;
    MTLResourceOptions _mtlResourceOptions = MTLResourceStorageModePrivate;

    // to store vertex and ubo data when size is small, otherwise use MTLBuffer instead.
    bool _useOptimizedBufferEncoder = false;
    uint8_t *_bufferBytes = nullptr;

    bool _isIndexIndirectCommand = false;
};

} // namespace gfx
} // namespace cc

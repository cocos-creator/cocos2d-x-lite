#include "MTLStd.h"

#include "MTLBindingLayout.h"
#include "MTLBuffer.h"
#include "MTLCommandAllocator.h"
#include "MTLCommandBuffer.h"
#include "MTLContext.h"
#include "MTLDevice.h"
#include "MTLFence.h"
#include "MTLFrameBuffer.h"
#include "MTLInputAssembler.h"
#include "MTLPipelineLayout.h"
#include "MTLPipelineState.h"
#include "MTLQueue.h"
#include "MTLRenderPass.h"
#include "MTLSampler.h"
#include "MTLShader.h"
#include "MTLStateCache.h"
#include "MTLTexture.h"
#include "MTLUtils.h"
#include <platform/mac/CCView.h>

#import <MetalKit/MTKView.h>

namespace cc {
namespace gfx {

CCMTLDevice::CCMTLDevice() {}
CCMTLDevice::~CCMTLDevice() {}

bool CCMTLDevice::initialize(const DeviceInfo &info) {
    _gfxAPI = GFXAPI::METAL;
    _deviceName = "Metal";
    _width = info.width;
    _height = info.height;
    _nativeWidth = info.nativeWidth;
    _nativeHeight = info.nativeHeight;
    _windowHandle = info.windowHandle;

    _stateCache = CC_NEW(CCMTLStateCache);

    _mtkView = (MTKView *)_windowHandle;
    _mtlDevice = ((MTKView *)_mtkView).device;

    GFXContextInfo contextCreateInfo;
    contextCreateInfo.windowHandle = _windowHandle;
    contextCreateInfo.sharedCtx = info.sharedCtx;
    _context = CC_NEW(CCMTLContext(this));
    if (!_context->initialize(contextCreateInfo)) {
        destroy();
        return false;
    }

    GFXQueueInfo queue_info;
    queue_info.type = GFXQueueType::GRAPHICS;
    _queue = createQueue(queue_info);

    GFXCommandAllocatorInfo cmd_alloc_info;
    _cmdAllocator = createCommandAllocator(cmd_alloc_info);

    _minClipZ = 0;
    _depthBits = 24;
    _stencilBits = 8;
    //TODO: other information

    _mtlFeatureSet = mu::highestSupportedFeatureSet(id<MTLDevice>(_mtlDevice));
    auto gpuFamily = mu::getGPUFamily(MTLFeatureSet(_mtlFeatureSet));
    _maxVertexAttributes = mu::getMaxVertexAttributes(gpuFamily);
    _maxTextureUnits = mu::getMaxEntriesInTextureArgumentTable(gpuFamily);
    _maxSamplerUnits = mu::getMaxEntriesInSamplerStateArgumentTable(gpuFamily);
    _maxTextureSize = mu::getMaxTexture2DWidthHeight(gpuFamily);
    _maxCubeMapTextureSize = mu::getMaxCubeMapTextureWidthHeight(gpuFamily);
    _maxColorRenderTargets = mu::getMaxColorRenderTarget(gpuFamily);
    _icbSuppored = mu::isIndirectCommandBufferSupported(MTLFeatureSet(_mtlFeatureSet));
    if ([id<MTLDevice>(_mtlDevice) isDepth24Stencil8PixelFormatSupported]) {
        _depthBits = 24;
        _stencilBits = 8;
        _features[(int)GFXFeature::FORMAT_D24S8] = true;
    }

    _features[static_cast<int>(GFXFeature::COLOR_FLOAT)] = mu::isColorBufferFloatSupported(gpuFamily);
    _features[static_cast<int>(GFXFeature::COLOR_HALF_FLOAT)] = mu::isColorBufferHalfFloatSupported(gpuFamily);
    _features[static_cast<int>(GFXFeature::TEXTURE_FLOAT_LINEAR)] = mu::isLinearTextureSupported(gpuFamily);
    _features[static_cast<int>(GFXFeature::TEXTURE_HALF_FLOAT_LINEAR)] = mu::isLinearTextureSupported(gpuFamily);

    String compressedFormats;
    if (mu::isPVRTCSuppported(gpuFamily)) {
        _features[static_cast<int>(GFXFeature::FORMAT_PVRTC)] = true;
        compressedFormats += "pvrtc ";
    }
    if (mu::isEAC_ETCCSuppported(gpuFamily)) {
        _features[static_cast<int>(GFXFeature::FORMAT_ETC2)] = true;
        compressedFormats += "etc2 ";
    }
    if (mu::isASTCSuppported(gpuFamily)) {
        _features[static_cast<int>(GFXFeature::FORMAT_ASTC)] = true;
        compressedFormats += "astc ";
    }
    if (mu::isBCSupported(gpuFamily)) {
        _features[static_cast<int>(GFXFeature::FORMAT_ASTC)] = true;
        compressedFormats += "dxt ";
    }

    _features[(int)GFXFeature::TEXTURE_FLOAT] = true;
    _features[(int)GFXFeature::TEXTURE_HALF_FLOAT] = true;
    _features[(int)GFXFeature::FORMAT_R11G11B10F] = true;
    _features[(int)GFXFeature::MSAA] = true;
    _features[(int)GFXFeature::INSTANCED_ARRAYS] = true;
    _features[static_cast<uint>(GFXFeature::DEPTH_BOUNDS)] = false;
    _features[static_cast<uint>(GFXFeature::LINE_WIDTH)] = false;
    _features[static_cast<uint>(GFXFeature::STENCIL_COMPARE_MASK)] = false;
    _features[static_cast<uint>(GFXFeature::STENCIL_WRITE_MASK)] = false;
    _features[static_cast<uint>(GFXFeature::FORMAT_RGB8)] = false;
    _features[static_cast<uint>(GFXFeature::FORMAT_D16)] = mu::isDepthStencilFormatSupported(GFXFormat::D16, gpuFamily);
    _features[static_cast<uint>(GFXFeature::FORMAT_D16S8)] = mu::isDepthStencilFormatSupported(GFXFormat::D16S8, gpuFamily);
    _features[static_cast<uint>(GFXFeature::FORMAT_D32F)] = mu::isDepthStencilFormatSupported(GFXFormat::D32F, gpuFamily);
    _features[static_cast<uint>(GFXFeature::FORMAT_D32FS8)] = mu::isDepthStencilFormatSupported(GFXFormat::D32F_S8, gpuFamily);

    CC_LOG_INFO("Metal Feature Set: %s", mu::featureSetToString(MTLFeatureSet(_mtlFeatureSet)).c_str());

    return true;
}

void CCMTLDevice::destroy() {
    CC_SAFE_DELETE(_stateCache);
}

void CCMTLDevice::resize(uint width, uint height) {}

void CCMTLDevice::present() {
    ((CCMTLCommandAllocator *)_cmdAllocator)->releaseCmds();
    CCMTLQueue *queue = (CCMTLQueue *)_queue;
    _numDrawCalls = queue->_numDrawCalls;
    _numInstances = queue->_numInstances;
    _numTriangles = queue->_numTriangles;

    // Clear queue stats
    queue->_numDrawCalls = 0;
    queue->_numInstances = 0;
    queue->_numTriangles = 0;
}

GFXFence *CCMTLDevice::createFence(const GFXFenceInfo &info) {
    auto fence = CC_NEW(CCMTLFence(this));
    if (fence && fence->initialize(info))
        return fence;

    CC_SAFE_DESTROY(fence);
    return nullptr;
}

GFXQueue *CCMTLDevice::createQueue(const GFXQueueInfo &info) {
    auto queue = CC_NEW(CCMTLQueue(this));
    if (queue && queue->initialize(info))
        return queue;

    CC_SAFE_DESTROY(queue);
    return nullptr;
}

GFXCommandAllocator *CCMTLDevice::createCommandAllocator(const GFXCommandAllocatorInfo &info) {
    auto allocator = CC_NEW(CCMTLCommandAllocator(this));
    if (allocator && allocator->initialize(info))
        return allocator;

    CC_SAFE_DESTROY(allocator);
    return nullptr;
}

GFXCommandBuffer *CCMTLDevice::createCommandBuffer(const GFXCommandBufferInfo &info) {
    auto commandBuffer = CC_NEW(CCMTLCommandBuffer(this));
    if (commandBuffer && commandBuffer->initialize(info))
        return commandBuffer;

    CC_SAFE_DESTROY(commandBuffer);
    return nullptr;
}

Buffer *CCMTLDevice::createBuffer(const BufferInfo &info) {
    auto buffer = CC_NEW(CCMTLBuffer(this));
    if (buffer && buffer->initialize(info))
        return buffer;

    CC_SAFE_DESTROY(buffer);
    return nullptr;
}

GFXTexture *CCMTLDevice::createTexture(const GFXTextureInfo &info) {
    auto texture = CC_NEW(CCMTLTexture(this));
    if (texture && texture->initialize(info))
        return texture;

    CC_SAFE_DESTROY(texture);
    return nullptr;
}

GFXTexture *CCMTLDevice::createTexture(const GFXTextureViewInfo &info) {
    auto texture = CC_NEW(CCMTLTexture(this));
    if (texture && texture->initialize(info))
        return texture;

    CC_SAFE_DESTROY(texture);
    return nullptr;
}

GFXSampler *CCMTLDevice::createSampler(const GFXSamplerInfo &info) {
    auto sampler = CC_NEW(CCMTLSampler(this));
    if (sampler && sampler->initialize(info))
        return sampler;

    CC_SAFE_DESTROY(sampler);
    return sampler;
}

GFXShader *CCMTLDevice::createShader(const GFXShaderInfo &info) {
    auto shader = CC_NEW(CCMTLShader(this));
    if (shader && shader->initialize(info))
        return shader;

    CC_SAFE_DESTROY(shader);
    return shader;
}

GFXInputAssembler *CCMTLDevice::createInputAssembler(const GFXInputAssemblerInfo &info) {
    auto ia = CC_NEW(CCMTLInputAssembler(this));
    if (ia && ia->initialize(info))
        return ia;

    CC_SAFE_DESTROY(ia);
    return nullptr;
}

GFXRenderPass *CCMTLDevice::createRenderPass(const GFXRenderPassInfo &info) {
    auto renderPass = CC_NEW(CCMTLRenderPass(this));
    if (renderPass && renderPass->initialize(info))
        return renderPass;

    CC_SAFE_DESTROY(renderPass);
    return nullptr;
}

GFXFramebuffer *CCMTLDevice::createFramebuffer(const GFXFramebufferInfo &info) {
    auto frameBuffer = CC_NEW(CCMTLFramebuffer(this));
    if (frameBuffer && frameBuffer->initialize(info))
        return frameBuffer;

    CC_SAFE_DESTROY(frameBuffer);
    return nullptr;
}

GFXBindingLayout *CCMTLDevice::createBindingLayout(const GFXBindingLayoutInfo &info) {
    auto bl = CC_NEW(CCMTLBindingLayout(this));
    if (bl && bl->initialize(info))
        return bl;

    CC_SAFE_DESTROY(bl);
    return nullptr;
}

GFXPipelineState *CCMTLDevice::createPipelineState(const GFXPipelineStateInfo &info) {
    auto ps = CC_NEW(CCMTLPipelineState(this));
    if (ps && ps->initialize(info))
        return ps;

    CC_SAFE_DESTROY(ps);
    return nullptr;
}

GFXPipelineLayout *CCMTLDevice::createPipelineLayout(const GFXPipelineLayoutInfo &info) {
    auto pl = CC_NEW(CCMTLPipelineLayout(this));
    if (pl && pl->initialize(info))
        return pl;

    CC_SAFE_DESTROY(pl);
    return nullptr;
}

void CCMTLDevice::copyBuffersToTexture(const GFXDataArray &buffers, GFXTexture *dst, const BufferTextureCopyList &regions) {
    static_cast<CCMTLTexture *>(dst)->update(buffers.datas.data(), regions);
}

void CCMTLDevice::blitBuffer(void *srcData, uint offset, uint size, void *dstBuffer) {
    id<MTLBuffer> sourceBuffer = id<MTLBuffer>(_blitedBuffer);
    if (sourceBuffer == nil || sourceBuffer.allocatedSize < size) {
        if (sourceBuffer)
            [sourceBuffer release];
        sourceBuffer = [id<MTLDevice>(_mtlDevice) newBufferWithBytes:srcData
                                                              length:size
                                                             options:MTLResourceStorageModeShared];
    }

    // Create a command buffer for GPU work.
    id<MTLCommandBuffer> commandBuffer = [static_cast<View *>(_mtkView).mtlCommandQueue commandBuffer];

    // Encode a blit pass to copy data from the source buffer to the private buffer.
    id<MTLBlitCommandEncoder> blitCommandEncoder = [commandBuffer blitCommandEncoder];
    [blitCommandEncoder copyFromBuffer:sourceBuffer
                          sourceOffset:0
                              toBuffer:id<MTLBuffer>(dstBuffer)
                     destinationOffset:offset
                                  size:size];
    [blitCommandEncoder endEncoding];
    [commandBuffer commit];
}

} // namespace gfx
} // namespace cc

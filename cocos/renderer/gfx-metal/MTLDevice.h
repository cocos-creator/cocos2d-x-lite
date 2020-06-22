#pragma once

namespace cc {
namespace gfx {

class CCMTLStateCache;

class CCMTLDevice : public Device {
public:
    CCMTLDevice();
    ~CCMTLDevice();

    virtual bool initialize(const DeviceInfo &info) override;
    virtual void destroy() override;
    virtual void resize(uint width, uint height) override;
    virtual void acquire() override{};
    virtual void present() override;
    virtual Fence *createFence(const FenceInfo &info) override;
    virtual Queue *createQueue(const QueueInfo &info) override;
    virtual CommandAllocator *createCommandAllocator(const CommandAllocatorInfo &info) override;
    virtual CommandBuffer *createCommandBuffer(const CommandBufferInfo &info) override;
    virtual Buffer *createBuffer(const BufferInfo &info) override;
    virtual Texture *createTexture(const TextureInfo &info) override;
    virtual Texture *createTexture(const TextureViewInfo &info) override;
    virtual Sampler *createSampler(const SamplerInfo &info) override;
    virtual GFXShader *createShader(const GFXShaderInfo &info) override;
    virtual InputAssembler *createInputAssembler(const InputAssemblerInfo &info) override;
    virtual RenderPass *createRenderPass(const RenderPassInfo &info) override;
    virtual Framebuffer *createFramebuffer(const FramebufferInfo &info) override;
    virtual BindingLayout *createBindingLayout(const BindingLayoutInfo &info) override;
    virtual PipelineState *createPipelineState(const PipelineStateInfo &info) override;
    virtual PipelineLayout *createPipelineLayout(const PipelineLayoutInfo &info) override;
    virtual void copyBuffersToTexture(const GFXDataArray &buffers, Texture *dst, const BufferTextureCopyList &regions) override;
    virtual void blitBuffer(void *srcBuffer, uint offset, uint size, void *dstBuffer);

    CC_INLINE void *getMTKView() const { return _mtkView; }
    CC_INLINE void *getMTLDevice() const { return _mtlDevice; }
    CC_INLINE CCMTLStateCache *getStateCache() const { return _stateCache; }
    CC_INLINE uint getMaximumSamplerUnits() const { return _maxSamplerUnits; }
    CC_INLINE uint getMaximumColorRenderTargets() const { return _maxColorRenderTargets; }
    CC_INLINE bool isIndirectCommandBufferSupported() const { return _icbSuppored; }

private:
    void *_mtkView = nullptr;
    void *_mtlDevice = nullptr;
    CCMTLStateCache *_stateCache = nullptr;
    unsigned long _mtlFeatureSet = 0;
    uint _maxSamplerUnits = 0;
    uint _maxColorRenderTargets = 0;
    bool _icbSuppored = false;
    void *_blitedBuffer = nullptr;
};

} // namespace gfx
} // namespace cc

#pragma once

NS_CC_BEGIN

class CCMTLStateCache;

class CCMTLDevice : public GFXDevice
{
public:
    CCMTLDevice();
    ~CCMTLDevice();
    
    virtual bool Initialize(const GFXDeviceInfo& info) override;
    virtual void Destroy() override;
    virtual void Resize(uint width, uint height) override;
    virtual void Present() override;
    virtual GFXWindow* CreateGFXWindow(const GFXWindowInfo& info) override;
    virtual GFXQueue* CreateGFXQueue(const GFXQueueInfo& info) override;
    virtual GFXCommandAllocator* CreateGFXCommandAllocator(const GFXCommandAllocatorInfo& info) override;
    virtual GFXCommandBuffer* CreateGFXCommandBuffer(const GFXCommandBufferInfo& info) override;
    virtual GFXBuffer* CreateGFXBuffer(const GFXBufferInfo& info) override;
    virtual GFXTexture* CreateGFXTexture(const GFXTextureInfo& info) override;
    virtual GFXTextureView* CreateGFXTextureView(const GFXTextureViewInfo& info) override;
    virtual GFXSampler* CreateGFXSampler(const GFXSamplerInfo& info) override;
    virtual GFXShader* CreateGFXShader(const GFXShaderInfo& info) override;
    virtual GFXInputAssembler* CreateGFXInputAssembler(const GFXInputAssemblerInfo& info) override;
    virtual GFXRenderPass* CreateGFXRenderPass(const GFXRenderPassInfo& info) override;
    virtual GFXFramebuffer* CreateGFXFramebuffer(const GFXFramebufferInfo& info) override;
    virtual GFXBindingLayout* CreateGFXBindingLayout(const GFXBindingLayoutInfo& info) override;
    virtual GFXPipelineState* CreateGFXPipelineState(const GFXPipelineStateInfo& info) override;
    virtual GFXPipelineLayout* CreateGFXPipelieLayout(const GFXPipelineLayoutInfo& info) override;
    
    CC_INLINE void* getMTKView() const { return _mtkView; }
    CC_INLINE CCMTLStateCache* getStateCache() const { return _stateCache; }
    
private:
    void* _mtkView= nullptr;
    CCMTLStateCache* _stateCache = nullptr;
};

NS_CC_END

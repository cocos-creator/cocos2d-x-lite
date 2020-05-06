#ifndef CC_GFXVULKAN_CCVK_DEVICE_H_
#define CC_GFXVULKAN_CCVK_DEVICE_H_

NS_CC_BEGIN

class CCVKStateCache;
class CCVKGPUDevice;
class CCVKGPUSwapchain;

class CC_VULKAN_API CCVKDevice : public GFXDevice
{
public:
    CCVKDevice();
    ~CCVKDevice();

    CCVKStateCache* stateCache = nullptr;

public:
    bool initialize(const GFXDeviceInfo& info) override;
    void destroy() override;
    void resize(uint width, uint height) override;
    void begin() override;
    void present() override;
    GFXWindow* createWindow(const GFXWindowInfo& info) override;
    GFXQueue* createQueue(const GFXQueueInfo& info) override;
    GFXCommandAllocator* createCommandAllocator(const GFXCommandAllocatorInfo& info) override;
    GFXCommandBuffer* createCommandBuffer(const GFXCommandBufferInfo& info) override;
    GFXBuffer* createBuffer(const GFXBufferInfo& info) override;
    GFXTexture* createTexture(const GFXTextureInfo& info) override;
    GFXTextureView* createTextureView(const GFXTextureViewInfo& info) override;
    GFXSampler* createSampler(const GFXSamplerInfo& info) override;
    GFXShader* createShader(const GFXShaderInfo& info) override;
    GFXInputAssembler* createInputAssembler(const GFXInputAssemblerInfo& info) override;
    GFXRenderPass* createRenderPass(const GFXRenderPassInfo& info) override;
    GFXFramebuffer* createFramebuffer(const GFXFramebufferInfo& info) override;
    GFXBindingLayout* createBindingLayout(const GFXBindingLayoutInfo& info) override;
    virtual GFXPipelineState* createPipelineState(const GFXPipelineStateInfo& info) override;
    virtual GFXPipelineLayout* createPipelineLayout(const GFXPipelineLayoutInfo& info) override;
    virtual void copyBuffersToTexture(const GFXDataArray& buffers, GFXTexture* dst, const GFXBufferTextureCopyList& regions) override;

    CC_INLINE bool checkExtension(const String& extension) const {
        return std::find_if(_extensions.begin(), _extensions.end(),
            [extension](auto &device_extension) {
            return std::strcmp(device_extension, extension.c_str()) == 0;
        }) != _extensions.end();
    }

    CC_INLINE CCVKGPUDevice* gpuDevice() const { return _gpuDevice; }
    CC_INLINE const std::vector<const char *>& getLayers() const { return _layers; }
    CC_INLINE const std::vector<const char *>& getExtensions() const { return _extensions; }

private:
    void buildSwapchain();

    CCVKGPUDevice* _gpuDevice = nullptr;
    std::vector<const char *> _layers;
    std::vector<const char *> _extensions;
};

NS_CC_END

#endif // CC_GFXVULKAN_CCVK_DEVICE_H_

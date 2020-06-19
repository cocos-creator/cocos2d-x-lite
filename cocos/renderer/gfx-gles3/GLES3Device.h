#ifndef CC_GFXGLES3_GLES3_DEVICE_H_
#define CC_GFXGLES3_GLES3_DEVICE_H_

namespace cc {
namespace gfx {

class GLES3StateCache;

class CC_GLES3_API GLES3Device : public Device {
public:
    GLES3Device();
    ~GLES3Device();

    GLES3StateCache *stateCache = nullptr;

public:
    virtual bool initialize(const DeviceInfo &info) override;
    virtual void destroy() override;
    virtual void resize(uint width, uint height) override;
    virtual void acquire() override {}
    virtual void present() override;
    virtual GFXFence *createFence(const GFXFenceInfo &info) override;
    virtual GFXQueue *createQueue(const GFXQueueInfo &info) override;
    virtual GFXCommandAllocator *createCommandAllocator(const GFXCommandAllocatorInfo &info) override;
    virtual GFXCommandBuffer *createCommandBuffer(const GFXCommandBufferInfo &info) override;
    virtual Buffer *createBuffer(const BufferInfo &info) override;
    virtual GFXTexture *createTexture(const GFXTextureInfo &info) override;
    virtual GFXTexture *createTexture(const GFXTextureViewInfo &info) override;
    virtual GFXSampler *createSampler(const GFXSamplerInfo &info) override;
    virtual GFXShader *createShader(const GFXShaderInfo &info) override;
    virtual GFXInputAssembler *createInputAssembler(const GFXInputAssemblerInfo &info) override;
    virtual GFXRenderPass *createRenderPass(const GFXRenderPassInfo &info) override;
    virtual GFXFramebuffer *createFramebuffer(const GFXFramebufferInfo &info) override;
    virtual GFXBindingLayout *createBindingLayout(const GFXBindingLayoutInfo &info) override;
    virtual GFXPipelineState *createPipelineState(const GFXPipelineStateInfo &info) override;
    virtual GFXPipelineLayout *createPipelineLayout(const GFXPipelineLayoutInfo &info) override;
    virtual void copyBuffersToTexture(const GFXDataArray &buffers, GFXTexture *dst, const BufferTextureCopyList &regions) override;

    CC_INLINE bool useVAO() const { return _useVAO; }

    CC_INLINE bool checkExtension(const String &extension) const {
        for (size_t i = 0; i < _extensions.size(); ++i) {
            if (_extensions[i].find(extension) != String::npos) {
                return true;
            }
        }
        return false;
    }

private:
    StringArray _extensions;
    bool _useVAO = true;
};

} // namespace gfx
} // namespace cc

#endif

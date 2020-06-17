#ifndef CC_GFXVULKAN_CCVK_TEXTURE_H_
#define CC_GFXVULKAN_CCVK_TEXTURE_H_

namespace cc {
namespace gfx {

class CCVKGPUTexture;
class CCVKGPUTextureView;

class CC_VULKAN_API CCVKTexture : public GFXTexture {
public:
    CCVKTexture(GFXDevice *device);
    ~CCVKTexture();

public:
    bool initialize(const GFXTextureInfo &info);
    bool initialize(const GFXTextureViewInfo &info);
    void destroy();
    void resize(uint width, uint height);

    CC_INLINE CCVKGPUTexture *gpuTexture() const { return _gpuTexture; }
    CC_INLINE CCVKGPUTextureView *gpuTextureView() const { return _gpuTextureView; }

private:
    bool createTextureView(const GFXTextureViewInfo &info);

    CCVKGPUTexture *_gpuTexture = nullptr;
    CCVKGPUTextureView *_gpuTextureView = nullptr;
};

} // namespace gfx
} // namespace cc

#endif

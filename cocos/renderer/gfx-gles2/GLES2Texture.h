#ifndef CC_GFXGLES2_GLES2_TEXTURE_H_
#define CC_GFXGLES2_GLES2_TEXTURE_H_

namespace cc {
namespace gfx {

class GLES2GPUTexture;

class CC_GLES2_API GLES2Texture : public GFXTexture {
public:
    GLES2Texture(Device *device);
    ~GLES2Texture();

public:
    virtual bool initialize(const GFXTextureInfo &info) override;
    virtual bool initialize(const GFXTextureViewInfo &info) override;
    virtual void destroy() override;
    virtual void resize(uint width, uint height) override;

    CC_INLINE GLES2GPUTexture *gpuTexture() const { return _gpuTexture; }

private:
    GLES2GPUTexture *_gpuTexture = nullptr;
};

} // namespace gfx
} // namespace cc

#endif

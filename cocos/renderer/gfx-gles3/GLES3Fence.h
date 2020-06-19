#ifndef CC_GFXGLES3_GLES3_FENCE_H_
#define CC_GFXGLES3_GLES3_FENCE_H_

namespace cc {
namespace gfx {

class GLES3GPUFence;

class CC_GLES3_API GLES3Fence : public GFXFence {
public:
    GLES3Fence(Device *device);
    ~GLES3Fence();

public:
    virtual bool initialize(const GFXFenceInfo &info) override;
    virtual void destroy() override;
    virtual void wait() override;
    virtual void reset() override;

private:
    GLES3GPUFence *_gpuFence = nullptr;
};

} // namespace gfx
} // namespace cc

#endif

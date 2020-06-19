#ifndef CC_GFXVULKAN_CCVK_BINDING_LAYOUT_H_
#define CC_GFXVULKAN_CCVK_BINDING_LAYOUT_H_

namespace cc {
namespace gfx {

class CCVKGPUBindingLayout;

class CC_VULKAN_API CCVKBindingLayout : public GFXBindingLayout {
public:
    CCVKBindingLayout(Device *device);
    ~CCVKBindingLayout();

public:
    bool initialize(const GFXBindingLayoutInfo &info);
    void destroy();
    void update();

    CC_INLINE CCVKGPUBindingLayout *gpuBindingLayout() const { return _gpuBindingLayout; }

private:
    CCVKGPUBindingLayout *_gpuBindingLayout = nullptr;
};

} // namespace gfx
} // namespace cc

#endif

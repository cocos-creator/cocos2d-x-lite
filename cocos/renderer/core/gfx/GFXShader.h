#ifndef CC_CORE_GFX_SHADER_H_
#define CC_CORE_GFX_SHADER_H_

#include "GFXDef.h"

namespace cc {
namespace gfx {

class CC_DLL GFXShader : public GFXObject {
public:
    GFXShader(Device *device);
    virtual ~GFXShader();

public:
    virtual bool initialize(const GFXShaderInfo &info) = 0;
    virtual void destroy() = 0;

    CC_INLINE Device *getDevice() const { return _device; }
    // Can not rename to `id` as JS code, because `id` is a keyword in objective-c.
    CC_INLINE uint getHash() const { return _hash; }
    CC_INLINE const String &getName() const { return _name; }
    CC_INLINE const GFXShaderStageList &getStages() const { return _stages; }
    CC_INLINE const GFXAttributeList &getAttributes() const { return _attributes; }
    CC_INLINE const GFXUniformBlockList &getBlocks() const { return _blocks; }
    CC_INLINE const GFXUniformSamplerList &getSamplers() const { return _samplers; }

protected:
    Device *_device = nullptr;
    uint _hash = 0;
    String _name;
    GFXShaderStageList _stages;
    GFXAttributeList _attributes;
    GFXUniformBlockList _blocks;
    GFXUniformSamplerList _samplers;
};

} // namespace gfx
} // namespace cc

#endif // CC_CORE_GFX_TEXTURE_VIEW_H_

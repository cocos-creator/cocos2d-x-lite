#include "CoreStd.h"
#include "GFXBindingLayout.h"

namespace cc {
namespace gfx {

BindingLayout::BindingLayout(Device *device)
: GFXObject(GFXObjectType::BINDING_LAYOUT), _device(device) {
}

BindingLayout::~BindingLayout() {
}

void BindingLayout::bindBuffer(uint binding, Buffer *buffer) {
    for (size_t i = 0; i < _bindingUnits.size(); ++i) {
        GFXBindingUnit &bindingUnit = _bindingUnits[i];
        if (bindingUnit.binding == binding) {
            if (bindingUnit.type == GFXBindingType::UNIFORM_BUFFER) {
                if (bindingUnit.buffer != buffer) {
                    bindingUnit.buffer = buffer;
                    _isDirty = true;
                }
            } else {
                CCASSERT(false, "Setting binding is not GFXBindingType.UNIFORM_BUFFER.");
            }
            return;
        }
    }
}

void BindingLayout::bindTexture(uint binding, Texture *texture) {
    for (size_t i = 0; i < _bindingUnits.size(); ++i) {
        GFXBindingUnit &bindingUnit = _bindingUnits[i];
        if (bindingUnit.binding == binding) {
            if (bindingUnit.type == GFXBindingType::SAMPLER) {
                if (bindingUnit.texture != texture) {
                    bindingUnit.texture = texture;
                    _isDirty = true;
                }
            } else {
                CCASSERT(false, "Setting binding is not GFXBindingType.SAMPLER.");
            }
            return;
        }
    }
}

void BindingLayout::bindSampler(uint binding, GFXSampler *sampler) {
    for (size_t i = 0; i < _bindingUnits.size(); ++i) {
        GFXBindingUnit &bindingUnit = _bindingUnits[i];
        if (bindingUnit.binding == binding) {
            if (bindingUnit.type == GFXBindingType::SAMPLER) {
                if (bindingUnit.sampler != sampler) {
                    bindingUnit.sampler = sampler;
                    _isDirty = true;
                }
            } else {
                CCASSERT(false, "Setting binding is not GFXBindingType.SAMPLER.");
            }
            return;
        }
    }
}

} // namespace gfx
} // namespace cc

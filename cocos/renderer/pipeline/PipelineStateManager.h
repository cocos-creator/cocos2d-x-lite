#pragma once

#include "core/CoreStd.h"

namespace cc {
struct PSOCreateInfo;

namespace gfx {
class GFXDevice;
class GFXInputAssembler;
class GFXPipelineState;
class GFXRenderPass;
};

namespace pipeline {

class CC_DLL PipelineStateManager {
public:
    static gfx::GFXPipelineState *getOrCreatePipelineStage(gfx::GFXDevice *device,
                                                           const PSOCreateInfo &PSOInfo,
                                                           gfx::GFXRenderPass *renderPass,
                                                           gfx::GFXInputAssembler *inputAssembler);
    
private:
    static gfx::map<uint, gfx::GFXPipelineState *>::type _PSOHashMap;
};

} // namespace pipeline
} // namespace cc

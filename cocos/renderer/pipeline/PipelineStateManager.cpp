#include "PipelineStateManager.h"
#include "gfx/GFXDevice.h"
#include "gfx/GFXInputAssembler.h"
#include "gfx/GFXRenderPass.h"
#include "gfx/GFXShader.h"
#include "helper/SharedMemory.h"

namespace cc {
namespace pipeline {
map<uint, gfx::PipelineState *> PipelineStateManager::_PSOHashMap;
gfx::PipelineState *PipelineStateManager::getOrCreatePipelineState(const PassView *pass,
                                                                   gfx::Shader *shader,
                                                                   gfx::InputAssembler *inputAssembler,
                                                                   gfx::RenderPass *renderPass) {
    const auto passHash = pass->hash;
    const auto renderPassHash = renderPass->getHash();
    const auto iaHash = inputAssembler->getAttributesHash();
    const auto shaderID = shader->getID();
    const auto hash = passHash ^ renderPassHash ^ iaHash ^ shaderID;

    auto pso = _PSOHashMap[hash];
    if (!pso) {
        gfx::PipelineStateInfo info = {
            static_cast<gfx::PrimitiveMode>(pass->primitive),
            shader,
            {inputAssembler->getAttributes()},
            *(GET_RASTERIZER_STATE(pass->rasterizerStateID)),
            *(GET_DEPTH_STENCIL_STATE(pass->depthStencilStateID)),
            *(GET_BLEND_STATE(pass->blendStateID)),
            static_cast<gfx::DynamicStateFlags>(pass->dynamicState),
            renderPass};

        pso = gfx::Device::getInstance()->createPipelineState(std::move(info));
        _PSOHashMap[hash] = pso;
    }

    return pso;
}

} // namespace pipeline
} // namespace cc

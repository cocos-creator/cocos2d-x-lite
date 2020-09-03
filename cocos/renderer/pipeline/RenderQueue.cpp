#include "RenderQueue.h"
#include "PipelineStateManager.h"
#include "gfx/GFXCommandBuffer.h"
#include "gfx/GFXShader.h"
#include "helper/SharedMemory.h"

namespace cc {
namespace pipeline {

RenderQueue::RenderQueue(const RenderQueueCreateInfo &desc)
: _passDesc(desc) {
}

void RenderQueue::clear() {
    _queue.clear();
}

bool RenderQueue::insertRenderPass(const RenderObject &renderObj, uint subModelIdx, uint passIdx) {
    uint32_t *subModels = GET_SUBMODEL_ARRAY(renderObj.model->subModelsID);
    
    const auto subModel = GET_SUBMODEL(subModels[subModelIdx+1]);
    const auto pass = GET_PASS(subModel->pass0ID + passIdx);
    const auto blendState = GET_BLEND_STATE(pass->blendStateID);
    const auto isTransparent = blendState->targets[0].blend;

    if (isTransparent != _passDesc.isTransparent || !(pass->phase & _passDesc.phases)) {
        return false;
    }

    const auto hash = (0 << 30) | (pass->priority << 16) | (subModel->priority << 8) | passIdx;
    RenderPass renderPass = {hash, renderObj.depth, GET_SHADER(subModel->shader0ID + passIdx)->getHash(), passIdx, subModel};
    _queue.emplace_back(std::move(renderPass));
    return true;
}

void RenderQueue::sort() {
    std::sort(_queue.begin(), _queue.end(), _passDesc.sortFunc);
}

void RenderQueue::recordCommandBuffer(gfx::Device *device, gfx::RenderPass *renderPass, gfx::CommandBuffer *cmdBuff) {
    for (size_t i = 0; i < _queue.size(); ++i) {
        const auto subModel = _queue[i].subModel;
        const auto passIdx = _queue[i].passIndex;
        const auto inputAssembler = GET_IA(subModel->inputAssemblerID);

        const auto pass = GET_PASS(subModel->pass0ID + passIdx);
        const auto shader = GET_SHADER(subModel->shader0ID + passIdx);

        auto pso = PipelineStateManager::getOrCreatePipelineState(pass, shader, inputAssembler, renderPass);
        cmdBuff->bindPipelineState(pso);
        cmdBuff->bindDescriptorSet(static_cast<uint>(SetIndex::MATERIAL), GET_DESCRIPTOR_SET(pass->descriptorSetID));
        cmdBuff->bindDescriptorSet(static_cast<uint>(SetIndex::LOCAL), GET_DESCRIPTOR_SET(subModel->descriptorSetID));
        cmdBuff->bindInputAssembler(inputAssembler);
        cmdBuff->draw(inputAssembler);
    }
}

} // namespace pipeline
} // namespace cc

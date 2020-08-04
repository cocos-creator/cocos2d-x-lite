#include "RenderQueue.h"
#include "PipelineStateManager.h"
#include "gfx/GFXCommandBuffer.h"
#include "gfx/GFXShader.h"
#include "helper/Model.h"
#include "helper/Pass.h"
#include "helper/SharedMemoryPool.h"
#include "helper/SubModel.h"

namespace cc {
namespace pipeline {

RenderQueue::RenderQueue(const RenderQueueCreateInfo &desc)
: _passDesc(desc) {
}

void RenderQueue::clear() {
    _queue.clear();
}

bool RenderQueue::insertRenderPass(const RenderObject &renderObj, uint subModelIdx, uint passIdx) {
    const auto subModelPtr = SharedMemory::get<SubModel *>(subModelIdx);
    const auto passPtr = SharedMemory::get<Pass *>(subModelPtr->materialID) + passIdx;
    const auto psoCIPtr = SharedMemory::get<gfx::PipelineStateInfo *>(subModelPtr->psociID) + passIdx;
    const auto isTransparent = psoCIPtr->blendState.targets[0].blend;

    if (isTransparent != _passDesc.isTransparent || !(passPtr->phase & _passDesc.phases)) {
        return false;
    }

    const auto hash = (0 << 30) | (passPtr->priority << 16) | (subModelPtr->priority << 8) | passIdx;
    RenderPass renderPass = {hash, renderObj.depth, psoCIPtr->shader->getHash(), passIdx, subModelPtr};
    _queue.emplace_back(std::move(renderPass));
    return true;
}

void RenderQueue::sort() {
    std::sort(_queue.begin(), _queue.end(), _passDesc.sortFunc);
}

void RenderQueue::recordCommandBuffer(gfx::Device *device, gfx::RenderPass *renderPass, gfx::CommandBuffer *cmdBuff) {
    for (size_t i = 0; i < _queue.size(); ++i) {
        const auto subModelPtr = _queue[i].subModel;
        const auto passIdx = _queue[i].passIndex;
        const auto iaPtr = SharedMemory::get<gfx::InputAssembler *>(subModelPtr->iaID);
        const auto psoCIPtr = SharedMemory::get<gfx::PipelineStateInfo *>(subModelPtr->psociID) + passIdx;
        const auto passPtr = SharedMemory::get<Pass *>(subModelPtr->materialID) + passIdx;
        auto *pso = PipelineStateManager::getOrCreatePipelineStage(psoCIPtr, iaPtr, passPtr->hash, renderPass);
        cmdBuff->bindPipelineState(pso);
        cmdBuff->bindBindingLayout(psoCIPtr->bindingLayout);
        cmdBuff->bindInputAssembler(iaPtr);
        cmdBuff->draw(iaPtr);
    }
}

} // namespace pipeline
} // namespace cc

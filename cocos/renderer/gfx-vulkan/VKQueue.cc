#include "VKStd.h"

#include "VKCommandBuffer.h"
#include "VKCommands.h"
#include "VKDevice.h"
#include "VKFence.h"
#include "VKQueue.h"

namespace cc {
namespace gfx {

CCVKQueue::CCVKQueue(Device *device)
: GFXQueue(device) {
}

CCVKQueue::~CCVKQueue() {
}

bool CCVKQueue::initialize(const GFXQueueInfo &info) {
    _type = info.type;
    _isAsync = !info.forceSync;

    _gpuQueue = CC_NEW(CCVKGPUQueue);
    _gpuQueue->type = _type;
    CCVKCmdFuncGetDeviceQueue((CCVKDevice *)_device, _gpuQueue);

    _status = GFXStatus::SUCCESS;
    return true;
}

void CCVKQueue::destroy() {
    if (_gpuQueue) {
        _gpuQueue->vkQueue = VK_NULL_HANDLE;
        CC_DELETE(_gpuQueue);
        _gpuQueue = nullptr;
    }
    _status = GFXStatus::UNREADY;
}

void CCVKQueue::submit(const vector<CommandBuffer *> &cmdBuffs, Fence *fence) {
    CCVKDevice *device = (CCVKDevice *)_device;

    _gpuQueue->commandBuffers.clear();
    uint count = cmdBuffs.size();
    for (uint i = 0u; i < count; ++i) {
        CCVKCommandBuffer *cmdBuffer = (CCVKCommandBuffer *)cmdBuffs[i];
        _gpuQueue->commandBuffers.push(cmdBuffer->_gpuCommandBuffer->vkCommandBuffer);
        _numDrawCalls += cmdBuffer->_numDrawCalls;
        _numInstances += cmdBuffer->_numInstances;
        _numTriangles += cmdBuffer->_numTriangles;
    }

    VkSubmitInfo submitInfo{VK_STRUCTURE_TYPE_SUBMIT_INFO};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = &_gpuQueue->nextWaitSemaphore;
    submitInfo.pWaitDstStageMask = &_gpuQueue->submitStageMask;
    submitInfo.commandBufferCount = count;
    submitInfo.pCommandBuffers = &_gpuQueue->commandBuffers[0];
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = &_gpuQueue->nextSignalSemaphore;

    if (_isAsync) {
        VkFence vkFence = fence ? ((CCVKFence *)fence)->gpuFence()->vkFence : VK_NULL_HANDLE;
        VK_CHECK(vkQueueSubmit(_gpuQueue->vkQueue, 1, &submitInfo, vkFence));
    } else {
        VkFence vkFence = fence ? ((CCVKFence *)fence)->gpuFence()->vkFence : device->gpuFencePool()->alloc();
        VK_CHECK(vkQueueSubmit(_gpuQueue->vkQueue, 1, &submitInfo, vkFence));
        VK_CHECK(vkWaitForFences(device->gpuDevice()->vkDevice, 1, &vkFence, VK_TRUE, DEFAULT_FENCE_TIMEOUT));
    }

    _gpuQueue->nextWaitSemaphore = _gpuQueue->nextSignalSemaphore;
    _gpuQueue->nextSignalSemaphore = device->gpuSemaphorePool()->alloc();
}

} // namespace gfx
} // namespace cc

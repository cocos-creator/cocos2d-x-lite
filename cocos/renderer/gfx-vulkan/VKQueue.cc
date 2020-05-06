#include "VKStd.h"
#include "VKQueue.h"
#include "VKDevice.h"
#include "VKCommands.h"
#include "VKCommandBuffer.h"

NS_CC_BEGIN

CCVKQueue::CCVKQueue(GFXDevice* device)
    : GFXQueue(device)
{
}

CCVKQueue::~CCVKQueue()
{
}

bool CCVKQueue::initialize(const GFXQueueInfo &info)
{
    _type = info.type;

    _gpuQueue = CC_NEW(CCVKGPUQueue);
    _gpuQueue->type = _type;
    CCVKCmdFuncGetDeviceQueue((CCVKDevice*)_device, _gpuQueue);

    _status = GFXStatus::SUCCESS;
    return true;
}

void CCVKQueue::destroy()
{
    if (_gpuQueue)
    {
        _gpuQueue->vkQueue = VK_NULL_HANDLE;
        CC_DELETE(_gpuQueue);
        _gpuQueue = nullptr;
    }
    _status = GFXStatus::UNREADY;
}

void CCVKQueue::submit(const std::vector<GFXCommandBuffer*>& cmdBuffs)
{
    if (!_isAsync)
    {
        _gpuQueue->commandBuffers.clear();
        uint32_t count = cmdBuffs.size();
        for (uint32_t i = 0u; i < count; ++i)
        {
            CCVKCommandBuffer* cmdBuffer = (CCVKCommandBuffer*)cmdBuffs[i];
            //_gpuQueue->commandBuffers.push(cmdBuffer->_gpuCommandBuffer->vkCommandBuffer);
            _numDrawCalls += cmdBuffer->_numDrawCalls;
            _numInstances += cmdBuffer->_numInstances;
            _numTriangles += cmdBuffer->_numTriangles;
        }

        VkSubmitInfo submitInfo{ VK_STRUCTURE_TYPE_SUBMIT_INFO };
        submitInfo.waitSemaphoreCount = 1;
        submitInfo.pWaitSemaphores = &_gpuQueue->waitSemaphore;
        submitInfo.pWaitDstStageMask = &_gpuQueue->submitStageMask;
        submitInfo.commandBufferCount = _gpuQueue->commandBuffers.size();
        submitInfo.pCommandBuffers = &_gpuQueue->commandBuffers[0];
        submitInfo.signalSemaphoreCount = 1;
        submitInfo.pSignalSemaphores = &_gpuQueue->signalSemaphore;

        VK_CHECK(vkQueueSubmit(_gpuQueue->vkQueue, 1, &submitInfo, VK_NULL_HANDLE));

        _gpuQueue->waitSemaphore = _gpuQueue->signalSemaphore;
        _gpuQueue->signalSemaphore = ((CCVKDevice*)_device)->gpuSemaphorePool()->alloc();
    }
}

NS_CC_END

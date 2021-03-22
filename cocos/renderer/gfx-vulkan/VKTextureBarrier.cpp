/****************************************************************************
 Copyright (c) 2020-2021 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#include "VKStd.h"

#include "VKGPUObjects.h"
#include "VKQueue.h"
#include "VKTextureBarrier.h"

namespace cc {
namespace gfx {

CCVKTextureBarrier::CCVKTextureBarrier()
: TextureBarrier() {
}

CCVKTextureBarrier::~CCVKTextureBarrier() {
    CC_SAFE_DELETE(_gpuBarrier);
}

void CCVKTextureBarrier::doInit(const TextureBarrierInfo &info) {
    _gpuBarrier = CC_NEW(CCVKGPUTextureBarrier);
    _gpuBarrier->accessTypes.resize(info.prevAccesses.size() + info.nextAccesses.size());

    uint index = 0u;
    for (AccessType type : info.prevAccesses) {
        _gpuBarrier->accessTypes[index++] = THSVS_ACCESS_TYPES[(uint)type];
    }
    for (AccessType type : info.nextAccesses) {
        _gpuBarrier->accessTypes[index++] = THSVS_ACCESS_TYPES[(uint)type];
    }

    _gpuBarrier->barrier.prevAccessCount = info.prevAccesses.size();
    _gpuBarrier->barrier.pPrevAccesses   = _gpuBarrier->accessTypes.data();
    _gpuBarrier->barrier.nextAccessCount = info.nextAccesses.size();
    _gpuBarrier->barrier.pNextAccesses   = _gpuBarrier->accessTypes.data() + info.prevAccesses.size();

    _gpuBarrier->barrier.prevLayout = _gpuBarrier->barrier.nextLayout = THSVS_IMAGE_LAYOUT_OPTIMAL;
    _gpuBarrier->barrier.discardContents                              = info.discardContents;
    _gpuBarrier->barrier.subresourceRange.baseMipLevel                = 0u;
    _gpuBarrier->barrier.subresourceRange.levelCount                  = VK_REMAINING_MIP_LEVELS;
    _gpuBarrier->barrier.subresourceRange.baseArrayLayer              = 0u;
    _gpuBarrier->barrier.subresourceRange.layerCount                  = VK_REMAINING_ARRAY_LAYERS;
    _gpuBarrier->barrier.srcQueueFamilyIndex = info.srcQueue
                                                   ? ((CCVKQueue *)info.srcQueue)->gpuQueue()->queueFamilyIndex
                                                   : VK_QUEUE_FAMILY_IGNORED;
    _gpuBarrier->barrier.dstQueueFamilyIndex = info.dstQueue
                                                   ? ((CCVKQueue *)info.dstQueue)->gpuQueue()->queueFamilyIndex
                                                   : VK_QUEUE_FAMILY_IGNORED;

    thsvsGetVulkanImageMemoryBarrier(_gpuBarrier->barrier, &_gpuBarrier->srcStageMask, &_gpuBarrier->dstStageMask, &_gpuBarrier->vkBarrier);
}

} // namespace gfx
} // namespace cc

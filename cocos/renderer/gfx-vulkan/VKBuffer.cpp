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

#include "VKBuffer.h"
#include "VKCommandBuffer.h"
#include "VKCommands.h"
#include "VKDevice.h"

namespace cc {
namespace gfx {

CCVKBuffer::CCVKBuffer(Device *device)
: Buffer(device) {
}

CCVKBuffer::~CCVKBuffer() {
}

void CCVKBuffer::doInit(const BufferInfo &info) {
    _gpuBuffer = CC_NEW(CCVKGPUBuffer);
    _gpuBuffer->usage = _usage;
    _gpuBuffer->memUsage = _memUsage;
    _gpuBuffer->size = _size;
    _gpuBuffer->stride = _stride;
    _gpuBuffer->count = _count;

    if (_usage & BufferUsageBit::INDIRECT) {
        const size_t drawInfoCount = _size / sizeof(DrawInfo);
        _gpuBuffer->indexedIndirectCmds.resize(drawInfoCount);
        _gpuBuffer->indirectCmds.resize(drawInfoCount);
    }

    CCVKCmdFuncCreateBuffer((CCVKDevice *)_device, _gpuBuffer);

    _gpuBufferView = CC_NEW(CCVKGPUBufferView);
    createBufferView();
}

void CCVKBuffer::doInit(const BufferViewInfo &info) {
    CCVKBuffer *buffer = (CCVKBuffer *)info.buffer;
    _gpuBuffer = ((CCVKBuffer *)info.buffer)->gpuBuffer();
    _gpuBufferView = CC_NEW(CCVKGPUBufferView);
    createBufferView();
}

void CCVKBuffer::createBufferView() {
    _gpuBufferView->gpuBuffer = _gpuBuffer;
    _gpuBufferView->offset = _offset;
    _gpuBufferView->range = _size;
    ((CCVKDevice *)_device)->gpuDescriptorHub()->update(_gpuBufferView);
}

void CCVKBuffer::doDestroy() {
    if (_gpuBufferView) {
        ((CCVKDevice *)_device)->gpuDescriptorHub()->disengage(_gpuBufferView);
        CC_DELETE(_gpuBufferView);
        _gpuBufferView = nullptr;
    }

    if (_gpuBuffer) {
        if (!_isBufferView) {
            ((CCVKDevice *)_device)->gpuBufferHub()->erase(_gpuBuffer);
            ((CCVKDevice *)_device)->gpuRecycleBin()->collect(_gpuBuffer);
            ((CCVKDevice *)_device)->gpuBarrierManager()->cancel(_gpuBuffer);
            _device->getMemoryStatus().bufferSize -= _size;
            CC_DELETE(_gpuBuffer);
        }
        _gpuBuffer = nullptr;
    }
}

void CCVKBuffer::doResize(uint size) {
    ((CCVKDevice *)_device)->gpuRecycleBin()->collect(_gpuBuffer);

    _gpuBuffer->size = _size;
    _gpuBuffer->count = _count;
    CCVKCmdFuncCreateBuffer((CCVKDevice *)_device, _gpuBuffer);

    createBufferView();

    if (_usage & BufferUsageBit::INDIRECT) {
        const size_t drawInfoCount = _size / sizeof(DrawInfo);
        _gpuBuffer->indexedIndirectCmds.resize(drawInfoCount);
        _gpuBuffer->indirectCmds.resize(drawInfoCount);
    }
}

void CCVKBuffer::update(void *buffer, uint size) {
    CCASSERT(!_isBufferView, "Cannot update through buffer views");

#if CC_DEBUG > 0
    if (_usage & BufferUsageBit::INDIRECT) {
        DrawInfo *drawInfo = static_cast<DrawInfo *>(buffer);
        const size_t drawInfoCount = size / sizeof(DrawInfo);
        const bool isIndexed = drawInfoCount > 0 && drawInfo->indexCount > 0;
        for (size_t i = 1u; i < drawInfoCount; i++) {
            if ((++drawInfo)->indexCount > 0 != isIndexed) {
                CC_LOG_WARNING("Inconsistent indirect draw infos on using index buffer");
                return;
            }
        }
    }
#endif

    if (_buffer) {
        memcpy(_buffer, buffer, size);
    }

    ((CCVKDevice *)_device)->gpuTransportHub()->checkIn([this, buffer, size](CCVKGPUCommandBuffer *gpuCommandBuffer) {
        CCVKCmdFuncUpdateBuffer((CCVKDevice *)_device, _gpuBuffer, buffer, size, gpuCommandBuffer);
    });
}

} // namespace gfx
} // namespace cc

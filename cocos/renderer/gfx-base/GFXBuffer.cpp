/****************************************************************************
 Copyright (c) 2019-2021 Xiamen Yaji Software Co., Ltd.

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

#include "base/CoreStd.h"

#include "GFXBuffer.h"
#include "GFXDevice.h"
#include "GFXObject.h"

namespace cc {
namespace gfx {

Buffer::Buffer(Device *device)
: GFXObject(ObjectType::BUFFER),
  _device(device) {
}

Buffer::~Buffer() {
}

uint Buffer::computeHash(const BufferInfo &info) {
    uint seed = 4;
    seed ^= (uint)(info.usage) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= (uint)(info.memUsage) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= (uint)(info.size) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= (uint)(info.flags) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
    return seed;
}

void Buffer::initialize(const BufferInfo &info) {
    _usage    = info.usage;
    _memUsage = info.memUsage;
    _size     = info.size;
    _flags    = info.flags;
    _stride   = std::max(info.stride, 1u);
    _count    = _size / _stride;

    _device->getMemoryStatus().bufferSize += _size;

    doInit(info);
}

void Buffer::initialize(const BufferViewInfo &info) {
    _usage    = info.buffer->getUsage();
    _memUsage = info.buffer->getMemUsage();
    _flags    = info.buffer->getFlags();
    _offset   = info.offset;
    _size = _stride = info.range;
    _count          = 1u;
    _isBufferView   = true;

    doInit(info);
}

void Buffer::resize(uint size) {
    CCASSERT(!_isBufferView, "Cannot resize buffer views");

    if (size != _size) {
        doResize(size);
        _device->getMemoryStatus().bufferSize -= _size;

        _size  = size;
        _count = size / _stride;

        _device->getMemoryStatus().bufferSize += _size;
    }
}

void Buffer::destroy() {
    doDestroy();

    _device->getMemoryStatus().bufferSize -= _size;

    _offset = _size = _stride = _count = 0u;
}

} // namespace gfx
} // namespace cc

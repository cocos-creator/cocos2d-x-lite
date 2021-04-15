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

#pragma once

#include "PoolType.h"
#include "cocos/base/Macros.h"
#include "cocos/base/Object.h"
#include "cocos/base/TypeDef.h"
#include "cocos/base/memory/StlAlloc.h"
#include "cocos/bindings/jswrapper/Object.h"

namespace se {

class CC_DLL BufferAllocator final : public cc::Object {
public:
    template <class T>
    static T *getBuffer(PoolType type, uint index) {
        index &= _bufferMask;
        const auto p = GET_ARRAY_POOL_ID(type);

#ifdef CC_DEBUG
        CCASSERT(BufferAllocator::_pools[p] != nullptr, "BufferPool: Invalid buffer pool type");
#endif

        const auto pool = BufferAllocator::_pools[p];

#ifdef CC_DEBUG
        CCASSERT(pool->_caches.count(index) != 0, "BufferPool: Invalid buffer pool index");
#endif

        return reinterpret_cast<T *>(pool->_caches[index]);
    }

    template <class T>
    static T *getBuffer(PoolType type, uint index, uint *size) {
        index &= _bufferMask;
        const auto p = GET_ARRAY_POOL_ID(type);

#ifdef CC_DEBUG
        CCASSERT(BufferAllocator::_pools[p] != nullptr, "BufferPool: Invalid buffer pool type");
#endif

        const auto pool = BufferAllocator::_pools[p];

#ifdef CC_DEBUG
        CCASSERT(pool->_buffers.count(index) != 0, "BufferPool: Invalid buffer pool index");
#endif

        T *    ret = nullptr;
        size_t len;
        pool->_buffers[index]->getArrayBufferData((uint8_t **)&ret, &len);
        *size = (uint)len;
        return ret;
    }

    BufferAllocator(PoolType type);
    ~BufferAllocator();

    Object *alloc(uint index, uint bytes);
    void    free(uint index);

private:
    static cc::vector<BufferAllocator *> _pools;
    static constexpr uint                _bufferMask = ~(1 << 30);

    cc::map<uint, Object *>  _buffers;
    cc::map<uint, uint8_t *> _caches;
    PoolType                 _type = PoolType::UNKNOWN;
};

} // namespace se

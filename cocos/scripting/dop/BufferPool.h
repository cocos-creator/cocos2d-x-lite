/****************************************************************************
Copyright (c) 2020 Xiamen Yaji Software Co., Ltd.

http://www.cocos.com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/

#pragma once

#include "cocos/base/Object.h"
#include "cocos/base/memory/StlAlloc.h"
#include "cocos/scripting/js-bindings/jswrapper/Object.h"
#include "cocos/base/ccMacros.h"
#include "cocos/base/TypeDef.h"

namespace se {

class CC_DLL BufferPool final : public cc::Object {
public:
    using Chunk = uint8_t *;

    enum class Type {
        // objects
        RASTERIZER_STATE,
        DEPTH_STENCIL_STATE,
        BLEND_STATE,
        BINDING_LAYOUT,
        SHADER,
        // buffers
        PASS_INFO,
        PSOCI,
        MODEL_INFO,
        SUBMODEL_INFO,
        INPUT_ASSEMBLER_INFO
    };

    CC_INLINE static const cc::map<Type, BufferPool *> &getPoolMap() { return BufferPool::_poolMap; }

    BufferPool(Type type, uint entryBits, uint bytesPerEntry);
    ~BufferPool();

    template <class T>
    T *getTypedObject(uint id) {
        uint chunk = (_chunkMask & id) >> _entryBits;
        uint entry = _entryMask & id;
        CCASSERT(chunk < _chunks.size() && entry < _entriesPerChunk, "BufferPool: Invalid buffer pool entry id");
        return reinterpret_cast<T *>(_chunks[chunk][entry]);
    }

    Object *allocateNewChunk();

private:
    static cc::map<Type, BufferPool *> _poolMap;

    cc::vector<Chunk> _chunks;
    cc::map<Chunk, Object *> _jsObjs;
    const uint _poolFlag = 1 << 30;
    uint _entryBits = 1 << 8;
    uint _chunkMask = 0;
    uint _entryMask = 0;
    uint _bytesPerChunk = 0;
    uint _entriesPerChunk = 0;
    uint _bytesPerEntry = 0;
    Type _type = Type::RASTERIZER_STATE;
};

} // namespace se

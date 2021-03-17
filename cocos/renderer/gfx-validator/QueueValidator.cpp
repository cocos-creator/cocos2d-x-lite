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

#include "base/CoreStd.h"

#include "CommandBufferValidator.h"
#include "DeviceValidator.h"
#include "QueueValidator.h"

namespace cc {
namespace gfx {

QueueValidator::~QueueValidator() {
    CC_SAFE_DELETE(_actor);
}

void QueueValidator::doInit(const QueueInfo &info) {
    _actor->initialize(info);
}

void QueueValidator::doDestroy() {
    _actor->destroy();
}

void QueueValidator::submit(CommandBuffer *const *cmdBuffs, uint count) {
    if (!count) return;

    static vector<CommandBuffer *> cmdBuffActors;
    cmdBuffActors.resize(count);

    for (uint i = 0u; i < count; ++i) {
        cmdBuffActors[i] = ((CommandBufferValidator *)cmdBuffs[i])->getActor();
    }

    _actor->submit(cmdBuffActors.data(), count);
}

} // namespace gfx
} // namespace cc

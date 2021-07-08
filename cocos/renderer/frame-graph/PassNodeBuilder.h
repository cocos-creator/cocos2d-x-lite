/****************************************************************************
 Copyright (c) 2021 Xiamen Yaji Software Co., Ltd.

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

#include "PassNode.h"

namespace cc {
namespace framegraph {

class PassNodeBuilder final {
public:
    PassNodeBuilder(FrameGraph &graph, PassNode &passNode) noexcept;
    PassNodeBuilder()                        = delete;
    ~PassNodeBuilder()                       = default;
    PassNodeBuilder(const PassNodeBuilder &) = delete;
    PassNodeBuilder(PassNodeBuilder &&)      = delete;
    PassNodeBuilder &operator=(const PassNodeBuilder &) = delete;
    PassNodeBuilder &operator=(PassNodeBuilder &&) = delete;

    template <typename ResourceType>
    void create(TypedHandle<ResourceType> &handle, const StringHandle &name, const typename ResourceType::Descriptor &desc) const noexcept;
    template <typename ResourceType>
    void importExternal(TypedHandle<ResourceType> &handle, const StringHandle &name, ResourceType &resource) const noexcept;
    template <typename ResourceType>
    TypedHandle<ResourceType> read(TypedHandle<ResourceType> const &input) const noexcept;
    Handle                    read(const Handle &input) const noexcept;
    template <typename ResourceType>
    TypedHandle<ResourceType> write(TypedHandle<ResourceType> const &output) const noexcept;
    TextureHandle             write(const TextureHandle &output, uint8_t mipmapLevel, uint8_t faceId, uint8_t arrayPosition, const RenderTargetAttachment::Descriptor &attachmentDesc) const noexcept;
    TextureHandle             write(const TextureHandle &output, const RenderTargetAttachment::Descriptor &attachmentDesc) const noexcept;

    inline void sideEffect() const noexcept;
    inline void subpass(bool clearActionIgnoreable, bool end) const noexcept;
    inline void setViewport(const gfx::Viewport &viewport, const gfx::Rect &scissor) noexcept;

    void   writeToBlackboard(const StringHandle &name, const Handle &handle) const noexcept;
    Handle readFromBlackboard(const StringHandle &name) const noexcept;

private:
    FrameGraph &_graph;
    PassNode &  _passNode;
};

//////////////////////////////////////////////////////////////////////////

template <typename ResourceType>
TypedHandle<ResourceType> PassNodeBuilder::read(TypedHandle<ResourceType> const &input) const noexcept {
    return TypedHandle<ResourceType>(read(static_cast<const Handle &>(input)));
}

template <typename ResourceType>
TypedHandle<ResourceType> PassNodeBuilder::write(TypedHandle<ResourceType> const &output) const noexcept {
    return TypedHandle<ResourceType>(_passNode.write(_graph, output));
}

void PassNodeBuilder::sideEffect() const noexcept {
    _passNode.sideEffect();
}

void PassNodeBuilder::subpass(bool clearActionIgnoreable, bool end) const noexcept {
    _passNode.subpass(clearActionIgnoreable, end);
}

void PassNodeBuilder::setViewport(const gfx::Viewport &viewport, const gfx::Rect &scissor) noexcept {
    _passNode.setViewport(viewport, scissor);
}

} // namespace framegraph
} // namespace cc

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
#include "base/threading/MessageQueue.h"

#include "DeviceValidator.h"
#include "TextureValidator.h"

namespace cc {
namespace gfx {

namespace {
unordered_map<Format, Feature> featureCheckMap{
    {Format::D16, Feature::FORMAT_D16},
    {Format::D16S8, Feature::FORMAT_D16S8},
    {Format::D24, Feature::FORMAT_D24},
    {Format::D24S8, Feature::FORMAT_D24S8},
    {Format::D32F, Feature::FORMAT_D32F},
    {Format::D32F_S8, Feature::FORMAT_D32FS8},
    {Format::RGB8, Feature::FORMAT_RGB8},
};
}

TextureValidator::~TextureValidator() {
    CC_SAFE_DELETE(_actor);
}

void TextureValidator::doInit(const TextureInfo &info) {
    CCASSERT(DeviceValidator::getInstance()->hasFeature(featureCheckMap[_format]), "unsupported format");

    _actor->initialize(info);
}

void TextureValidator::doInit(const TextureViewInfo &info) {
    TextureViewInfo actorInfo = info;
    actorInfo.texture         = ((TextureValidator *)info.texture)->getActor();

    _actor->initialize(info);
}

void TextureValidator::doDestroy() {
    _actor->destroy();
}

void TextureValidator::doResize(uint width, uint height, uint size) {
    CCASSERT(!_isTextureView, "Cannot resize texture views");

    _actor->resize(width, height);
}

} // namespace gfx
} // namespace cc

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

#include "RenderPipeline.h"
#include "PipelineStateManager.h"
#include "RenderFlow.h"
#include "gfx-base/GFXCommandBuffer.h"
#include "gfx-base/GFXDescriptorSet.h"
#include "gfx-base/GFXDescriptorSetLayout.h"
#include "gfx-base/GFXDevice.h"
#include "gfx-base/GFXTexture.h"

namespace cc {
namespace pipeline {

RenderPipeline *RenderPipeline::instance = nullptr;

RenderPipeline *RenderPipeline::getInstance() {
    return RenderPipeline::instance;
}

RenderPipeline::RenderPipeline()
: _device(gfx::Device::getInstance()) {
    RenderPipeline::instance = this;

    setDescriptorSetLayout();
    generateConstantMacros();

    _pipelineUBO       = new PipelineUBO();
    _pipelineSceneData = new PipelineSceneData();
}

RenderPipeline::~RenderPipeline() {
    RenderPipeline::instance = nullptr;
}

#define INIT_GLOBAL_DESCSET_LAYOUT(info)                                      \
    do {                                                                      \
        globalDescriptorSetLayout.samplers[info::NAME]    = info::LAYOUT;     \
        globalDescriptorSetLayout.bindings[info::BINDING] = info::DESCRIPTOR; \
    } while (0)

void RenderPipeline::setDescriptorSetLayout() {
    globalDescriptorSetLayout.bindings.resize(static_cast<size_t>(PipelineGlobalBindings::COUNT));

    globalDescriptorSetLayout.blocks[UBOGlobal::NAME]            = UBOGlobal::LAYOUT;
    globalDescriptorSetLayout.bindings[UBOGlobal::BINDING]       = UBOGlobal::DESCRIPTOR;
    globalDescriptorSetLayout.blocks[UBOCamera::NAME]            = UBOCamera::LAYOUT;
    globalDescriptorSetLayout.bindings[UBOCamera::BINDING]       = UBOCamera::DESCRIPTOR;
    globalDescriptorSetLayout.blocks[UBOShadow::NAME]            = UBOShadow::LAYOUT;
    globalDescriptorSetLayout.bindings[UBOShadow::BINDING]       = UBOShadow::DESCRIPTOR;
    globalDescriptorSetLayout.samplers[SHADOWMAP::NAME]          = SHADOWMAP::LAYOUT;
    globalDescriptorSetLayout.bindings[SHADOWMAP::BINDING]       = SHADOWMAP::DESCRIPTOR;
    globalDescriptorSetLayout.samplers[ENVIRONMENT::NAME]        = ENVIRONMENT::LAYOUT;
    globalDescriptorSetLayout.bindings[ENVIRONMENT::BINDING]     = ENVIRONMENT::DESCRIPTOR;
    globalDescriptorSetLayout.samplers[SPOTLIGHTINGMAP::NAME]    = SPOTLIGHTINGMAP::LAYOUT;
    globalDescriptorSetLayout.bindings[SPOTLIGHTINGMAP::BINDING] = SPOTLIGHTINGMAP::DESCRIPTOR;

    INIT_GLOBAL_DESCSET_LAYOUT(SAMPLERGBUFFERALBEDOMAP);
    INIT_GLOBAL_DESCSET_LAYOUT(SAMPLERGBUFFERPOSITIONMAP);
    INIT_GLOBAL_DESCSET_LAYOUT(SAMPLERGBUFFEREMISSIVEMAP);
    INIT_GLOBAL_DESCSET_LAYOUT(SAMPLERGBUFFERNORMALMAP);
    INIT_GLOBAL_DESCSET_LAYOUT(SAMPLERLIGHTINGRESULTMAP);

    localDescriptorSetLayout.bindings.resize(static_cast<size_t>(ModelLocalBindings::COUNT));
    localDescriptorSetLayout.blocks[UBOLocalBatched::NAME]           = UBOLocalBatched::LAYOUT;
    localDescriptorSetLayout.bindings[UBOLocalBatched::BINDING]      = UBOLocalBatched::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOLocal::NAME]                  = UBOLocal::LAYOUT;
    localDescriptorSetLayout.bindings[UBOLocal::BINDING]             = UBOLocal::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOForwardLight::NAME]           = UBOForwardLight::LAYOUT;
    localDescriptorSetLayout.bindings[UBOForwardLight::BINDING]      = UBOForwardLight::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOSkinningTexture::NAME]        = UBOSkinningTexture::LAYOUT;
    localDescriptorSetLayout.bindings[UBOSkinningTexture::BINDING]   = UBOSkinningTexture::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOSkinningAnimation::NAME]      = UBOSkinningAnimation::LAYOUT;
    localDescriptorSetLayout.bindings[UBOSkinningAnimation::BINDING] = UBOSkinningAnimation::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOSkinning::NAME]               = UBOSkinning::LAYOUT;
    localDescriptorSetLayout.bindings[UBOSkinning::BINDING]          = UBOSkinning::DESCRIPTOR;
    localDescriptorSetLayout.blocks[UBOMorph::NAME]                  = UBOMorph::LAYOUT;
    localDescriptorSetLayout.bindings[UBOMorph::BINDING]             = UBOMorph::DESCRIPTOR;
    localDescriptorSetLayout.samplers[JOINTTEXTURE::NAME]            = JOINTTEXTURE::LAYOUT;
    localDescriptorSetLayout.bindings[JOINTTEXTURE::BINDING]         = JOINTTEXTURE::DESCRIPTOR;
    localDescriptorSetLayout.samplers[POSITIONMORPH::NAME]           = POSITIONMORPH::LAYOUT;
    localDescriptorSetLayout.bindings[POSITIONMORPH::BINDING]        = POSITIONMORPH::DESCRIPTOR;
    localDescriptorSetLayout.samplers[NORMALMORPH::NAME]             = NORMALMORPH::LAYOUT;
    localDescriptorSetLayout.bindings[NORMALMORPH::BINDING]          = NORMALMORPH::DESCRIPTOR;
    localDescriptorSetLayout.samplers[TANGENTMORPH::NAME]            = TANGENTMORPH::LAYOUT;
    localDescriptorSetLayout.bindings[TANGENTMORPH::BINDING]         = TANGENTMORPH::DESCRIPTOR;
    localDescriptorSetLayout.samplers[LIGHTMAPTEXTURE::NAME]         = LIGHTMAPTEXTURE::LAYOUT;
    localDescriptorSetLayout.bindings[LIGHTMAPTEXTURE::BINDING]      = LIGHTMAPTEXTURE::DESCRIPTOR;
    localDescriptorSetLayout.samplers[SPRITETEXTURE::NAME]           = SPRITETEXTURE::LAYOUT;
    localDescriptorSetLayout.bindings[SPRITETEXTURE::BINDING]        = SPRITETEXTURE::DESCRIPTOR;
}

bool RenderPipeline::initialize(const RenderPipelineInfo &info) {
    _flows = info.flows;
    _tag   = info.tag;
    return true;
}

bool RenderPipeline::activate() {
    if (_descriptorSetLayout) {
        _descriptorSetLayout->destroy();
        CC_DELETE(_descriptorSetLayout);
    }
    _descriptorSetLayout = _device->createDescriptorSetLayout({globalDescriptorSetLayout.bindings});

    if (_descriptorSet) {
        _descriptorSet->destroy();
        CC_DELETE(_descriptorSet);
    }
    _descriptorSet = _device->createDescriptorSet({_descriptorSetLayout});
    _pipelineUBO->activate(_device, this);
    _pipelineSceneData->activate(_device, this);

    for (auto *const flow : _flows) {
        flow->activate(this);
    }

    // has not initBuiltinRes,
    // create temporary default Texture to binding sampler2d
    if (!_defaultTexture) {
        _defaultTexture = _device->createTexture({
            gfx::TextureType::TEX2D,
            gfx::TextureUsageBit::COLOR_ATTACHMENT | gfx::TextureUsageBit::SAMPLED,
            gfx::Format::RGBA8,
            1U,
            1U,
        });
    }

    return true;
}

void RenderPipeline::render(const vector<uint> &/*cameras*/, const vector<scene::Camera *> &cameras) {
    for (auto *const flow : _flows) {
        for (auto *camera : cameras) {
            flow->render(nullptr, camera);
        }
    }
}

void RenderPipeline::destroy() {
    for (auto *flow : _flows) {
        flow->destroy();
    }
    _flows.clear();

    CC_SAFE_DESTROY(_descriptorSetLayout);
    CC_SAFE_DESTROY(_descriptorSet);
    CC_SAFE_DESTROY(_pipelineUBO);
    CC_SAFE_DESTROY(_pipelineSceneData);

    for (auto *const cmdBuffer : _commandBuffers) {
        cmdBuffer->destroy();
    }
    _commandBuffers.clear();

    CC_SAFE_DESTROY(_defaultTexture);

    CC_SAFE_DELETE(_defaultTexture);

    SamplerLib::destroyAll();
    PipelineStateManager::destroyAll();
}

void RenderPipeline::setPipelineSharedSceneData(scene::PipelineSharedSceneData *data) {
    _pipelineSceneData->setPipelineSharedSceneData(data);
}

void RenderPipeline::generateConstantMacros() {
    _constantMacros = StringUtil::format(
        R"(
#define CC_DEVICE_SUPPORT_FLOAT_TEXTURE %d
#define CC_DEVICE_MAX_VERTEX_UNIFORM_VECTORS %d
#define CC_DEVICE_MAX_FRAGMENT_UNIFORM_VECTORS %d
        )",
        _device->hasFeature(gfx::Feature::TEXTURE_FLOAT) ? 1 : 0,
        _device->getCapabilities().maxVertexUniformVectors,
        _device->getCapabilities().maxFragmentUniformVectors);
}

} // namespace pipeline
} // namespace cc

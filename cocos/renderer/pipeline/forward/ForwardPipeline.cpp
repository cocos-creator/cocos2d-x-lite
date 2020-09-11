#include "ForwardPipeline.h"
#include "../RenderView.h"
#include "../helper/SharedMemory.h"
#include "../shadow/ShadowFlow.h"
#include "../ui/UIFlow.h"
#include "ForwardFlow.h"
#include "SceneCulling.h"
#include "cocos/math/Mat4.h"
#include "gfx/GFXBuffer.h"
#include "gfx/GFXDescriptorSet.h"
#include "gfx/GFXDevice.h"
#include "gfx/GFXRenderPass.h"
#include "platform/Application.h"

namespace cc {
namespace pipeline {
namespace {
float UNIT_Z[3] = {0, 0, 1.0f};
float ZERO[4] = {0, 0, 0, 0};
#define TO_VEC3(dst, src, offset) \
    dst[offset] = src[0];         \
    dst[offset + 1] = src[1];     \
    dst[offset + 2] = src[2];
#define TO_VEC4(dst, src, offset) \
    dst[offset] = src[0];         \
    dst[offset + 1] = src[1];     \
    dst[offset + 2] = src[2];     \
    dst[offset + 3] = src[3];
} // namespace

gfx::RenderPass *ForwardPipeline::getOrCreateRenderPass(gfx::ClearFlags clearFlags) {
    if (_renderPasses.find(clearFlags) != _renderPasses.end()) {
        return _renderPasses[clearFlags];
    }

    auto device = gfx::Device::getInstance();
    gfx::ColorAttachment colorAttachment;
    gfx::DepthStencilAttachment depthStencilAttachment;
    colorAttachment.format = device->getColorFormat();
    depthStencilAttachment.format = device->getDepthStencilFormat();

    if (!(clearFlags & gfx::ClearFlagBit::COLOR)) {
        if (clearFlags & static_cast<gfx::ClearFlagBit>(SKYBOX_FLAG)) {
            colorAttachment.loadOp = gfx::LoadOp::DISCARD;
        } else {
            colorAttachment.loadOp = gfx::LoadOp::LOAD;
            colorAttachment.beginLayout = gfx::TextureLayout::PRESENT_SRC;
        }
    }

    if (static_cast<gfx::ClearFlagBit>(clearFlags & gfx::ClearFlagBit::DEPTH_STENCIL) != gfx::ClearFlagBit::DEPTH_STENCIL) {
        if (!(clearFlags & gfx::ClearFlagBit::DEPTH)) depthStencilAttachment.depthLoadOp = gfx::LoadOp::LOAD;
        if (!(clearFlags & gfx::ClearFlagBit::STENCIL)) depthStencilAttachment.stencilLoadOp = gfx::LoadOp::LOAD;
        depthStencilAttachment.beginLayout = gfx::TextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    }

    auto renderPass = device->createRenderPass({
        {colorAttachment},
        depthStencilAttachment,
    });
    _renderPasses[clearFlags] = renderPass;

    return renderPass;
}

ForwardPipeline::ForwardPipeline() {
}

ForwardPipeline::~ForwardPipeline() {
    destroy();
}

bool ForwardPipeline::initialize(const RenderPipelineInfo &info) {
    RenderPipeline::initialize(info);

    if (_flows.size() == 0) {
        //TODO coulsonwang
        //        auto shadowFlow = CC_NEW(ShadowFlow);
        //        shadowFlow->initialize(ShadowFlow::getInitializeInfo());
        //        _flows.emplace_back(shadowFlow);

        auto forwardFlow = CC_NEW(ForwardFlow);
        forwardFlow->initialize(ForwardFlow::getInitializeInfo());
        _flows.emplace_back(forwardFlow);

        auto uiFlow = CC_NEW(UIFlow);
        uiFlow->initialize(UIFlow::getInitializeInfo());
        _flows.emplace_back(uiFlow);
    }

    return true;
}

bool ForwardPipeline::activate() {
    if (!RenderPipeline::activate()) {
        CC_LOG_ERROR("RenderPipeline active failed.");
        return false;
    }

    if (!activeRenderer()) {
        CC_LOG_ERROR("ForwardPipeline startup failed!");
        return false;
    }

    return true;
}

void ForwardPipeline::render(const vector<RenderView *> &views) {
    for (const auto view : views) {
        sceneCulling(this, view);
        for (const auto flow : view->getFlows()) {
            flow->render(view);
        }
    }
}

void ForwardPipeline::updateUBOs(RenderView *view) {
    updateUBO(view);
    const auto scene = GET_SCENE(view->getCamera()->getSceneID());
    const auto mainLight = GET_MAIN_LIGHT(scene->mainLightID);
    const auto shadowInfo = _shadows;

    if (mainLight && shadowInfo->isEnabled()) {
        const auto node = GET_NODE(mainLight->getNodeID());

        // shadow view
        auto shadowView = cc::Mat4(node->getWorldMatrix()).getInversed();

        // shadow view proj
        const auto x = shadowInfo->getOrthoSize() * shadowInfo->getAspect();
        const auto y = shadowInfo->getOrthoSize();
        const auto projectionSinY = _device->getScreenSpaceSignY() * _device->getUVSpaceSignY();
        Mat4 shadowViewProj;
        Mat4::createOrthographicOffCenter(-x, x, -y, y, shadowInfo->getNearValue(), shadowInfo->getFarValue(), _device->getClipSpaceMinZ(), projectionSinY, &shadowViewProj);

        shadowViewProj.multiply(shadowView);
        memcpy(_shadowUBO.data() + UBOShadow::MAT_LIGHT_VIEW_PROJ_OFFSET, shadowViewProj.m, sizeof(shadowViewProj));
    }

    // update ubos
    _descriptorSet->getBuffer(UBOGlobal::BLOCK.binding)->update(_globalUBO.data(), 0, static_cast<uint>(_globalUBO.size() * sizeof(float)));
    _descriptorSet->getBuffer(UBOShadow::BLOCK.binding)->update(_shadowUBO.data(), 0, static_cast<uint>(_shadowUBO.size() * sizeof(float)));
}

void ForwardPipeline::updateUBO(RenderView *view) {
    _descriptorSet->update();

    const auto root = GET_ROOT(0);

    const auto camera = view->getCamera();
    const auto scene = GET_SCENE(camera->getSceneID());

    const auto mainLight = GET_MAIN_LIGHT(scene->mainLightID);
    const auto ambient = GET_AMBIENT(scene->ambientID);
    const auto fog = GET_FOG(scene->fogID);
    auto &uboGlobalView = _globalUBO;

    const auto shadingWidth = std::floor(_device->getWidth());
    const auto shadingHeight = std::floor(_device->getHeight());

    // update UBOGlobal
    uboGlobalView[UBOGlobal::TIME_OFFSET] = root->cumulativeTime;
    uboGlobalView[UBOGlobal::TIME_OFFSET + 1] = root->frameTime;
    uboGlobalView[UBOGlobal::TIME_OFFSET + 2] = Application::getInstance()->getTotalFrames();

    uboGlobalView[UBOGlobal::SCREEN_SIZE_OFFSET] = _device->getWidth();
    uboGlobalView[UBOGlobal::SCREEN_SIZE_OFFSET + 1] = _device->getHeight();
    uboGlobalView[UBOGlobal::SCREEN_SIZE_OFFSET + 2] = 1.0f / uboGlobalView[UBOGlobal::SCREEN_SIZE_OFFSET];
    uboGlobalView[UBOGlobal::SCREEN_SIZE_OFFSET + 3] = 1.0f / uboGlobalView[UBOGlobal::SCREEN_SIZE_OFFSET + 1];

    uboGlobalView[UBOGlobal::SCREEN_SCALE_OFFSET] = camera->getWidth() / shadingWidth * _shadingScale;
    uboGlobalView[UBOGlobal::SCREEN_SCALE_OFFSET + 1] = camera->getHeight() / shadingHeight * _shadingScale;
    uboGlobalView[UBOGlobal::SCREEN_SCALE_OFFSET + 2] = 1.0 / uboGlobalView[UBOGlobal::SCREEN_SCALE_OFFSET];
    uboGlobalView[UBOGlobal::SCREEN_SCALE_OFFSET + 3] = 1.0 / uboGlobalView[UBOGlobal::SCREEN_SCALE_OFFSET + 1];

    uboGlobalView[UBOGlobal::NATIVE_SIZE_OFFSET] = shadingWidth;
    uboGlobalView[UBOGlobal::NATIVE_SIZE_OFFSET + 1] = shadingHeight;
    uboGlobalView[UBOGlobal::NATIVE_SIZE_OFFSET + 2] = 1.0f / uboGlobalView[UBOGlobal::NATIVE_SIZE_OFFSET];
    uboGlobalView[UBOGlobal::NATIVE_SIZE_OFFSET + 3] = 1.0f / uboGlobalView[UBOGlobal::NATIVE_SIZE_OFFSET + 1];

    memcpy(uboGlobalView.data() + UBOGlobal::MAT_VIEW_OFFSET, camera->getMatView(), sizeof(cc::Mat4));
    memcpy(uboGlobalView.data() + UBOGlobal::MAT_VIEW_INV_OFFSET, GET_NODE(camera->getNodeID())->getWorldMatrix(), sizeof(cc::Mat4));
    memcpy(uboGlobalView.data() + UBOGlobal::MAT_PROJ_OFFSET, camera->getMatProj(), sizeof(cc::Mat4));
    memcpy(uboGlobalView.data() + UBOGlobal::MAT_PROJ_INV_OFFSET, camera->getMatProjInv(), sizeof(cc::Mat4));
    memcpy(uboGlobalView.data() + UBOGlobal::MAT_VIEW_PROJ_OFFSET, camera->getMatViewProj(), sizeof(cc::Mat4));
    memcpy(uboGlobalView.data() + UBOGlobal::MAT_VIEW_PROJ_INV_OFFSET, camera->getMatViewProjInv(), sizeof(cc::Mat4));
    TO_VEC3(uboGlobalView, camera->getPosition(), UBOGlobal::CAMERA_POS_OFFSET);

    auto projectionSignY = _device->getScreenSpaceSignY();
    if (view->getWindow()->hasOffScreenAttachments) {
        projectionSignY *= _device->getUVSpaceSignY(); // need flipping if drawing on render targets
    }
    uboGlobalView[UBOGlobal::CAMERA_POS_OFFSET + 3] = projectionSignY;

    const auto exposure = camera->getExposure();
    uboGlobalView[UBOGlobal::EXPOSURE_OFFSET] = exposure;
    uboGlobalView[UBOGlobal::EXPOSURE_OFFSET + 1] = 1.0f / exposure;
    uboGlobalView[UBOGlobal::EXPOSURE_OFFSET + 2] = _isHDR ? 1.0f : 0.0;
    uboGlobalView[UBOGlobal::EXPOSURE_OFFSET + 3] = _fpScale / exposure;

    if (mainLight) {
        TO_VEC3(uboGlobalView, mainLight->getDirection(), UBOGlobal::MAIN_LIT_DIR_OFFSET);
        TO_VEC3(uboGlobalView, mainLight->getColor(), UBOGlobal::MAIN_LIT_COLOR_OFFSET);
        if (mainLight->getUseColorTemperature()) {
            const auto colorTempRGB = mainLight->getColorTemperatureRGB();
            uboGlobalView[UBOGlobal::MAIN_LIT_COLOR_OFFSET] *= colorTempRGB[X];
            uboGlobalView[UBOGlobal::MAIN_LIT_COLOR_OFFSET + 1] *= colorTempRGB[Y];
            uboGlobalView[UBOGlobal::MAIN_LIT_COLOR_OFFSET + 2] *= colorTempRGB[Z];
        }

        if (_isHDR) {
            uboGlobalView[UBOGlobal::MAIN_LIT_COLOR_OFFSET + 3] = mainLight->getIlluminance() * _fpScale;
        } else {
            uboGlobalView[UBOGlobal::MAIN_LIT_COLOR_OFFSET + 3] = mainLight->getIlluminance() * exposure;
        }
    } else {

        TO_VEC3(uboGlobalView, UNIT_Z, UBOGlobal::MAIN_LIT_DIR_OFFSET);
        TO_VEC4(uboGlobalView, ZERO, UBOGlobal::MAIN_LIT_COLOR_OFFSET);
    }

    //TODO coulsonwang
    //    auto skyColor = ambient->skyColor;
    //    if (_isHDR) {
    //        skyColor.w = ambient->skyIllum * _fpScale;
    //    } else {
    //        skyColor.w = ambient->skyIllum * exposure;
    //    }
    //    TO_VEC4(uboGlobalView, skyColor, UBOGlobal::AMBIENT_SKY_OFFSET);
    //    TO_VEC4(uboGlobalView, ambient->groundAlbedo, UBOGlobal::AMBIENT_GROUND_OFFSET);
    //
    //    if (fog->enabled) {
    //        TO_VEC4(uboGlobalView, fog->fogColor, UBOGlobal::GLOBAL_FOG_COLOR_OFFSET);
    //
    //        uboGlobalView[UBOGlobal::GLOBAL_FOG_BASE_OFFSET] = fog->fogStart;
    //        uboGlobalView[UBOGlobal::GLOBAL_FOG_BASE_OFFSET + 1] = fog->fogEnd;
    //        uboGlobalView[UBOGlobal::GLOBAL_FOG_BASE_OFFSET + 2] = fog->fogDensity;
    //
    //        uboGlobalView[UBOGlobal::GLOBAL_FOG_ADD_OFFSET] = fog->fogTop;
    //        uboGlobalView[UBOGlobal::GLOBAL_FOG_ADD_OFFSET + 1] = fog->fogRange;
    //        uboGlobalView[UBOGlobal::GLOBAL_FOG_ADD_OFFSET + 2] = fog->fogAtten;
    //    }
}

bool ForwardPipeline::activeRenderer() {
    _commandBuffers.emplace_back(_device->createCommandBuffer({
        _device->getQueue(),
        gfx::CommandBufferType::PRIMARY,
    }));

    auto globalUBO = _device->createBuffer({gfx::BufferUsageBit::UNIFORM | gfx::BufferUsageBit::TRANSFER_DST,
                                            gfx::MemoryUsageBit::HOST | gfx::MemoryUsageBit::DEVICE,
                                            UBOGlobal::SIZE,
                                            UBOGlobal::SIZE,
                                            gfx::BufferFlagBit::NONE});
    _descriptorSet->bindBuffer(UBOGlobal::BLOCK.binding, globalUBO);

    auto shadowUBO = _device->createBuffer({gfx::BufferUsageBit::UNIFORM | gfx::BufferUsageBit::TRANSFER_DST,
                                            gfx::MemoryUsageBit::HOST | gfx::MemoryUsageBit::DEVICE,
                                            UBOShadow::SIZE,
                                            UBOShadow::SIZE,
                                            gfx::BufferFlagBit::NONE});
    _descriptorSet->bindBuffer(UBOShadow::BLOCK.binding, shadowUBO);

    gfx::SamplerInfo info;
    info.addressU = info.addressV = info.addressW = gfx::Address::CLAMP;
    auto shadowMapSamplerHash = genSamplerHash(std::move(info));
    auto shadowMapSampler = getSampler(shadowMapSamplerHash);
    _descriptorSet->bindSampler(UNIFORM_SHADOWMAP.binding, shadowMapSampler);

    // update global defines when all states initialized.
    _macros.setValue("CC_USE_HDR", _isHDR);
    _macros.setValue("CC_SUPPORT_FLOAT_TEXTURE", _device->hasFeature(gfx::Feature::TEXTURE_FLOAT));

    return true;
}

void ForwardPipeline::destroy() {

    if (_descriptorSet) {
        _descriptorSet->getBuffer(UBOGlobal::BLOCK.binding)->destroy();
        _descriptorSet->getBuffer(UBOShadow::BLOCK.binding)->destroy();
    }
    for (auto &it : _renderPasses) {
        it.second->destroy();
    }
    _renderPasses.clear();

    RenderPipeline::destroy();
}

} // namespace pipeline
} // namespace cc

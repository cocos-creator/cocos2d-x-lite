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

#include "GLES2Std.h"

#include "GLES2Buffer.h"
#include "GLES2CommandBuffer.h"
#include "GLES2Context.h"
#include "GLES2DescriptorSet.h"
#include "GLES2DescriptorSetLayout.h"
#include "GLES2Device.h"
#include "GLES2Framebuffer.h"
#include "GLES2InputAssembler.h"
#include "GLES2PipelineLayout.h"
#include "GLES2PipelineState.h"
#include "GLES2PrimaryCommandBuffer.h"
#include "GLES2Queue.h"
#include "GLES2RenderPass.h"
#include "GLES2Sampler.h"
#include "GLES2Shader.h"
#include "GLES2Texture.h"

namespace cc {
namespace gfx {

GLES2Device *GLES2Device::instance = nullptr;

GLES2Device *GLES2Device::getInstance() {
    return GLES2Device::instance;
}

GLES2Device::GLES2Device() {
    _api        = API::GLES2;
    _deviceName = "GLES2";

    GLES2Device::instance = this;
}

GLES2Device::~GLES2Device() {
    GLES2Device::instance = nullptr;
}

bool GLES2Device::doInit(const DeviceInfo &info) {
    ContextInfo ctxInfo;
    ctxInfo.windowHandle = _windowHandle;
    ctxInfo.msaaEnabled  = info.isAntiAlias;
    ctxInfo.performance  = Performance::HIGH_QUALITY;

    _renderContext = CC_NEW(GLES2Context);
    if (!_renderContext->initialize(ctxInfo)) {
        destroy();
        return false;
    }

    QueueInfo queueInfo;
    queueInfo.type = QueueType::GRAPHICS;
    _queue         = createQueue(queueInfo);

    CommandBufferInfo cmdBuffInfo;
    cmdBuffInfo.type  = CommandBufferType::PRIMARY;
    cmdBuffInfo.queue = _queue;
    _cmdBuff          = createCommandBuffer(cmdBuffInfo);

    _gpuStateCache          = CC_NEW(GLES2GPUStateCache);
    _gpuBlitManager         = CC_NEW(GLES2GPUBlitManager);
    _gpuStagingBufferPool   = CC_NEW(GLES2GPUStagingBufferPool);
    _gpuExtensionRegistry   = CC_NEW(GLES2GPUExtensionRegistry);
    _gpuFramebufferCacheMap = CC_NEW(GLES2GPUFramebufferCacheMap(_gpuStateCache));

    bindRenderContext(true);

    String extStr = reinterpret_cast<const char *>(glGetString(GL_EXTENSIONS));
    _extensions   = StringUtil::split(extStr, " ");

    if (checkExtension("EXT_sRGB")) {
        _features[static_cast<uint>(Feature::FORMAT_SRGB)] = true;
    }

    if (checkExtension("GL_OES_texture_float")) {
        _features[static_cast<uint>(Feature::TEXTURE_FLOAT)] = true;
    }

    if (checkExtension("GL_OES_texture_half_float")) {
        _features[static_cast<uint>(Feature::TEXTURE_HALF_FLOAT)] = true;
    }

    _features[static_cast<uint>(Feature::FORMAT_R11G11B10F)] = true;

    _features[static_cast<uint>(Feature::MSAA)] = _renderContext->multiSampleCount() > 0;

    if (checkExtension("GL_OES_element_index_uint")) {
        _features[static_cast<uint>(Feature::ELEMENT_INDEX_UINT)] = true;
    }

    if (checkExtension("color_buffer_float")) {
        _features[static_cast<uint>(Feature::COLOR_FLOAT)] = true;
    }

    if (checkExtension("color_buffer_half_float")) {
        _features[static_cast<uint>(Feature::COLOR_HALF_FLOAT)] = true;
    }

    if (checkExtension("texture_float_linear")) {
        _features[static_cast<uint>(Feature::TEXTURE_FLOAT_LINEAR)] = true;
    }

    if (checkExtension("texture_half_float_linear")) {
        _features[static_cast<uint>(Feature::TEXTURE_HALF_FLOAT_LINEAR)] = true;
    }

    if (checkExtension("draw_buffers")) {
        _features[static_cast<uint>(Feature::MULTIPLE_RENDER_TARGETS)] = true;
        glGetIntegerv(GL_MAX_DRAW_BUFFERS_EXT, reinterpret_cast<GLint *>(&_caps.maxColorRenderTargets));
    }

    if (checkExtension("blend_minmax")) {
        _features[static_cast<uint>(Feature::BLEND_MINMAX)] = true;
    }

    _gpuExtensionRegistry->useVAO                = checkExtension("vertex_array_object");
    _gpuExtensionRegistry->useDrawInstanced      = checkExtension("draw_instanced");
    _gpuExtensionRegistry->useInstancedArrays    = checkExtension("instanced_arrays");
    _gpuExtensionRegistry->useDiscardFramebuffer = checkExtension("discard_framebuffer");

    _features[static_cast<uint>(Feature::INSTANCED_ARRAYS)] = _gpuExtensionRegistry->useInstancedArrays;

    String fbfLevelStr = "NONE";
    if (checkExtension("framebuffer_fetch")) {
        String nonCoherent = "framebuffer_fetch_non";

        auto it = std::find_if(_extensions.begin(), _extensions.end(), [&nonCoherent](auto &ext) {
            return ext.find(nonCoherent) != String::npos;
        });

        if (it != _extensions.end()) {
            if (*it == CC_TOSTR(GL_EXT_shader_framebuffer_fetch_non_coherent)) {
                _gpuExtensionRegistry->mFBF = FBFSupportLevel::NON_COHERENT_EXT;
                fbfLevelStr                 = "NON_COHERENT_EXT";
            } else if (*it == CC_TOSTR(GL_QCOM_shader_framebuffer_fetch_noncoherent)) {
                _gpuExtensionRegistry->mFBF = FBFSupportLevel::NON_COHERENT_QCOM;
                fbfLevelStr                 = "NON_COHERENT_QCOM";
                GL_CHECK(glEnable(GL_FRAMEBUFFER_FETCH_NONCOHERENT_QCOM));
            }
        } else if (checkExtension(CC_TOSTR(GL_EXT_shader_framebuffer_fetch))) {
            // we only care about EXT_shader_framebuffer_fetch, the ARM version does not support MRT
            _gpuExtensionRegistry->mFBF = FBFSupportLevel::COHERENT;
            fbfLevelStr                 = "COHERENT";
        }
    }

    String compressedFmts;

    if (checkExtension("compressed_ETC1")) {
        _features[static_cast<uint>(Feature::FORMAT_ETC1)] = true;
        compressedFmts += "etc1 ";
    }

    if (checkForETC2()) {
        _features[static_cast<uint>(Feature::FORMAT_ETC2)] = true;
        compressedFmts += "etc2 ";
    }

    if (checkExtension("texture_compression_pvrtc")) {
        _features[static_cast<uint>(Feature::FORMAT_PVRTC)] = true;
        compressedFmts += "pvrtc ";
    }

    if (checkExtension("texture_compression_astc")) {
        _features[static_cast<uint>(Feature::FORMAT_ASTC)] = true;
        compressedFmts += "astc ";
    }
    _features[static_cast<uint>(Feature::DEPTH_BOUNDS)]         = true;
    _features[static_cast<uint>(Feature::LINE_WIDTH)]           = true;
    _features[static_cast<uint>(Feature::STENCIL_COMPARE_MASK)] = true;
    _features[static_cast<uint>(Feature::STENCIL_WRITE_MASK)]   = true;
    _features[static_cast<uint>(Feature::FORMAT_RGB8)]          = true;

    _renderer = reinterpret_cast<const char *>(glGetString(GL_RENDERER));
    _vendor   = reinterpret_cast<const char *>(glGetString(GL_VENDOR));
    _version  = reinterpret_cast<const char *>(glGetString(GL_VERSION));

    CC_LOG_INFO("GLES2 device initialized.");
    CC_LOG_INFO("RENDERER: %s", _renderer.c_str());
    CC_LOG_INFO("VENDOR: %s", _vendor.c_str());
    CC_LOG_INFO("VERSION: %s", _version.c_str());
    CC_LOG_INFO("SCREEN_SIZE: %d x %d", _width, _height);
    CC_LOG_INFO("COMPRESSED_FORMATS: %s", compressedFmts.c_str());
    CC_LOG_INFO("USE_VAO: %s", _gpuExtensionRegistry->useVAO ? "true" : "false");
    CC_LOG_INFO("FRAMEBUFFER_FETCH: %s", fbfLevelStr.c_str());

    glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, reinterpret_cast<GLint *>(&_caps.maxVertexAttributes));
    glGetIntegerv(GL_MAX_VERTEX_UNIFORM_VECTORS, reinterpret_cast<GLint *>(&_caps.maxVertexUniformVectors));
    glGetIntegerv(GL_MAX_FRAGMENT_UNIFORM_VECTORS, reinterpret_cast<GLint *>(&_caps.maxFragmentUniformVectors));
    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, reinterpret_cast<GLint *>(&_caps.maxTextureUnits));
    glGetIntegerv(GL_MAX_VERTEX_TEXTURE_IMAGE_UNITS, reinterpret_cast<GLint *>(&_caps.maxVertexTextureUnits));
    glGetIntegerv(GL_MAX_TEXTURE_SIZE, reinterpret_cast<GLint *>(&_caps.maxTextureSize));
    glGetIntegerv(GL_MAX_CUBE_MAP_TEXTURE_SIZE, reinterpret_cast<GLint *>(&_caps.maxCubeMapTextureSize));
    glGetIntegerv(GL_DEPTH_BITS, reinterpret_cast<GLint *>(&_caps.depthBits));
    glGetIntegerv(GL_STENCIL_BITS, reinterpret_cast<GLint *>(&_caps.stencilBits));

    _gpuStateCache->initialize(_caps.maxTextureUnits, _caps.maxVertexAttributes);
    _gpuBlitManager->initialize();

    return true;
}

void GLES2Device::doDestroy() {
    _gpuBlitManager->destroy();

    CC_SAFE_DELETE(_gpuFramebufferCacheMap)
    CC_SAFE_DELETE(_gpuExtensionRegistry)
    CC_SAFE_DELETE(_gpuStagingBufferPool)
    CC_SAFE_DELETE(_gpuBlitManager)
    CC_SAFE_DELETE(_gpuStateCache)

    CCASSERT(!_memoryStatus.bufferSize, "Buffer memory leaked");
    CCASSERT(!_memoryStatus.textureSize, "Texture memory leaked");

    CC_SAFE_DESTROY(_cmdBuff)
    CC_SAFE_DESTROY(_queue)
    CC_SAFE_DESTROY(_deviceContext)
    CC_SAFE_DESTROY(_renderContext)
}

void GLES2Device::releaseSurface(uintptr_t windowHandle) {
    static_cast<GLES2Context *>(_context)->releaseSurface(windowHandle);
}

void GLES2Device::acquireSurface(uintptr_t windowHandle) {
    static_cast<GLES2Context *>(_context)->acquireSurface(windowHandle);
}

void GLES2Device::resize(uint width, uint height) {
    _width  = width;
    _height = height;
}

void GLES2Device::acquire() {
    _gpuStagingBufferPool->reset();
}

void GLES2Device::present() {
    auto *queue   = static_cast<GLES2Queue *>(_queue);
    _numDrawCalls = queue->_numDrawCalls;
    _numInstances = queue->_numInstances;
    _numTriangles = queue->_numTriangles;

    _context->present();

    // Clear queue stats
    queue->_numDrawCalls = 0;
    queue->_numInstances = 0;
    queue->_numTriangles = 0;
}

void GLES2Device::bindRenderContext(bool bound) {
    _renderContext->makeCurrent(bound);
    _context = bound ? _renderContext : nullptr;

    if (bound) {
        _threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        _gpuStateCache->reset();
    }
}

void GLES2Device::bindDeviceContext(bool bound) {
    if (!_deviceContext) {
        ContextInfo ctxInfo;
        ctxInfo.windowHandle = _windowHandle;
        ctxInfo.sharedCtx    = _renderContext;

        _deviceContext = CC_NEW(GLES2Context);
        _deviceContext->initialize(ctxInfo);
    }
    _deviceContext->makeCurrent(bound);
    _context = bound ? _deviceContext : nullptr;

    if (bound) {
        _threadID = std::hash<std::thread::id>{}(std::this_thread::get_id());
        _gpuStateCache->reset();
    }
}

CommandBuffer *GLES2Device::createCommandBuffer(const CommandBufferInfo &info, bool hasAgent) {
    if (hasAgent || info.type == CommandBufferType::PRIMARY) return CC_NEW(GLES2PrimaryCommandBuffer);
    return CC_NEW(GLES2CommandBuffer);
}

Queue *GLES2Device::createQueue() {
    return CC_NEW(GLES2Queue);
}

Buffer *GLES2Device::createBuffer() {
    return CC_NEW(GLES2Buffer);
}

Texture *GLES2Device::createTexture() {
    return CC_NEW(GLES2Texture);
}

Sampler *GLES2Device::createSampler() {
    return CC_NEW(GLES2Sampler);
}

Shader *GLES2Device::createShader() {
    return CC_NEW(GLES2Shader);
}

InputAssembler *GLES2Device::createInputAssembler() {
    return CC_NEW(GLES2InputAssembler);
}

RenderPass *GLES2Device::createRenderPass() {
    return CC_NEW(GLES2RenderPass);
}

Framebuffer *GLES2Device::createFramebuffer() {
    return CC_NEW(GLES2Framebuffer);
}

DescriptorSet *GLES2Device::createDescriptorSet() {
    return CC_NEW(GLES2DescriptorSet);
}

DescriptorSetLayout *GLES2Device::createDescriptorSetLayout() {
    return CC_NEW(GLES2DescriptorSetLayout);
}

PipelineLayout *GLES2Device::createPipelineLayout() {
    return CC_NEW(GLES2PipelineLayout);
}

PipelineState *GLES2Device::createPipelineState() {
    return CC_NEW(GLES2PipelineState);
}

GlobalBarrier *GLES2Device::createGlobalBarrier() {
    return CC_NEW(GlobalBarrier);
}

TextureBarrier *GLES2Device::createTextureBarrier() {
    return CC_NEW(TextureBarrier);
}

void GLES2Device::copyBuffersToTexture(const uint8_t *const *buffers, Texture *dst, const BufferTextureCopy *regions, uint count) {
    cmdFuncGLES2CopyBuffersToTexture(this, buffers, static_cast<GLES2Texture *>(dst)->gpuTexture(), regions, count);
}

bool GLES2Device::checkForETC2() {
    GLint numFormats = 0;
    glGetIntegerv(GL_NUM_COMPRESSED_TEXTURE_FORMATS, &numFormats);
    auto *formats = new GLint[numFormats];
    glGetIntegerv(GL_COMPRESSED_TEXTURE_FORMATS, formats);

    int supportNum = 0;
    for (GLint i = 0; i < numFormats; ++i) {
        if (formats[i] == GL_COMPRESSED_RGB8_ETC2 || formats[i] == GL_COMPRESSED_RGBA8_ETC2_EAC) {
            supportNum++;
        }
    }
    delete[] formats;

    return supportNum >= 2;
}

} // namespace gfx
} // namespace cc

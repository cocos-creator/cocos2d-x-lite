#include "GLES2Std.h"
#include "GLES2Framebuffer.h"
#include "GLES2RenderPass.h"
#include "GLES2Commands.h"
#include "GLES2Texture.h"

namespace cc {
namespace gfx {

GLES2Framebuffer::GLES2Framebuffer(Device *device)
: Framebuffer(device) {
}

GLES2Framebuffer::~GLES2Framebuffer() {
}

bool GLES2Framebuffer::initialize(const FramebufferInfo &info) {

    _renderPass = info.renderPass;
    _colorTextures = info.colorTextures;
    _depthStencilTexture = info.depthStencilTexture;

    if (info.depthStencilMipmapLevel != 0) {
        CC_LOG_WARNING("Mipmap level of depth stencil attachment should be 0 in GLES2. Convert to 0.");
    }
    if (info.colorMipmapLevels.size() > 0) {
        int i = 0;
        for (const auto mipmapLevel : info.colorMipmapLevels) {
            if (mipmapLevel != 0) {
                CC_LOG_WARNING("Mipmap level of color attachment %d should be 0 in GLES2. Convert to 0.", i);
            }
            ++i;
        }
    }

    _gpuFBO = CC_NEW(GLES2GPUFramebuffer);
    _gpuFBO->gpuRenderPass = ((GLES2RenderPass *)_renderPass)->gpuRenderPass();

    _gpuFBO->gpuColorTextures.resize(_colorTextures.size());
    for (size_t i = 0; i < _colorTextures.size(); ++i) {
        GLES2Texture *colorTexture = (GLES2Texture *)_colorTextures[i];
        if (colorTexture) {
            _gpuFBO->gpuColorTextures[i] = colorTexture->gpuTexture();
        }
    }

    if (_depthStencilTexture) {
        _gpuFBO->gpuDepthStencilTexture = ((GLES2Texture *)_depthStencilTexture)->gpuTexture();
    }

    GLES2CmdFuncCreateFramebuffer((GLES2Device *)_device, _gpuFBO);

    return true;
}

void GLES2Framebuffer::destroy() {
    if (_gpuFBO) {
        GLES2CmdFuncDestroyFramebuffer((GLES2Device *)_device, _gpuFBO);
        CC_DELETE(_gpuFBO);
        _gpuFBO = nullptr;
    }
}

} // namespace gfx
} // namespace cc

#include "GLES2Std.h"
#include "GLES2Shader.h"
#include "GLES2Commands.h"

namespace cc {
namespace gfx {

GLES2Shader::GLES2Shader(Device *device)
: Shader(device) {
}

GLES2Shader::~GLES2Shader() {
}

bool GLES2Shader::initialize(const ShaderInfo &info) {
    _name = info.name;
    _stages = info.stages;
    _attributes = info.attributes;
    _blocks = info.blocks;
    _samplers = info.samplers;

    _gpuShader = CC_NEW(GLES2GPUShader);
    _gpuShader->name = _name;
    _gpuShader->blocks = _blocks;
    _gpuShader->samplers = _samplers;
    for (const auto &stage : _stages) {
        GLES2GPUShaderStage gpuShaderStage = {stage.stage, stage.source};
        _gpuShader->gpuStages.emplace_back(std::move(gpuShaderStage));
    }

    GLES2CmdFuncCreateShader((GLES2Device *)_device, _gpuShader);

    return true;
}

void GLES2Shader::destroy() {
    if (_gpuShader) {
        GLES2CmdFuncDestroyShader((GLES2Device *)_device, _gpuShader);
        CC_DELETE(_gpuShader);
        _gpuShader = nullptr;
    }
}

} // namespace gfx
} // namespace cc

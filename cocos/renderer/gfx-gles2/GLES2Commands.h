#ifndef CC_GFXGLES2_COMMANDS_H_
#define CC_GFXGLES2_COMMANDS_H_

#include "GLES2GPUObjects.h"

NS_CC_BEGIN

class GLES2Device;

struct GLES2DepthBias {
  float constant = 0.0f;
  float clamp = 0.0f;
  float slope = 0.0f;
};

struct GLES2DepthBounds {
  float min_bounds = 0.0f;
  float max_bounds = 0.0f;
};

struct GLES2StencilWriteMask {
  GFXStencilFace face = GFXStencilFace::FRONT;
  GLuint write_mask = 0;
};

struct GLES2StencilCompareMask {
  GFXStencilFace face = GFXStencilFace::FRONT;
  GLint refrence = 0;
  GLuint compare_mask = 0;
};

struct GLES2TextureSubres {
  uint baseMipLevel = 0;
  uint levelCount = 1;
  uint baseArrayLayer = 0;
  uint layerCount = 1;
};

struct GLES2BufferTextureCopy {
  uint buffOffset = 0;
  uint buffStride = 0;
  uint buffTexHeight = 0;
  uint texOffset[3] = {0};
  uint texExtent[3] = {0};
  GLES2TextureSubres texSubres;
};

class GLES2CmdBeginRenderPass : public GFXCmd {
 public:
  GLES2GPUFramebuffer* gpu_fbo = nullptr;
  GFXRect render_area;
  GFXClearFlags clear_flags = GFXClearFlagBit::NONE;
  uint num_clear_colors = 0;
  GFXColor clear_colors[GFX_MAX_ATTACHMENTS];
  float clear_depth = 1.0f;
  int clear_stencil = 0;
  
  GLES2CmdBeginRenderPass() : GFXCmd(GFXCmdType::BEGIN_RENDER_PASS) {}
  
  void clear() {
    gpu_fbo = nullptr;
    num_clear_colors = 0;
  }
};

enum class GLES2State : uint8_t {
  VIEWPORT,
  SCISSOR,
  LINE_WIDTH,
  DEPTH_BIAS,
  BLEND_CONSTANTS,
  DEPTH_BOUNDS,
  STENCIL_WRITE_MASK,
  STENCIL_COMPARE_MASK,
  COUNT,
};

class GLES2CmdBindStates : public GFXCmd {
 public:
  GLES2GPUPipelineState* gpu_pso = nullptr;
  GLES2GPUBindingLayout* gpu_binding_layout = nullptr;
  GLES2GPUInputAssembler* gpu_ia = nullptr;
  uint8_t state_flags[(int)GLES2State::COUNT] = {0};
  GFXViewport viewport;
  GFXRect scissor;
  float lineWidth = 1.0f;
  GLES2DepthBias depthBias;
  GFXColor blend_constants;
  GLES2DepthBounds depth_bounds;
  GLES2StencilWriteMask stencil_write_mask;
  GLES2StencilCompareMask stencil_compare_mask;
  
  GLES2CmdBindStates() : GFXCmd(GFXCmdType::BIND_STATES) {}
  
  void clear() {
    gpu_pso = nullptr;
    gpu_binding_layout = nullptr;
    gpu_ia = nullptr;
    memset(state_flags, 0, sizeof(state_flags));
  }
};

class GLES2CmdDraw : public GFXCmd {
 public:
  GFXDrawInfo draw_info;
  
  GLES2CmdDraw() : GFXCmd(GFXCmdType::DRAW) {}
  void clear() {}
};

class GLES2CmdUpdateBuffer : public GFXCmd {
 public:
  GLES2GPUBuffer* gpuBuffer = nullptr;
  uint8_t* buffer = nullptr;
  uint size = 0;
  uint offset = 0;
  
  GLES2CmdUpdateBuffer() : GFXCmd(GFXCmdType::UPDATE_BUFFER) {}
  
  void clear() {
    gpuBuffer = nullptr;
    buffer = nullptr;
  }
};

class GLES2CmdCopyBufferToTexture : public GFXCmd {
 public:
  GLES2GPUBuffer* gpuBuffer = nullptr;
  GLES2GPUTexture* gpuTexture = nullptr;
  GFXTextureLayout dst_layout;
  GFXBufferTextureCopyList regions;
  
  GLES2CmdCopyBufferToTexture() : GFXCmd(GFXCmdType::COPY_BUFFER_TO_TEXTURE) {}
  
  void clear() {
    gpuBuffer = nullptr;
    gpuTexture = nullptr;
    regions.clear();
  }
};

class GLES2CmdPackage : public Object {
 public:
  CachedArray<GFXCmdType> cmd_types;
  CachedArray<GLES2CmdBeginRenderPass*> begin_render_pass_cmds;
  CachedArray<GLES2CmdBindStates*> bind_states_cmds;
  CachedArray<GLES2CmdDraw*> draw_cmds;
  CachedArray<GLES2CmdUpdateBuffer*> update_buffer_cmds;
  CachedArray<GLES2CmdCopyBufferToTexture*> copy_buffer_to_texture_cmds;
};

CC_GLES2_API void GLES2CmdFuncCreateBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer);
CC_GLES2_API void GLES2CmdFuncDestroyBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer);
CC_GLES2_API void GLES2CmdFuncResizeBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer);
CC_GLES2_API void GLES2CmdFuncUpdateBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer, void* buffer, uint offset, uint size);
CC_GLES2_API void GLES2CmdFuncCreateTexture(GLES2Device* device, GLES2GPUTexture* gpuTexture);
CC_GLES2_API void GLES2CmdFuncDestroyTexture(GLES2Device* device, GLES2GPUTexture* gpuTexture);
CC_GLES2_API void GLES2CmdFuncResizeTexture(GLES2Device* device, GLES2GPUTexture* gpuTexture);
CC_GLES2_API void GLES2CmdFuncCreateSampler(GLES2Device* device, GLES2GPUSampler* gpuSampler);
CC_GLES2_API void GLES2CmdFuncDestroySampler(GLES2Device* device, GLES2GPUSampler* gpuSampler);
CC_GLES2_API void GLES2CmdFuncCreateShader(GLES2Device* device, GLES2GPUShader* gpuShader);
CC_GLES2_API void GLES2CmdFuncDestroyShader(GLES2Device* device, GLES2GPUShader* gpuShader);
CC_GLES2_API void GLES2CmdFuncCreateInputAssembler(GLES2Device* device, GLES2GPUInputAssembler* gpu_ia);
CC_GLES2_API void GLES2CmdFuncDestroyInputAssembler(GLES2Device* device, GLES2GPUInputAssembler* gpu_ia);
CC_GLES2_API void GLES2CmdFuncCreateFramebuffer(GLES2Device* device, GLES2GPUFramebuffer* gpu_fbo);
CC_GLES2_API void GLES2CmdFuncDestroyFramebuffer(GLES2Device* device, GLES2GPUFramebuffer* gpu_fbo);
CC_GLES2_API void GLES2CmdFuncExecuteCmds(GLES2Device* device, GLES2CmdPackage* cmd_package);
CC_GLES2_API void GLES2CmdFuncCopyBuffersToTexture(GLES2Device* device, uint8_t** buffers, uint count, GLES2GPUTexture* gpuTexture, const GFXBufferTextureCopyList& regions);

NS_CC_END

#endif

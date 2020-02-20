#ifndef CC_GFXGLES3_GPU_OBJECTS_H_
#define CC_GFXGLES3_GPU_OBJECTS_H_

#include "gles3w.h"

NS_CC_BEGIN

class GLES3GPUBuffer : public Object {
 public:
  GFXBufferUsage usage = GFXBufferUsage::NONE;
  GFXMemoryUsage memUsage = GFXMemoryUsage::NONE;
  uint size = 0;
  uint stride = 0;
  uint count = 0;
  GLenum gl_target = 0;
  GLuint gl_buffer = 0;
  uint8_t* buffer = nullptr;
  GFXIndirectBuffer indirect_buff;
};
typedef vector<GLES3GPUBuffer*>::type GLES3GPUBufferList;

class GLES3GPUTexture : public Object {
 public:
  GFXTextureType type = GFXTextureType::TEX2D;
  GFXTextureViewType view_type = GFXTextureViewType::TV2D;
  GFXFormat format = GFXFormat::UNKNOWN;
  GFXTextureUsage usage = GFXTextureUsageBit::NONE;
  uint width = 0;
  uint height = 0;
  uint depth = 1;
  uint size = 0;
  uint arrayLayer = 1;
  uint mipLevel = 1;
  GFXSampleCount samples = GFXSampleCount::X1;
  GFXTextureFlags flags = GFXTextureFlagBit::NONE;
  bool is_pot = false;
  GLenum gl_target = 0;
  GLenum gl_internal_fmt = 0;
  GLenum gl_format = 0;
  GLenum gl_type = 0;
  GLenum gl_usage = 0;
  GLuint gl_texture = 0;
  GLenum gl_wrap_s = 0;
  GLenum gl_wrap_t = 0;
  GLenum gl_min_filter = 0;
  GLenum gl_mag_filter = 0;
};

class GLES3GPUTextureView : public Object {
 public:
  GLES3GPUTexture* gpu_texture = nullptr;
  GFXTextureViewType type = GFXTextureViewType::TV2D;
  GFXFormat format = GFXFormat::UNKNOWN;
  uint baseLevel = 0;
  uint levelCount = 1;
};

typedef vector<GLES3GPUTextureView*>::type GLES3GPUTextureViewList;

class GLES3GPUSampler : public Object {
 public:
  GFXFilter minFilter = GFXFilter::NONE;
  GFXFilter magFilter = GFXFilter::NONE;
  GFXFilter mipFilter = GFXFilter::NONE;
  GFXAddress addressU = GFXAddress::CLAMP;
  GFXAddress addressV = GFXAddress::CLAMP;
  GFXAddress addressW = GFXAddress::CLAMP;
  uint minLOD = 0;
  uint maxLOD = 1000;
  GLuint gl_sampler = 0;
  GLenum gl_min_filter = 0;
  GLenum gl_mag_filter = 0;
  GLenum gl_wrap_s = 0;
  GLenum gl_wrap_t = 0;
  GLenum gl_wrap_r = 0;
};

struct GLES3GPUInput {
  uint binding;
  String name;
  GFXType type;
  uint stride;
  uint count;
  uint size;
  GLenum gl_type;
  GLint gl_loc;
};
typedef vector<GLES3GPUInput>::type GLES3GPUInputList;

struct GLES3GPUUniform {
  uint binding = GFX_INVALID_BINDING;
  String name;
    GFXType type = GFXType::UNKNOWN;
  uint stride = 0;
  uint count = 0;
  uint size = 0;
  uint offset = 0;
  GLenum gl_type;
  GLint gl_loc = -1;
};
typedef vector<GLES3GPUUniform>::type GLES3GPUUniformList;

struct GLES3GPUUniformBlock {
  uint binding;
  uint idx;
  String name;
  uint size;
  GLES3GPUUniformList uniforms;
  GLES3GPUUniformList active_uniforms;
};
typedef vector<GLES3GPUUniformBlock>::type GLES3GPUUniformBlockList;

struct GLES3GPUUniformSampler {
  uint binding;
  String name;
  GFXType type;
  vector<int>::type units;
  GLenum gl_type;
  GLint gl_loc;
};
typedef vector<GLES3GPUUniformSampler>::type GLES3GPUUniformSamplerList;

struct GLES3GPUShaderStage {
  GLES3GPUShaderStage(GFXShaderType t, String s, GFXShaderMacroList m, GLuint shader = 0)
    : type(t)
    , source(s)
    , macros(m)
    , gl_shader(shader)
    {}
  GFXShaderType type;
  String source;
  GFXShaderMacroList macros;
  GLuint gl_shader = 0;
};
typedef vector<GLES3GPUShaderStage>::type GLES3GPUShaderStageList;

class GLES3GPUShader : public Object {
public:
  String name;
  GFXUniformBlockList blocks;
  GFXUniformSamplerList samplers;
  GLuint gl_program = 0;
  GLES3GPUShaderStageList gpu_stages;
  GLES3GPUInputList gpu_inputs;
  GLES3GPUUniformBlockList gpu_blocks;
  GLES3GPUUniformSamplerList gpu_samplers;
};

struct GLES3GPUAttribute {
  String name;
  GLuint gl_buffer = 0;
  GLenum gl_type = 0;
  uint size = 0;
  uint count = 0;
  uint stride = 1;
  uint component_count = 1;
  bool isNormalized = false;
  bool isInstanced = false;
  uint offset = 0;
};
typedef vector<GLES3GPUAttribute>::type GLES3GPUAttributeList;

class GLES3GPUInputAssembler : public Object {
 public:
  GFXAttributeList attribs;
  GLES3GPUBufferList gpu_vertex_buffers;
  GLES3GPUBuffer* gpu_index_buffer = nullptr;
  GLES3GPUBuffer* gpu_indirect_buffer = nullptr;
  GLES3GPUAttributeList gpu_attribs;
  GLenum gl_index_type;
  map<GLuint, GLuint>::type gl_vaos;
};

class GLES3GPURenderPass : public Object {
 public:
  GFXColorAttachmentList colorAttachments;
  GFXDepthStencilAttachment depthStencilAttachment;
};

class GLES3GPUFramebuffer : public Object {
 public:
  GLES3GPURenderPass* gpu_render_pass = nullptr;
  GLES3GPUTextureViewList gpu_color_views;
  GLES3GPUTextureView* gpu_depth_stencil_view = nullptr;
  bool isOffscreen = false;
  GLuint gl_fbo = 0;
};

class GLES3GPUPipelineLayout : public Object {
 public:
};

class GLES3GPUPipelineState : public Object {
 public:
  GLenum gl_primitive = GL_TRIANGLES;
  GLES3GPUShader* gpu_shader = nullptr;
  GFXRasterizerState rs;
  GFXDepthStencilState dss;
  GFXBlendState bs;
  GFXDynamicStateList dynamicStates;
  GLES3GPUPipelineLayout* gpu_layout = nullptr;
  GLES3GPURenderPass* gpu_render_pass = nullptr;
};

struct GLES3GPUBinding {
  uint binding = GFX_INVALID_BINDING;
  GFXBindingType type = GFXBindingType::UNKNOWN;
  String name;
  GLES3GPUBuffer* gpu_buffer = nullptr;
  GLES3GPUTextureView* gpu_tex_view = nullptr;
  GLES3GPUSampler* gpu_sampler = nullptr;
};
typedef vector<GLES3GPUBinding>::type GLES3GPUBindingList;

class GLES3GPUBindingLayout : public Object {
 public:
  GLES3GPUBindingList gpu_bindings;
};

NS_CC_END

#endif

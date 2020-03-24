#include "GLES2Std.h"
#include "GLES2Commands.h"
#include "GLES2Device.h"
#include "GLES2StateCache.h"

#define BUFFER_OFFSET(idx) (static_cast<char*>(0) + (idx))

NS_CC_BEGIN

namespace
{
GLenum MapGLInternalFormat(GFXFormat format) {
  switch (format) {
    case GFXFormat::A8: return GL_ALPHA;
    case GFXFormat::L8: return GL_LUMINANCE;
    case GFXFormat::LA8: return GL_LUMINANCE_ALPHA;
    case GFXFormat::R8: return GL_LUMINANCE;
    case GFXFormat::R8SN: return GL_LUMINANCE;
    case GFXFormat::R8UI: return GL_LUMINANCE;
    case GFXFormat::R8I: return GL_LUMINANCE;
    case GFXFormat::RG8: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RG8SN: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RG8UI: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RG8I: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RGB8: return GL_RGB;
    case GFXFormat::RGB8SN: return GL_RGB;
    case GFXFormat::RGB8UI: return GL_RGB;
    case GFXFormat::RGB8I: return GL_RGB;
    case GFXFormat::RGBA8: return GL_RGBA;
    case GFXFormat::RGBA8SN: return GL_RGBA;
    case GFXFormat::RGBA8UI: return GL_RGBA;
    case GFXFormat::RGBA8I: return GL_RGBA;
    case GFXFormat::R16I: return GL_LUMINANCE;
    case GFXFormat::R16UI: return GL_LUMINANCE;
    case GFXFormat::R16F: return GL_LUMINANCE;
    case GFXFormat::RG16I: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RG16UI: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RG16F: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RGB16I: return GL_RGB;
    case GFXFormat::RGB16UI: return GL_RGB;
    case GFXFormat::RGB16F: return GL_RGB;
    case GFXFormat::RGBA16I: return GL_RGBA;
    case GFXFormat::RGBA16UI: return GL_RGBA;
    case GFXFormat::RGBA16F: return GL_RGBA;
    case GFXFormat::R32I: return GL_LUMINANCE;
    case GFXFormat::R32UI: return GL_LUMINANCE;
    case GFXFormat::R32F: return GL_LUMINANCE;
    case GFXFormat::RG32I: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RG32UI: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RG32F: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RGB32I: return GL_RGB;
    case GFXFormat::RGB32UI: return GL_RGB;
    case GFXFormat::RGB32F: return GL_RGB;
    case GFXFormat::RGBA32I: return GL_RGBA;
    case GFXFormat::RGBA32UI: return GL_RGBA;
#if CC_PLATFORM == CC_PLATFORM_WINDOWS
    case GFXFormat::RGBA32F: return GL_RGBA32F_EXT; // driver issue
#else
    case GFXFormat::RGBA32F: return GL_RGBA;
#endif
    case GFXFormat::R5G6B5: return GL_RGB565;
    case GFXFormat::RGB5A1: return GL_RGB5_A1;
    case GFXFormat::RGBA4: return GL_RGBA4;
    case GFXFormat::RGB10A2: return GL_RGB;
    case GFXFormat::RGB10A2UI: return GL_RGB;
    case GFXFormat::R11G11B10F: return GL_RGB;
    case GFXFormat::D16: return GL_DEPTH_COMPONENT;
    case GFXFormat::D16S8: return GL_DEPTH_STENCIL_OES;
    case GFXFormat::D24: return GL_DEPTH_COMPONENT;
    case GFXFormat::D24S8: return GL_DEPTH_STENCIL_OES;
    case GFXFormat::D32F: return GL_DEPTH_COMPONENT;
    case GFXFormat::D32F_S8: return GL_DEPTH_STENCIL_OES;
      
    case GFXFormat::BC1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case GFXFormat::BC1_ALPHA: return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    case GFXFormat::BC1_SRGB: return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
    case GFXFormat::BC1_SRGB_ALPHA: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
    case GFXFormat::BC2: return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case GFXFormat::BC2_SRGB: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
    case GFXFormat::BC3: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case GFXFormat::BC3_SRGB: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;
      
    case GFXFormat::ETC_RGB8: return GL_ETC1_RGB8_OES;
      
    case GFXFormat::PVRTC_RGB2: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
    case GFXFormat::PVRTC_RGBA2: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
    case GFXFormat::PVRTC_RGB4: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
    case GFXFormat::PVRTC_RGBA4: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
      
    default: {
      CCASSERT(false, "Unsupported GFXFormat, convert to GL internal format failed.");
      return GL_RGBA;
    }
  }
}

GLenum MapGLFormat(GFXFormat format) {
  switch (format) {
    case GFXFormat::A8: return GL_ALPHA;
    case GFXFormat::L8: return GL_LUMINANCE;
    case GFXFormat::LA8: return GL_LUMINANCE_ALPHA;
    case GFXFormat::R8:
    case GFXFormat::R8SN:
    case GFXFormat::R8UI:
    case GFXFormat::R8I: return GL_LUMINANCE;
    case GFXFormat::RG8:
    case GFXFormat::RG8SN:
    case GFXFormat::RG8UI:
    case GFXFormat::RG8I: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RGB8:
    case GFXFormat::RGB8SN:
    case GFXFormat::RGB8UI:
    case GFXFormat::RGB8I: return GL_RGB;
    case GFXFormat::RGBA8:
    case GFXFormat::RGBA8SN:
    case GFXFormat::RGBA8UI:
    case GFXFormat::RGBA8I: return GL_RGBA;
    case GFXFormat::R16UI:
    case GFXFormat::R16I:
    case GFXFormat::R16F: return GL_LUMINANCE;
    case GFXFormat::RG16UI:
    case GFXFormat::RG16I:
    case GFXFormat::RG16F: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RGB16UI:
    case GFXFormat::RGB16I:
    case GFXFormat::RGB16F: return GL_RGB;
    case GFXFormat::RGBA16UI:
    case GFXFormat::RGBA16I:
    case GFXFormat::RGBA16F: return GL_RGBA;
    case GFXFormat::R32UI:
    case GFXFormat::R32I:
    case GFXFormat::R32F: return GL_LUMINANCE;
    case GFXFormat::RG32UI:
    case GFXFormat::RG32I:
    case GFXFormat::RG32F: return GL_LUMINANCE_ALPHA;
    case GFXFormat::RGB32UI:
    case GFXFormat::RGB32I:
    case GFXFormat::RGB32F: return GL_RGB;
    case GFXFormat::RGBA32UI:
    case GFXFormat::RGBA32I:
    case GFXFormat::RGBA32F: return GL_RGBA;
    case GFXFormat::RGB10A2: return GL_RGBA;
    case GFXFormat::R11G11B10F: return GL_RGB;
    case GFXFormat::R5G6B5: return GL_RGB;
    case GFXFormat::RGB5A1: return GL_RGBA;
    case GFXFormat::RGBA4: return GL_RGBA;
    case GFXFormat::D16: return GL_DEPTH_COMPONENT;
    case GFXFormat::D16S8: return GL_DEPTH_STENCIL_OES;
    case GFXFormat::D24: return GL_DEPTH_COMPONENT;
    case GFXFormat::D24S8: return GL_DEPTH_STENCIL_OES;
    case GFXFormat::D32F: return GL_DEPTH_COMPONENT;
    case GFXFormat::D32F_S8: return GL_DEPTH_STENCIL_OES;
      
    case GFXFormat::BC1: return GL_COMPRESSED_RGB_S3TC_DXT1_EXT;
    case GFXFormat::BC1_ALPHA: return GL_COMPRESSED_RGBA_S3TC_DXT1_EXT;
    case GFXFormat::BC1_SRGB: return GL_COMPRESSED_SRGB_S3TC_DXT1_EXT;
    case GFXFormat::BC1_SRGB_ALPHA: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT;
    case GFXFormat::BC2: return GL_COMPRESSED_RGBA_S3TC_DXT3_EXT;
    case GFXFormat::BC2_SRGB: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT;
    case GFXFormat::BC3: return GL_COMPRESSED_RGBA_S3TC_DXT5_EXT;
    case GFXFormat::BC3_SRGB: return GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT;

    case GFXFormat::ETC_RGB8: return GL_ETC1_RGB8_OES;
      
    case GFXFormat::PVRTC_RGB2: return GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG;
    case GFXFormat::PVRTC_RGBA2: return GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG;
    case GFXFormat::PVRTC_RGB4: return GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG;
    case GFXFormat::PVRTC_RGBA4: return GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG;
      
    default: {
      CCASSERT(false, "Unsupported GFXFormat, convert to WebGL format failed.");
      return GL_RGBA;
    }
  }
}

GLenum MapGLType(GFXType type) {
  switch (type) {
    case GFXType::BOOL: return GL_BOOL;
    case GFXType::BOOL2: return GL_BOOL_VEC2;
    case GFXType::BOOL3: return GL_BOOL_VEC3;
    case GFXType::BOOL4: return GL_BOOL_VEC4;
    case GFXType::INT: return GL_INT;
    case GFXType::INT2: return GL_INT_VEC2;
    case GFXType::INT3: return GL_INT_VEC3;
    case GFXType::INT4: return GL_INT_VEC4;
    case GFXType::UINT: return GL_UNSIGNED_INT;
    case GFXType::FLOAT: return GL_FLOAT;
    case GFXType::FLOAT2: return GL_FLOAT_VEC2;
    case GFXType::FLOAT3: return GL_FLOAT_VEC3;
    case GFXType::FLOAT4: return GL_FLOAT_VEC4;
    case GFXType::MAT2: return GL_FLOAT_MAT2;
    case GFXType::MAT3: return GL_FLOAT_MAT3;
    case GFXType::MAT4: return GL_FLOAT_MAT4;
    case GFXType::SAMPLER2D: return GL_SAMPLER_2D;
    case GFXType::SAMPLER3D: return GL_SAMPLER_3D_OES;
    case GFXType::SAMPLER_CUBE: return GL_SAMPLER_CUBE;
    default: {
      CCASSERT(false, "Unsupported GLType, convert to GL type failed.");
      return GL_NONE;
    }
  }
}

GFXType MapGFXType(GLenum glType) {
  switch (glType) {
    case GL_BOOL: return GFXType::BOOL;
    case GL_BOOL_VEC2: return GFXType::BOOL2;
    case GL_BOOL_VEC3: return GFXType::BOOL3;
    case GL_BOOL_VEC4: return GFXType::BOOL4;
    case GL_INT: return GFXType::INT;
    case GL_INT_VEC2: return GFXType::INT2;
    case GL_INT_VEC3: return GFXType::INT3;
    case GL_INT_VEC4: return GFXType::INT4;
    case GL_UNSIGNED_INT: return GFXType::UINT;
    case GL_FLOAT: return GFXType::FLOAT;
    case GL_FLOAT_VEC2: return GFXType::FLOAT2;
    case GL_FLOAT_VEC3: return GFXType::FLOAT3;
    case GL_FLOAT_VEC4: return GFXType::FLOAT4;
    case GL_FLOAT_MAT2: return GFXType::MAT2;
    case GL_FLOAT_MAT3: return GFXType::MAT3;
    case GL_FLOAT_MAT4: return GFXType::MAT4;
    case GL_SAMPLER_2D: return GFXType::SAMPLER2D;
    case GL_SAMPLER_3D_OES: return GFXType::SAMPLER3D;
    case GL_SAMPLER_CUBE: return GFXType::SAMPLER_CUBE;
    default: {
      CCASSERT(false, "Unsupported GLType, convert to GFXType failed.");
      return GFXType::UNKNOWN;
    }
  }
}

GLenum GFXFormatToGLType(GFXFormat format) {
  switch (format) {
    case GFXFormat::R8: return GL_UNSIGNED_BYTE;
    case GFXFormat::R8SN: return GL_BYTE;
    case GFXFormat::R8UI: return GL_UNSIGNED_BYTE;
    case GFXFormat::R8I: return GL_BYTE;
    case GFXFormat::R16UI: return GL_UNSIGNED_SHORT;
    case GFXFormat::R16I: return GL_SHORT;
    case GFXFormat::R32F: return GL_FLOAT;
    case GFXFormat::R32UI: return GL_UNSIGNED_INT;
    case GFXFormat::R32I: return GL_INT;
      
    case GFXFormat::RG8: return GL_UNSIGNED_BYTE;
    case GFXFormat::RG8SN: return GL_BYTE;
    case GFXFormat::RG8UI: return GL_UNSIGNED_BYTE;
    case GFXFormat::RG8I: return GL_BYTE;
    case GFXFormat::RG16UI: return GL_UNSIGNED_SHORT;
    case GFXFormat::RG16I: return GL_SHORT;
    case GFXFormat::RG32F: return GL_FLOAT;
    case GFXFormat::RG32UI: return GL_UNSIGNED_INT;
    case GFXFormat::RG32I: return GL_INT;
      
    case GFXFormat::RGB8: return GL_UNSIGNED_BYTE;
    case GFXFormat::SRGB8: return GL_UNSIGNED_BYTE;
    case GFXFormat::RGB8SN: return GL_BYTE;
    case GFXFormat::RGB8UI: return GL_UNSIGNED_BYTE;
    case GFXFormat::RGB8I: return GL_BYTE;
    case GFXFormat::RGB16F: return GL_HALF_FLOAT_OES;
    case GFXFormat::RGB16UI: return GL_UNSIGNED_SHORT;
    case GFXFormat::RGB16I: return GL_SHORT;
    case GFXFormat::RGB32F: return GL_FLOAT;
    case GFXFormat::RGB32UI: return GL_UNSIGNED_INT;
    case GFXFormat::RGB32I: return GL_INT;
      
    case GFXFormat::RGBA8: return GL_UNSIGNED_BYTE;
    case GFXFormat::SRGB8_A8: return GL_UNSIGNED_BYTE;
    case GFXFormat::RGBA8SN: return GL_BYTE;
    case GFXFormat::RGBA8UI: return GL_UNSIGNED_BYTE;
    case GFXFormat::RGBA8I: return GL_BYTE;
    case GFXFormat::RGBA16F: return GL_HALF_FLOAT_OES;
    case GFXFormat::RGBA16UI: return GL_UNSIGNED_SHORT;
    case GFXFormat::RGBA16I: return GL_SHORT;
    case GFXFormat::RGBA32F: return GL_FLOAT;
    case GFXFormat::RGBA32UI: return GL_UNSIGNED_INT;
    case GFXFormat::RGBA32I: return GL_INT;
      
    case GFXFormat::R5G6B5: return GL_UNSIGNED_SHORT_5_6_5;
    case GFXFormat::R11G11B10F: return GL_FLOAT;
    case GFXFormat::RGB5A1: return GL_UNSIGNED_SHORT_5_5_5_1;
    case GFXFormat::RGBA4: return GL_UNSIGNED_SHORT_4_4_4_4;
    case GFXFormat::RGB10A2: return GL_UNSIGNED_BYTE;
    case GFXFormat::RGB10A2UI: return GL_UNSIGNED_INT;
    case GFXFormat::RGB9E5: return GL_FLOAT;
      
    case GFXFormat::D16: return GL_UNSIGNED_SHORT;
    case GFXFormat::D16S8: return GL_UNSIGNED_SHORT;
    case GFXFormat::D24: return GL_UNSIGNED_INT;
    case GFXFormat::D24S8: return GL_UNSIGNED_INT_24_8_OES;
    case GFXFormat::D32F: return GL_FLOAT;
      
    case GFXFormat::BC1: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC1_SRGB: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC2: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC2_SRGB: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC3: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC3_SRGB: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC4: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC4_SNORM: return GL_BYTE;
    case GFXFormat::BC5: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC5_SNORM: return GL_BYTE;
    case GFXFormat::BC6H_SF16: return GL_FLOAT;
    case GFXFormat::BC6H_UF16: return GL_FLOAT;
    case GFXFormat::BC7: return GL_UNSIGNED_BYTE;
    case GFXFormat::BC7_SRGB: return GL_UNSIGNED_BYTE;
      
    case GFXFormat::ETC_RGB8: return GL_UNSIGNED_BYTE;
    case GFXFormat::ETC2_RGB8: return GL_UNSIGNED_BYTE;
    case GFXFormat::ETC2_SRGB8: return GL_UNSIGNED_BYTE;
    case GFXFormat::ETC2_RGB8_A1: return GL_UNSIGNED_BYTE;
    case GFXFormat::ETC2_SRGB8_A1: return GL_UNSIGNED_BYTE;
    case GFXFormat::EAC_R11: return GL_UNSIGNED_BYTE;
    case GFXFormat::EAC_R11SN: return GL_BYTE;
    case GFXFormat::EAC_RG11: return GL_UNSIGNED_BYTE;
    case GFXFormat::EAC_RG11SN: return GL_BYTE;
      
    case GFXFormat::PVRTC_RGB2: return GL_UNSIGNED_BYTE;
    case GFXFormat::PVRTC_RGBA2: return GL_UNSIGNED_BYTE;
    case GFXFormat::PVRTC_RGB4: return GL_UNSIGNED_BYTE;
    case GFXFormat::PVRTC_RGBA4: return GL_UNSIGNED_BYTE;
    case GFXFormat::PVRTC2_2BPP: return GL_UNSIGNED_BYTE;
    case GFXFormat::PVRTC2_4BPP: return GL_UNSIGNED_BYTE;
      
    default: {
      return GL_UNSIGNED_BYTE;
    }
  }
}

uint GLTypeSize(GLenum glType) {
  switch (glType) {
    case GL_BOOL: return 4;
    case GL_BOOL_VEC2: return 8;
    case GL_BOOL_VEC3: return 12;
    case GL_BOOL_VEC4: return 16;
    case GL_INT: return 4;
    case GL_INT_VEC2: return 8;
    case GL_INT_VEC3: return 12;
    case GL_INT_VEC4: return 16;
    case GL_UNSIGNED_INT: return 4;
    case GL_FLOAT: return 4;
    case GL_FLOAT_VEC2: return 8;
    case GL_FLOAT_VEC3: return 12;
    case GL_FLOAT_VEC4: return 16;
    case GL_FLOAT_MAT2: return 16;
    case GL_FLOAT_MAT3: return 36;
    case GL_FLOAT_MAT4: return 64;
    case GL_SAMPLER_2D: return 4;
    case GL_SAMPLER_3D_OES: return 4;
    case GL_SAMPLER_CUBE: return 4;
    case GL_SAMPLER_CUBE_MAP_ARRAY_OES: return 4;
    case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_OES: return 4;
    case GL_INT_SAMPLER_CUBE_MAP_ARRAY_OES: return 4;
    case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_OES: return 4;
    default: {
      CCASSERT(false, "Unsupported GLType, get type failed.");
      return 0;
    }
  }
}

uint GLComponentCount (GLenum glType) {
  switch (glType) {
    case GL_FLOAT_MAT2: return 2;
    case GL_FLOAT_MAT3: return 3;
    case GL_FLOAT_MAT4: return 4;
    default: {
      return 1;
    }
  }
}

const GLenum GLES3_WRAPS[] = {
  GL_REPEAT,
  GL_MIRRORED_REPEAT,
  GL_CLAMP_TO_EDGE,
  GL_CLAMP_TO_EDGE,
};

const GLenum GLES2_CMP_FUNCS[] = {
  GL_NEVER,
  GL_LESS,
  GL_EQUAL,
  GL_LEQUAL,
  GL_GREATER,
  GL_NOTEQUAL,
  GL_GEQUAL,
  GL_ALWAYS,
};

const GLenum GLES2_STENCIL_OPS[] = {
  GL_ZERO,
  GL_KEEP,
  GL_REPLACE,
  GL_INCR,
  GL_DECR,
  GL_INVERT,
  GL_INCR_WRAP,
  GL_DECR_WRAP,
};

const GLenum GLES2_BLEND_OPS[] = {
  GL_FUNC_ADD,
  GL_FUNC_SUBTRACT,
  GL_FUNC_REVERSE_SUBTRACT,
  GL_FUNC_ADD,
  GL_FUNC_ADD,
};

const GLenum GLES2_BLEND_FACTORS[] = {
  GL_ZERO,
  GL_ONE,
  GL_SRC_ALPHA,
  GL_DST_ALPHA,
  GL_ONE_MINUS_SRC_ALPHA,
  GL_ONE_MINUS_DST_ALPHA,
  GL_SRC_COLOR,
  GL_DST_COLOR,
  GL_ONE_MINUS_SRC_COLOR,
  GL_ONE_MINUS_DST_COLOR,
  GL_SRC_ALPHA_SATURATE,
  GL_CONSTANT_COLOR,
  GL_ONE_MINUS_CONSTANT_COLOR,
  GL_CONSTANT_ALPHA,
  GL_ONE_MINUS_CONSTANT_ALPHA,
};
}

void GLES2CmdFuncCreateBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer) {
  GLenum glUsage = (gpuBuffer->memUsage & GFXMemoryUsageBit::HOST ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  
  if (gpuBuffer->usage & GFXBufferUsageBit::VERTEX) {
    gpuBuffer->glTarget = GL_ARRAY_BUFFER;
    glGenBuffers(1, &gpuBuffer->glBuffer);
    if (gpuBuffer->size) {
      if (device->useVAO()) {
        if (device->stateCache->glVAO) {
          glBindVertexArrayOES(0);
          device->stateCache->glVAO = 0;
        }
      }
      
      if (device->stateCache->glArrayBuffer != gpuBuffer->glBuffer) {
        glBindBuffer(GL_ARRAY_BUFFER, gpuBuffer->glBuffer);
      }
      
      glBufferData(GL_ARRAY_BUFFER, gpuBuffer->size, nullptr, glUsage);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      device->stateCache->glArrayBuffer = 0;
    }
  } else if (gpuBuffer->usage & GFXBufferUsageBit::INDEX) {
    gpuBuffer->glTarget = GL_ELEMENT_ARRAY_BUFFER;
    glGenBuffers(1, &gpuBuffer->glBuffer);
    if (gpuBuffer->size) {
      if (device->useVAO()) {
        if (device->stateCache->glVAO) {
          glBindVertexArrayOES(0);
          device->stateCache->glVAO = 0;
        }
      }
      
      if (device->stateCache->glElementArrayBuffer != gpuBuffer->glBuffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuBuffer->glBuffer);
      }
      
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, gpuBuffer->size, nullptr, glUsage);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      device->stateCache->glElementArrayBuffer = 0;
    }
  } else if (gpuBuffer->usage & GFXBufferUsageBit::UNIFORM) {
    gpuBuffer->buffer = (uint8_t*)CC_MALLOC(gpuBuffer->size);
    gpuBuffer->glTarget = GL_NONE;
  } else if (gpuBuffer->usage & GFXBufferUsageBit::INDIRECT){
    gpuBuffer->glTarget = GL_NONE;
  } else if ((gpuBuffer->usage & GFXBufferUsageBit::TRANSFER_DST) ||
             (gpuBuffer->usage & GFXBufferUsageBit::TRANSFER_SRC)) {
      gpuBuffer->buffer = (uint8_t*)CC_MALLOC(gpuBuffer->size);
    gpuBuffer->glTarget = GL_NONE;
  } else {
      CCASSERT(false, "Unsupported GFXBufferType, create buffer failed.");
    gpuBuffer->glTarget = GL_NONE;
  }
}

void GLES2CmdFuncDestroyBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer) {
    if (gpuBuffer->glBuffer) {
      if (gpuBuffer->usage & GFXBufferUsageBit::VERTEX) {
        if (device->stateCache->glArrayBuffer == gpuBuffer->glBuffer) {
          glBindBuffer(GL_ARRAY_BUFFER, 0);
          device->stateCache->glArrayBuffer = 0;
        }
      } else if (gpuBuffer->usage & GFXBufferUsageBit::INDEX) {
        if (device->stateCache->glElementArrayBuffer == gpuBuffer->glBuffer) {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
          device->stateCache->glElementArrayBuffer = 0;
        }
      }
      glDeleteBuffers(1, &gpuBuffer->glBuffer);
      gpuBuffer->glBuffer = 0;
    }
    CC_SAFE_FREE(gpuBuffer->buffer);
}

void GLES2CmdFuncResizeBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer) {
  GLenum glUsage = (gpuBuffer->memUsage & GFXMemoryUsageBit::HOST ? GL_DYNAMIC_DRAW : GL_STATIC_DRAW);
  
  if (gpuBuffer->usage & GFXBufferUsageBit::VERTEX) {
    gpuBuffer->glTarget = GL_ARRAY_BUFFER;
    if (gpuBuffer->size) {
      if (device->useVAO()) {
        if (device->stateCache->glVAO) {
          glBindVertexArrayOES(0);
          device->stateCache->glVAO = 0;
        }
      }
      
      if (device->stateCache->glArrayBuffer != gpuBuffer->glBuffer) {
        glBindBuffer(GL_ARRAY_BUFFER, gpuBuffer->glBuffer);
      }
      
      glBufferData(GL_ARRAY_BUFFER, gpuBuffer->size, nullptr, glUsage);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      device->stateCache->glArrayBuffer = 0;
    }
  } else if (gpuBuffer->usage & GFXBufferUsageBit::INDEX) {
    gpuBuffer->glTarget = GL_ELEMENT_ARRAY_BUFFER;
    if (gpuBuffer->size) {
      if (device->useVAO()) {
        if (device->stateCache->glVAO) {
          glBindVertexArrayOES(0);
          device->stateCache->glVAO = 0;
        }
      }
      
      if (device->stateCache->glElementArrayBuffer != gpuBuffer->glBuffer) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuBuffer->glBuffer);
      }
      
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, gpuBuffer->size, nullptr, glUsage);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
      device->stateCache->glElementArrayBuffer = 0;
    }
  } else if (gpuBuffer->usage & GFXBufferUsageBit::UNIFORM) {
    if (gpuBuffer->buffer) {
      CC_FREE(gpuBuffer->buffer);
    }
    gpuBuffer->buffer = (uint8_t*)CC_MALLOC(gpuBuffer->size);
    gpuBuffer->glTarget = GL_NONE;
  } else if (gpuBuffer->usage & GFXBufferUsageBit::INDIRECT) {
    gpuBuffer->indirects.resize(gpuBuffer->count);
    gpuBuffer->glTarget = GL_NONE;
  } else if ((gpuBuffer->usage & GFXBufferUsageBit::TRANSFER_DST) ||
             (gpuBuffer->usage & GFXBufferUsageBit::TRANSFER_SRC)) {
      if (gpuBuffer->buffer) {
        CC_FREE(gpuBuffer->buffer);
      }
      gpuBuffer->buffer = (uint8_t*)CC_MALLOC(gpuBuffer->size);
    gpuBuffer->glTarget = GL_NONE;
  } else {
      CCASSERT(false, "Unsupported GFXBufferType, resize buffer failed.");
    gpuBuffer->glTarget = GL_NONE;
  }
}

void GLES2CmdFuncUpdateBuffer(GLES2Device* device, GLES2GPUBuffer* gpuBuffer, void* buffer, uint offset, uint size) {
  CCASSERT(buffer, "Buffer should not be nullptr");
  if (gpuBuffer->usage & GFXBufferUsageBit::UNIFORM) {
    memcpy(gpuBuffer->buffer + offset, buffer, size);
  } else if (gpuBuffer->usage & GFXBufferUsageBit::INDIRECT) {
      auto count = size / sizeof(GFXDrawInfo);
      if(gpuBuffer->indirects.size() < count)
      {
          gpuBuffer->indirects.resize(count);
      }
    memcpy((uint8_t*)gpuBuffer->indirects.data() + offset, buffer, size);
  } else if (gpuBuffer->usage & GFXBufferUsageBit::TRANSFER_SRC) {
      memcpy(gpuBuffer->buffer + offset, buffer, size);
  } else {
    switch (gpuBuffer->glTarget) {
      case GL_ARRAY_BUFFER: {
        if (device->useVAO()) {
          if (device->stateCache->glVAO) {
            glBindVertexArrayOES(0);
            device->stateCache->glVAO = 0;
          }
        }
        if (device->stateCache->glArrayBuffer != gpuBuffer->glBuffer) {
          glBindBuffer(GL_ARRAY_BUFFER, gpuBuffer->glBuffer);
          device->stateCache->glArrayBuffer = gpuBuffer->glBuffer;
        }
        glBufferSubData(GL_ARRAY_BUFFER, offset, size, buffer);
        break;
      }
      case GL_ELEMENT_ARRAY_BUFFER: {
        if (device->useVAO()) {
          if (device->stateCache->glVAO) {
            glBindVertexArrayOES(0);
            device->stateCache->glVAO = 0;
          }
        }
        if (device->stateCache->glElementArrayBuffer != gpuBuffer->glBuffer) {
          glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuBuffer->glBuffer);
          device->stateCache->glElementArrayBuffer = gpuBuffer->glBuffer;
        }
        glBufferSubData(GL_ELEMENT_ARRAY_BUFFER, offset, size, buffer);
        break;
      }
      default:
        CCASSERT(false, "Unsupported GFXBufferType, update buffer failed.");
        break;
    }
  }
}

void GLES2CmdFuncCreateTexture(GLES2Device* device, GLES2GPUTexture* gpuTexture) {
  gpuTexture->glInternelFmt = MapGLInternalFormat(gpuTexture->format);
  gpuTexture->glFormat = MapGLFormat(gpuTexture->format);
  gpuTexture->glType = GFXFormatToGLType(gpuTexture->format);
  
  switch (gpuTexture->viewType) {
    case GFXTextureViewType::TV2D: {
      gpuTexture->viewType = GFXTextureViewType::TV2D;
      gpuTexture->glTarget = GL_TEXTURE_2D;
      glGenTextures(1, &gpuTexture->glTexture);
      if (gpuTexture->size > 0) {
        GLuint& glTexture = device->stateCache->glTextures[device->stateCache->texUint];
        if (gpuTexture->glTexture != glTexture) {
          glBindTexture(GL_TEXTURE_2D, gpuTexture->glTexture);
          glTexture = gpuTexture->glTexture;
        }
        uint w = gpuTexture->width;
        uint h = gpuTexture->height;
        if (!GFX_FORMAT_INFOS[(int)gpuTexture->format].isCompressed) {
          for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
            glTexImage2D(GL_TEXTURE_2D, i, gpuTexture->glInternelFmt, w, h, 0, gpuTexture->glFormat, gpuTexture->glType, nullptr);
            w = std::max(1U, w >> 1);
            h = std::max(1U, h >> 1);
          }
        } else {
          for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
            uint img_size = GFXFormatSize(gpuTexture->format, w, h, 1);
            glCompressedTexImage2D(GL_TEXTURE_2D, i, gpuTexture->glInternelFmt, w, h, 0, img_size, nullptr);
            w = std::max(1U, w >> 1);
            h = std::max(1U, h >> 1);
          }
        }
      }
      break;
    }
    case GFXTextureViewType::CUBE: {
      gpuTexture->viewType = GFXTextureViewType::CUBE;
      gpuTexture->glTarget = GL_TEXTURE_CUBE_MAP;
      glGenTextures(1, &gpuTexture->glTexture);
      if (gpuTexture->size > 0) {
        GLuint& glTexture = device->stateCache->glTextures[device->stateCache->texUint];
        if (gpuTexture->glTexture != glTexture) {
          glBindTexture(GL_TEXTURE_CUBE_MAP, gpuTexture->glTexture);
          glTexture = gpuTexture->glTexture;
        }
        if (!GFX_FORMAT_INFOS[(int)gpuTexture->format].isCompressed) {
          for (uint f = 0; f < 6; ++f) {
            uint w = gpuTexture->width;
            uint h = gpuTexture->height;
            for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
              glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, i, gpuTexture->glInternelFmt, w, h, 0, gpuTexture->glFormat, gpuTexture->glType, nullptr);
              w = std::max(1U, w >> 1);
              h = std::max(1U, h >> 1);
            }
          }
        } else {
          for (uint f = 0; f < 6; ++f) {
            uint w = gpuTexture->width;
            uint h = gpuTexture->height;
            for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
              uint img_size = GFXFormatSize(gpuTexture->format, w, h, 1);
              glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, i, gpuTexture->glInternelFmt, w, h, 0, img_size, nullptr);
              w = std::max(1U, w >> 1);
              h = std::max(1U, h >> 1);
            }
          }
        }
      }
      break;
    }
    default:
      CCASSERT(false, "Unsupported GFXTextureType, create texture failed.");
      break;
  }
}

void GLES2CmdFuncDestroyTexture(GLES2Device* device, GLES2GPUTexture* gpuTexture) {
  if (gpuTexture->glTexture) {
    GLuint& glTexture = device->stateCache->glTextures[device->stateCache->texUint];
    if (glTexture == gpuTexture->glTexture) {
      switch (gpuTexture->viewType) {
        case GFXTextureViewType::TV2D: {
          glBindTexture(GL_TEXTURE_2D, 0);
          break;
        }
        case GFXTextureViewType::CUBE: {
          glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
          break;
        }
      }
      glTexture = 0;
    }
    glDeleteTextures(1, &gpuTexture->glTexture);
    gpuTexture->glTexture = 0;
  }
}

void GLES2CmdFuncResizeTexture(GLES2Device* device, GLES2GPUTexture* gpuTexture) {
  gpuTexture->glInternelFmt = MapGLInternalFormat(gpuTexture->format);
  gpuTexture->glFormat = MapGLFormat(gpuTexture->format);
  gpuTexture->glType = GFXFormatToGLType(gpuTexture->format);
  
  switch (gpuTexture->viewType) {
    case GFXTextureViewType::TV2D: {
      gpuTexture->viewType = GFXTextureViewType::TV2D;
      gpuTexture->glTarget = GL_TEXTURE_2D;
      if (gpuTexture->size > 0) {
        GLuint& glTexture = device->stateCache->glTextures[device->stateCache->texUint];
        if (gpuTexture->glTexture != glTexture) {
          glBindTexture(GL_TEXTURE_2D, gpuTexture->glTexture);
          glTexture = gpuTexture->glTexture;
        }
        uint w = gpuTexture->width;
        uint h = gpuTexture->height;
        if (!GFX_FORMAT_INFOS[(int)gpuTexture->format].isCompressed) {
          for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
            glTexImage2D(GL_TEXTURE_2D, i, gpuTexture->glInternelFmt, w, h, 0, gpuTexture->glFormat, gpuTexture->glType, nullptr);
            w = std::max(1U, w >> 1);
            h = std::max(1U, h >> 1);
          }
        } else {
          for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
            uint img_size = GFXFormatSize(gpuTexture->format, w, h, 1);
            glCompressedTexImage2D(GL_TEXTURE_2D, i, gpuTexture->glInternelFmt, w, h, 0, img_size, nullptr);
            w = std::max(1U, w >> 1);
            h = std::max(1U, h >> 1);
          }
        }
      }
      break;
    }
    case GFXTextureViewType::CUBE: {
      gpuTexture->viewType = GFXTextureViewType::CUBE;
      gpuTexture->glTarget = GL_TEXTURE_CUBE_MAP;
      if (gpuTexture->size > 0) {
        GLuint& glTexture = device->stateCache->glTextures[device->stateCache->texUint];
        if (gpuTexture->glTexture != glTexture) {
          glBindTexture(GL_TEXTURE_CUBE_MAP, gpuTexture->glTexture);
          glTexture = gpuTexture->glTexture;
        }
        if (!GFX_FORMAT_INFOS[(int)gpuTexture->format].isCompressed) {
          for (uint f = 0; f < 6; ++f) {
            uint w = gpuTexture->width;
            uint h = gpuTexture->height;
            for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
              glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, i, gpuTexture->glInternelFmt, w, h, 0, gpuTexture->glFormat, gpuTexture->glType, nullptr);
              w = std::max(1U, w >> 1);
              h = std::max(1U, h >> 1);
            }
          }
        } else {
          for (uint f = 0; f < 6; ++f) {
            uint w = gpuTexture->width;
            uint h = gpuTexture->height;
            for (uint i = 0; i < gpuTexture->mipLevel; ++i) {
              uint img_size = GFXFormatSize(gpuTexture->format, w, h, 1);
              glCompressedTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, i, gpuTexture->glInternelFmt, w, h, 0, img_size, nullptr);
              w = std::max(1U, w >> 1);
              h = std::max(1U, h >> 1);
            }
          }
        }
      }
      break;
    }
    default:
      CCASSERT(false, "Unsupported GFXTextureType, resize texture failed.");
      break;
  }
}

void GLES2CmdFuncCreateSampler(GLES2Device* device, GLES2GPUSampler* gpuSampler) {
  if (gpuSampler->minFilter == GFXFilter::LINEAR || gpuSampler->minFilter == GFXFilter::ANISOTROPIC) {
    if (gpuSampler->mipFilter == GFXFilter::LINEAR || gpuSampler->mipFilter == GFXFilter::ANISOTROPIC) {
      gpuSampler->glMinFilter = GL_LINEAR_MIPMAP_LINEAR;
    } else if (gpuSampler->mipFilter == GFXFilter::POINT) {
      gpuSampler->glMinFilter = GL_LINEAR_MIPMAP_NEAREST;
    } else {
      gpuSampler->glMinFilter = GL_LINEAR;
    }
  } else {
    if (gpuSampler->mipFilter == GFXFilter::LINEAR || gpuSampler->mipFilter == GFXFilter::ANISOTROPIC) {
      gpuSampler->glMinFilter = GL_NEAREST_MIPMAP_LINEAR;
    } else if (gpuSampler->mipFilter == GFXFilter::POINT) {
      gpuSampler->glMinFilter = GL_NEAREST_MIPMAP_NEAREST;
    } else {
      gpuSampler->glMinFilter = GL_NEAREST;
    }
  }
  
  if (gpuSampler->magFilter == GFXFilter::LINEAR || gpuSampler->magFilter == GFXFilter::ANISOTROPIC) {
    gpuSampler->glMagFilter = GL_LINEAR;
  } else {
    gpuSampler->glMagFilter = GL_NEAREST;
  }
  
  gpuSampler->glWrapS = GLES3_WRAPS[(int)gpuSampler->addressU];
  gpuSampler->glWrapT = GLES3_WRAPS[(int)gpuSampler->addressV];
  gpuSampler->glWrapR = GLES3_WRAPS[(int)gpuSampler->addressW];
}

void GLES2CmdFuncDestroySampler(GLES2Device* device, GLES2GPUSampler* gpuSampler) {
}

void GLES2CmdFuncCreateShader(GLES2Device* device, GLES2GPUShader* gpuShader) {
  GLenum gl_shader_type = 0;
  String shader_type_str;
  GLint status;
  
  for (size_t i = 0; i < gpuShader->gpuStages.size(); ++i) {
    GLES2GPUShaderStage& gpu_stage = gpuShader->gpuStages[i];
    
    switch (gpu_stage.type) {
      case GFXShaderType::VERTEX: {
        gl_shader_type = GL_VERTEX_SHADER;
        shader_type_str = "Vertex Shader";
        break;
      }
      case GFXShaderType::FRAGMENT: {
        gl_shader_type = GL_FRAGMENT_SHADER;
        shader_type_str = "Fragment Shader";
        break;
      }
      default: {
        CCASSERT(false, "Unsupported GFXShaderType");
        return;
      }
    }
    
    gpu_stage.glShader = glCreateShader(gl_shader_type);
    const char* shader_src = gpu_stage.source.c_str();
    glShaderSource(gpu_stage.glShader, 1, (const GLchar**)&shader_src, nullptr);
    glCompileShader(gpu_stage.glShader);
    
    glGetShaderiv(gpu_stage.glShader, GL_COMPILE_STATUS, &status);
    if (status != 1) {
      GLint log_size = 0;
      glGetShaderiv(gpu_stage.glShader, GL_INFO_LOG_LENGTH, &log_size);
      
      ++log_size;
      GLchar* logs = (GLchar*)CC_MALLOC(log_size);
      glGetShaderInfoLog(gpu_stage.glShader, log_size, nullptr, logs);
      
      CC_LOG_ERROR("%s in %s compilation failed.", shader_type_str.c_str(), gpuShader->name.c_str());
      CC_LOG_ERROR("Shader source:%s", gpu_stage.source.c_str());
      CC_LOG_ERROR(logs);
      CC_FREE(logs);
      glDeleteShader(gpu_stage.glShader);
      gpu_stage.glShader = 0;
      return;
    }
  }
  
  gpuShader->glProgram = glCreateProgram();
  
  // link program
  for (size_t i = 0; i < gpuShader->gpuStages.size(); ++i) {
    GLES2GPUShaderStage& gpu_stage = gpuShader->gpuStages[i];
    glAttachShader(gpuShader->glProgram, gpu_stage.glShader);
  }
  
  glLinkProgram(gpuShader->glProgram);

  // detach & delete immediately
  for (size_t i = 0; i < gpuShader->gpuStages.size(); ++i) {
    GLES2GPUShaderStage& gpu_stage = gpuShader->gpuStages[i];
    if (gpu_stage.glShader) {
      glDetachShader(gpuShader->glProgram, gpu_stage.glShader);
      glDeleteShader(gpu_stage.glShader);
      gpu_stage.glShader = 0;
    }
  }

  glGetProgramiv(gpuShader->glProgram, GL_LINK_STATUS, &status);
  if (status != 1) {
    CC_LOG_ERROR("Failed to link Shader [%s].", gpuShader->name.c_str());
    GLint log_size = 0;
    glGetProgramiv(gpuShader->glProgram, GL_INFO_LOG_LENGTH, &log_size);
    if (log_size) {
      ++log_size;
      GLchar* logs = (GLchar*)CC_MALLOC(log_size);
      glGetProgramInfoLog(gpuShader->glProgram, log_size, nullptr, logs);
      
      CC_LOG_ERROR("Failed to link shader '%s'.", gpuShader->name.c_str());
      CC_LOG_ERROR(logs);
      CC_FREE(logs);
      return;
    }
  }
  
  CC_LOG_INFO("Shader '%s' compilation successed.", gpuShader->name.c_str());
  
  GLint attr_max_length = 0;
  GLint attr_count = 0;
  glGetProgramiv(gpuShader->glProgram, GL_ACTIVE_ATTRIBUTE_MAX_LENGTH, &attr_max_length);
  glGetProgramiv(gpuShader->glProgram, GL_ACTIVE_ATTRIBUTES, &attr_count);
  
  GLchar gl_name[256];
  GLsizei gl_length;
  GLsizei gl_size;
  GLenum glType;
  
  gpuShader->glInputs.resize(attr_count);
  for (GLint i = 0; i < attr_count; ++i) {
    GLES2GPUInput& gpuInput = gpuShader->glInputs[i];
    
    memset(gl_name, 0, sizeof(gl_name));
    glGetActiveAttrib(gpuShader->glProgram, i, attr_max_length, &gl_length, &gl_size, &glType, gl_name);
    char* offset = strchr(gl_name, '[');
    if (offset) {
      gl_name[offset - gl_name] = '\0';
    }
    
    gpuInput.glLoc = glGetAttribLocation(gpuShader->glProgram, gl_name);
    gpuInput.binding = gpuInput.glLoc;
    gpuInput.name = gl_name;
    gpuInput.type = MapGFXType(glType);
    gpuInput.stride = GLTypeSize(glType);
    gpuInput.count = gl_size;
    gpuInput.size = gpuInput.stride * gpuInput.count;
    gpuInput.glType = glType;
  }
  
  // create uniform blocks
  if (gpuShader->blocks.size()) {

    gpuShader->glBlocks.resize(gpuShader->blocks.size());

    for (size_t i = 0; i < gpuShader->glBlocks.size(); ++i) {
      GLES2GPUUniformBlock& gpuBlock = gpuShader->glBlocks[i];
      GFXUniformBlock& block = gpuShader->blocks[i];

      gpuBlock.name = block.name;
      gpuBlock.binding = block.binding;
      gpuBlock.glUniforms.resize(block.uniforms.size());

      for (size_t j = 0; j < gpuBlock.glUniforms.size(); ++j) {
        GLES2GPUUniform& gpuUniform = gpuBlock.glUniforms[j];
        GFXUniform& uniform = block.uniforms[j];

        gpuUniform.binding = GFX_INVALID_BINDING;
        gpuUniform.name = uniform.name;
        gpuUniform.type = uniform.type;
        gpuUniform.stride = GFX_TYPE_SIZES[(int)uniform.type];
        gpuUniform.count = uniform.count;
        gpuUniform.size = gpuUniform.stride * gpuUniform.count;
        gpuUniform.offset = gpuBlock.size;
        gpuUniform.glType = MapGLType(gpuUniform.type);
        gpuUniform.glLoc = -1;
        gpuUniform.buff = nullptr;

        gpuBlock.size += gpuUniform.size;
      }
    }
  } // if

  // create uniform samplers
  if (gpuShader->samplers.size()) {
    gpuShader->glSamplers.resize(gpuShader->samplers.size());

    for (size_t i = 0; i < gpuShader->glSamplers.size(); ++i) {
      GFXUniformSampler& sampler = gpuShader->samplers[i];
      GLES2GPUUniformSampler& gpuSampler = gpuShader->glSamplers[i];
      gpuSampler.binding = sampler.binding;
      gpuSampler.name = sampler.name;
      gpuSampler.type = sampler.type;
      gpuSampler.glType = MapGLType(gpuSampler.type);
      gpuSampler.glLoc = -1;
    }
  }

  // parse glUniforms
  GLint glActiveUniforms;
  glGetProgramiv(gpuShader->glProgram, GL_ACTIVE_UNIFORMS, &glActiveUniforms);

  GLint unit_idx = 0;

  GLES2GPUUniformSamplerList active_gpu_samplers;

  for (GLint i = 0; i < glActiveUniforms; ++i) {
    memset(gl_name, 0, sizeof(gl_name));
    glGetActiveUniform(gpuShader->glProgram, i, 255, &gl_length, &gl_size, &glType, gl_name);
    char* u_offset = strchr(gl_name, '[');
    if (u_offset) {
      gl_name[u_offset - gl_name] = '\0';
    }
    String u_name = gl_name;

    bool is_sampler = (glType == GL_SAMPLER_2D) ||
      (glType == GL_SAMPLER_3D_OES) ||
      (glType == GL_SAMPLER_CUBE) ||
      (glType == GL_SAMPLER_CUBE_MAP_ARRAY_OES) ||
      (glType == GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW_OES) ||
      (glType == GL_INT_SAMPLER_CUBE_MAP_ARRAY_OES) ||
      (glType == GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY_OES);
    if (is_sampler) {
      for (size_t s = 0; s < gpuShader->glSamplers.size(); ++s) {
        GLES2GPUUniformSampler& gpuSampler = gpuShader->glSamplers[s];
        if (gpuSampler.name == u_name) {
          gpuSampler.units.resize(gl_size);
          for (GLsizei u = 0; u < gl_size; ++u) {
            gpuSampler.units[u] = unit_idx + u;
          }

          gpuSampler.glLoc = glGetUniformLocation(gpuShader->glProgram, gl_name);
          unit_idx += gl_size;

          active_gpu_samplers.push_back(gpuSampler);
          break;
        }
      }
    } else {
      for (size_t b = 0; b < gpuShader->glBlocks.size(); ++b) {
        GLES2GPUUniformBlock& gpuBlock = gpuShader->glBlocks[b];
        for (size_t u = 0; u < gpuBlock.glUniforms.size(); ++u) {
          if (gpuBlock.glUniforms[u].name == u_name) {
            GLES2GPUUniform& gpuUniform = gpuBlock.glUniforms[u];
            gpuUniform.glLoc = glGetUniformLocation(gpuShader->glProgram, gl_name);
            gpuUniform.buff = (uint8_t*)CC_MALLOC(gpuUniform.size);

            gpuBlock.glActiveUniforms.emplace_back(gpuUniform);
            break;
          }
        }
      }
    }
  } // for

  if (active_gpu_samplers.size()) {
    if (device->stateCache->glProgram != gpuShader->glProgram) {
      glUseProgram(gpuShader->glProgram);
      device->stateCache->glProgram = gpuShader->glProgram;
    }

    for (size_t i = 0; i < active_gpu_samplers.size(); ++i) {
      GLES2GPUUniformSampler& gpuSampler = active_gpu_samplers[i];
      glUniform1iv(gpuSampler.glLoc, (GLsizei)gpuSampler.units.size(), gpuSampler.units.data());
    }
  }
}

void GLES2CmdFuncDestroyShader(GLES2Device* device, GLES2GPUShader* gpuShader) {
  if (gpuShader->glProgram) {
    if (device->stateCache->glProgram == gpuShader->glProgram) {
      glUseProgram(0);
      device->stateCache->glProgram = 0;
    }
    glDeleteProgram(gpuShader->glProgram);
    gpuShader->glProgram = 0;
  }
}

void GLES2CmdFuncCreateInputAssembler(GLES2Device* device, GLES2GPUInputAssembler* gpuInputAssembler) {
  
  if (gpuInputAssembler->gpuIndexBuffer) {
    switch (gpuInputAssembler->gpuIndexBuffer->stride) {
      case 1: gpuInputAssembler->glIndexType = GL_UNSIGNED_BYTE; break;
      case 2: gpuInputAssembler->glIndexType = GL_UNSIGNED_SHORT; break;
      case 4: gpuInputAssembler->glIndexType = GL_UNSIGNED_INT; break;
      default: {
        CC_LOG_ERROR("Illegal index buffer stride.");
      }
    }
  }
  
  uint stream_offsets[GFX_MAX_VERTEX_ATTRIBUTES] = {0};
  
  gpuInputAssembler->glAttribs.resize(gpuInputAssembler->attributes.size());
  for (size_t i = 0; i < gpuInputAssembler->glAttribs.size(); ++i) {
    GLES2GPUAttribute& gpuAttribute = gpuInputAssembler->glAttribs[i];
    const GFXAttribute& attrib = gpuInputAssembler->attributes[i];
    
    GLES2GPUBuffer* gpu_vb = (GLES2GPUBuffer*)gpuInputAssembler->gpuVertexBuffers[attrib.stream];
    
    gpuAttribute.name = attrib.name;
    gpuAttribute.glType = GFXFormatToGLType(attrib.format);
    gpuAttribute.size = GFX_FORMAT_INFOS[(int)attrib.format].size;
    gpuAttribute.count = GFX_FORMAT_INFOS[(int)attrib.format].count;
    gpuAttribute.componentCount = GLComponentCount(gpuAttribute.glType);
    gpuAttribute.isNormalized = attrib.isNormalized;
    gpuAttribute.isInstanced = attrib.isInstanced;
    gpuAttribute.offset = stream_offsets[attrib.stream];
    
    if (gpu_vb) {
      gpuAttribute.glBuffer = gpu_vb->glBuffer;
      gpuAttribute.stride = gpu_vb->stride;
    }
      stream_offsets[attrib.stream] += gpuAttribute.size;
  }
}

void GLES2CmdFuncDestroyInputAssembler(GLES2Device* device, GLES2GPUInputAssembler* gpuInputAssembler) {
  for (auto it = gpuInputAssembler->glVAOs.begin(); it != gpuInputAssembler->glVAOs.end(); ++it) {
    if (device->stateCache->glVAO == it->second) {
      glBindVertexArrayOES(0);
      device->stateCache->glVAO = 0;
    }
    glDeleteVertexArraysOES(1, &it->second);
  }
  gpuInputAssembler->glVAOs.clear();
}

void GLES2CmdFuncCreateFramebuffer(GLES2Device* device, GLES2GPUFramebuffer* gpuFBO) {
  if (gpuFBO->isOffscreen) {
    glGenFramebuffers(1, &gpuFBO->glFramebuffer);
    if (device->stateCache->glFramebuffer != gpuFBO->glFramebuffer) {
      glBindFramebuffer(GL_FRAMEBUFFER, gpuFBO->glFramebuffer);
      device->stateCache->glFramebuffer = gpuFBO->glFramebuffer;
    }

    GLenum attachments[GFX_MAX_ATTACHMENTS] = {0};
    uint attachment_count = 0;
    
    for (size_t i = 0; i < gpuFBO->gpuColorViews.size(); ++i) {
      GLES2GPUTextureView* gpuColorView = gpuFBO->gpuColorViews[i];
      if (gpuColorView && gpuColorView->gpuTexture) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, (GLenum)(GL_COLOR_ATTACHMENT0 + i), gpuColorView->gpuTexture->glTarget, gpuColorView->gpuTexture->glTexture, gpuColorView->baseLevel);
        
        attachments[attachment_count++] = (GLenum)(GL_COLOR_ATTACHMENT0 + i);
      }
    }
    
    if (gpuFBO->gpuDepthStencilView) {
      GLES2GPUTextureView* gpuDepthStencilView = gpuFBO->gpuDepthStencilView;
      glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, gpuDepthStencilView->gpuTexture->glTarget, gpuDepthStencilView->gpuTexture->glTexture, gpuDepthStencilView->baseLevel);
      
      if (GFX_FORMAT_INFOS[(int)gpuDepthStencilView->format].hasStencil) {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, gpuDepthStencilView->gpuTexture->glTarget, gpuDepthStencilView->gpuTexture->glTexture, gpuDepthStencilView->baseLevel);
      }
    }
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE) {
      switch (status) {
        case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT: {
          CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT: {
          CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
          break;
        }
        case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS: {
          CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
          break;
        }
        case GL_FRAMEBUFFER_UNSUPPORTED: {
          CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_UNSUPPORTED");
          break;
        }
        default:;
      }
    }
  }
}

void GLES2CmdFuncDestroyFramebuffer(GLES2Device* device, GLES2GPUFramebuffer* gpuFBO) {
  if (gpuFBO->glFramebuffer) {
    if (device->stateCache->glFramebuffer == gpuFBO->glFramebuffer) {
      glBindFramebuffer(GL_FRAMEBUFFER, 0);
      device->stateCache->glFramebuffer = 0;
    }
    glDeleteFramebuffers(1, &gpuFBO->glFramebuffer);
    gpuFBO->glFramebuffer = 0;
  }
}

void GLES2CmdFuncExecuteCmds(GLES2Device* device, GLES2CmdPackage* cmd_package) {
  static uint cmd_indices[(int)GFXCmdType::COUNT] = {0};
  static GLenum gl_attachments[GFX_MAX_ATTACHMENTS] = {0};
  
  memset(cmd_indices, 0, sizeof(cmd_indices));
  
  GLES2StateCache* cache = device->stateCache;
  GLES2GPURenderPass* gpuRenderPass = nullptr;
  bool is_shader_changed = false;
  GLES2GPUPipelineState* gpuPipelineState = nullptr;
  GLenum glPrimitive = 0;
  GLES2GPUInputAssembler* gpuInputAssembler = nullptr;
  GLES2CmdBeginRenderPass* cmdBeginRenderPass = nullptr;
  GLenum glWrapS;
  GLenum glWrapT;
  GLenum glMinFilter;
  
  for (uint i = 0; i < cmd_package->cmds.size(); ++i) {
    GFXCmdType cmd_type = cmd_package->cmds[i];
    uint& cmd_idx = cmd_indices[(int)cmd_type];
    
    switch (cmd_type) {
      case GFXCmdType::BEGIN_RENDER_PASS: {
        GLES2CmdBeginRenderPass* cmd = cmd_package->beginRenderPassCmds[cmd_idx];
        cmdBeginRenderPass = cmd;
        if (cmd->gpuFBO) {
          if (cache->glFramebuffer != cmd->gpuFBO->glFramebuffer) {
            glBindFramebuffer(GL_FRAMEBUFFER, cmd->gpuFBO->glFramebuffer);
            cache->glFramebuffer = cmd->gpuFBO->glFramebuffer;
          }
          
          if (cache->viewport.left != cmd->render_area.x ||
              cache->viewport.top != cmd->render_area.y ||
              cache->viewport.width != cmd->render_area.width ||
              cache->viewport.height != cmd->render_area.height) {
            glViewport(cmd->render_area.x, cmd->render_area.y, cmd->render_area.width, cmd->render_area.height);
            cache->viewport.left = cmd->render_area.x;
            cache->viewport.top = cmd->render_area.y;
            cache->viewport.width = cmd->render_area.width;
            cache->viewport.height = cmd->render_area.height;
          }
          
          if (cache->scissor.x != cmd->render_area.x ||
              cache->scissor.y != cmd->render_area.y ||
              cache->scissor.width != cmd->render_area.width ||
              cache->scissor.height != cmd->render_area.height) {
            glScissor(cmd->render_area.x, cmd->render_area.y, cmd->render_area.width, cmd->render_area.height);
            cache->scissor.x = cmd->render_area.x;
            cache->scissor.y = cmd->render_area.y;
            cache->scissor.width = cmd->render_area.width;
            cache->scissor.height = cmd->render_area.height;
          }
          
          GLbitfield gl_clears = 0;
          uint num_attachments = 0;
          
          gpuRenderPass = cmd->gpuFBO->gpuRenderPass;
          for (uint j = 0; j < cmd->num_clear_colors; ++j) {
            const GFXColorAttachment& colorAttachment = gpuRenderPass->colorAttachments[j];
            if (colorAttachment.format != GFXFormat::UNKNOWN) {
              switch (colorAttachment.loadOp) {
                case GFXLoadOp::LOAD: break; // GL default behaviour
                case GFXLoadOp::CLEAR: {
                  if (cmd->clear_flags & GFXClearFlagBit::COLOR) {
                    if (cache->bs.targets[0].blendColorMask != GFXColorMask::ALL) {
                      glColorMask(true, true, true, true);
                    }
                    
                    const GFXColor& color = cmd->clear_colors[j];
                    glClearColor(color.r, color.g, color.b, color.a);
                    gl_clears |= GL_COLOR_BUFFER_BIT;
                  }
                  break;
                }
                case GFXLoadOp::DISCARD: {
                  // invalidate fbo
                  gl_attachments[num_attachments++] = (cmd->gpuFBO->isOffscreen ? GL_COLOR_ATTACHMENT0 + j : GL_COLOR_EXT);
                  break;
                }
                default:;
              }
            }
          } // for
          
          if (gpuRenderPass->depthStencilAttachment.format != GFXFormat::UNKNOWN) {
            bool hasDepth = GFX_FORMAT_INFOS[(int)gpuRenderPass->depthStencilAttachment.format].hasDepth;
            if (hasDepth) {
              switch (gpuRenderPass->depthStencilAttachment.depthLoadOp) {
                case GFXLoadOp::LOAD: break; // GL default behaviour
                case GFXLoadOp::CLEAR: {
                    glDepthMask(true);
                    cache->dss.depthWrite = true;
                  glClearDepthf(cmd->clear_depth);
                  gl_clears |= GL_DEPTH_BUFFER_BIT;
                  break;
                }
                case GFXLoadOp::DISCARD: {
                  // invalidate fbo
                  gl_attachments[num_attachments++] = (cmd->gpuFBO->isOffscreen ? GL_DEPTH_ATTACHMENT : GL_DEPTH_EXT);
                  break;
                }
                default:;
              }
            } // if (hasDepth)
            bool has_stencils = GFX_FORMAT_INFOS[(int)gpuRenderPass->depthStencilAttachment.format].hasStencil;
            if (has_stencils) {
              switch (gpuRenderPass->depthStencilAttachment.depthLoadOp) {
                case GFXLoadOp::LOAD: break; // GL default behaviour
                case GFXLoadOp::CLEAR: {
                  if (!cache->dss.stencilWriteMaskFront) {
                    glStencilMaskSeparate(GL_FRONT, 0xffffffff);
                  }
                  if (!cache->dss.stencilWriteMaskBack) {
                    glStencilMaskSeparate(GL_BACK, 0xffffffff);
                  }
                  glClearStencil(cmd->clear_stencil);
                  gl_clears |= GL_STENCIL_BUFFER_BIT;
                  break;
                }
                case GFXLoadOp::DISCARD: {
                  // invalidate fbo
                  gl_attachments[num_attachments++] = (cmd->gpuFBO->isOffscreen ? GL_STENCIL_ATTACHMENT : GL_STENCIL_EXT);
                  break;
                }
                default:;
              }
            } // if (has_stencils)
          } // if
          
          if (num_attachments && device->useDiscardFramebuffer()) {
            glDiscardFramebufferEXT(GL_FRAMEBUFFER, num_attachments, gl_attachments);
          }
          
          if (gl_clears) {
            glClear(gl_clears);
          }
          
          // restore states
          if (gl_clears & GL_COLOR_BUFFER_BIT) {
            GFXColorMask color_mask = cache->bs.targets[0].blendColorMask;
            if (color_mask != GFXColorMask::ALL) {
              glColorMask((GLboolean)(color_mask & GFXColorMask::R),
                (GLboolean)(color_mask & GFXColorMask::G),
                (GLboolean)(color_mask & GFXColorMask::B),
                (GLboolean)(color_mask & GFXColorMask::A));
            }
          }
          
          if ((gl_clears & GL_COLOR_BUFFER_BIT) && !cache->dss.depthWrite) {
            glDepthMask(false);
          }
          
          if (gl_clears & GL_STENCIL_BUFFER_BIT) {
            if (!cache->dss.stencilWriteMaskFront) {
              glStencilMaskSeparate(GL_FRONT, 0);
            }
            if (!cache->dss.stencilWriteMaskBack) {
              glStencilMaskSeparate(GL_BACK, 0);
            }
          }
        }
        break;
      }
      case GFXCmdType::END_RENDER_PASS: {
        GLES2CmdBeginRenderPass* cmd = cmdBeginRenderPass;
        uint num_attachments = 0;
        for (uint j = 0; j < cmd->num_clear_colors; ++j) {
          const GFXColorAttachment& colorAttachment = gpuRenderPass->colorAttachments[j];
          if (colorAttachment.format != GFXFormat::UNKNOWN) {
            switch (colorAttachment.loadOp) {
              case GFXLoadOp::LOAD: break; // GL default behaviour
              case GFXLoadOp::CLEAR: break;
              case GFXLoadOp::DISCARD: {
                // invalidate fbo
                gl_attachments[num_attachments++] = (cmd->gpuFBO->isOffscreen ? GL_COLOR_ATTACHMENT0 + j : GL_COLOR_EXT);
                break;
              }
              default:;
            }
          }
        } // for
        
        if (gpuRenderPass->depthStencilAttachment.format != GFXFormat::UNKNOWN) {
          bool hasDepth = GFX_FORMAT_INFOS[(int)gpuRenderPass->depthStencilAttachment.format].hasDepth;
          if (hasDepth) {
            switch (gpuRenderPass->depthStencilAttachment.depthLoadOp) {
              case GFXLoadOp::LOAD: break; // GL default behaviour
              case GFXLoadOp::CLEAR: break;
              case GFXLoadOp::DISCARD: {
                // invalidate fbo
                gl_attachments[num_attachments++] = (cmd->gpuFBO->isOffscreen ? GL_DEPTH_ATTACHMENT : GL_DEPTH_EXT);
                break;
              }
              default:;
            }
          } // if (hasDepth)
          bool has_stencils = GFX_FORMAT_INFOS[(int)gpuRenderPass->depthStencilAttachment.format].hasStencil;
          if (has_stencils) {
            switch (gpuRenderPass->depthStencilAttachment.depthLoadOp) {
              case GFXLoadOp::LOAD: break; // GL default behaviour
              case GFXLoadOp::CLEAR: break;
              case GFXLoadOp::DISCARD: {
                // invalidate fbo
                gl_attachments[num_attachments++] = (cmd->gpuFBO->isOffscreen ? GL_STENCIL_ATTACHMENT : GL_STENCIL_EXT);
                break;
              }
              default:;
            }
          } // if (has_stencils)
        } // if
        
        if (num_attachments && device->useDiscardFramebuffer()) {
          glDiscardFramebufferEXT(GL_FRAMEBUFFER, num_attachments, gl_attachments);
        }
        break;
      }
      case GFXCmdType::BIND_STATES: {
        GLES2CmdBindStates* cmd = cmd_package->bindStatesCmds[cmd_idx];
        is_shader_changed = false;
          
        if (cmd->gpuPipelineState) {
          gpuPipelineState = cmd->gpuPipelineState;
          glPrimitive = gpuPipelineState->glPrimitive;
          
          if (gpuPipelineState->gpuShader) {
            if (cache->glProgram != gpuPipelineState->gpuShader->glProgram) {
              glUseProgram(gpuPipelineState->gpuShader->glProgram);
              cache->glProgram = gpuPipelineState->gpuShader->glProgram;
              is_shader_changed = true;
            }
          }
          
          // bind rasterizer state
          if (cache->rs.cullMode != gpuPipelineState->rs.cullMode) {
            switch (gpuPipelineState->rs.cullMode) {
              case GFXCullMode::NONE: {
                if (cache->isCullFaceEnabled) {
                  glDisable(GL_CULL_FACE);
                  cache->isCullFaceEnabled = false;
                }
              } break;
              case GFXCullMode::FRONT: {
                if (!cache->isCullFaceEnabled) {
                  glEnable(GL_CULL_FACE);
                  cache->isCullFaceEnabled = true;
                }
                glCullFace(GL_FRONT);
              } break;
              case GFXCullMode::BACK: {
                if (!cache->isCullFaceEnabled) {
                  glEnable(GL_CULL_FACE);
                  cache->isCullFaceEnabled = true;
                }
                glCullFace(GL_BACK);
              } break;
              default:
                break;
            }
            cache->rs.cullMode = gpuPipelineState->rs.cullMode;
          }
        }
        if (cache->rs.isFrontFaceCCW != gpuPipelineState->rs.isFrontFaceCCW) {
          glFrontFace(gpuPipelineState->rs.isFrontFaceCCW? GL_CCW : GL_CW);
          cache->rs.isFrontFaceCCW = gpuPipelineState->rs.isFrontFaceCCW;
        }
        if ((cache->rs.depthBias != gpuPipelineState->rs.depthBias) ||
            (cache->rs.depthBiasSlop != gpuPipelineState->rs.depthBiasSlop)){
          glPolygonOffset(cache->rs.depthBias, cache->rs.depthBiasSlop);
          cache->rs.depthBiasSlop = gpuPipelineState->rs.depthBiasSlop;
        }
        if (cache->rs.lineWidth != gpuPipelineState->rs.lineWidth) {
          glLineWidth(gpuPipelineState->rs.lineWidth);
          cache->rs.lineWidth = gpuPipelineState->rs.lineWidth;
        }
        
        // bind depth-stencil state
        if (cache->dss.depthTest != gpuPipelineState->dss.depthTest) {
          if (gpuPipelineState->dss.depthTest) {
              glEnable(GL_DEPTH_TEST);
          } else {
              glDisable(GL_DEPTH_TEST);
          }
          cache->dss.depthTest = gpuPipelineState->dss.depthTest;
        }
        if (cache->dss.depthWrite != gpuPipelineState->dss.depthWrite) {
          glDepthMask(gpuPipelineState->dss.depthWrite);
          cache->dss.depthWrite = gpuPipelineState->dss.depthWrite;
        }
        if (cache->dss.depthFunc != gpuPipelineState->dss.depthFunc) {
          glDepthFunc(GLES2_CMP_FUNCS[(int)gpuPipelineState->dss.depthFunc]);
          cache->dss.depthFunc = gpuPipelineState->dss.depthFunc;
        }
        
        // bind depth-stencil state - front
          if (gpuPipelineState->dss.stencilTestFront || gpuPipelineState->dss.stencilTestBack) {
            if (!cache->isStencilTestEnabled) {
              glEnable(GL_STENCIL_TEST);
              cache->isStencilTestEnabled = true;
            }
          } else {
            if (cache->isStencilTestEnabled) {
              glDisable(GL_STENCIL_TEST);
              cache->isStencilTestEnabled = false;
            }
          }
        if (cache->dss.stencilFuncFront != gpuPipelineState->dss.stencilFuncFront ||
             cache->dss.stencilRefFront != gpuPipelineState->dss.stencilRefFront ||
             cache->dss.stencilReadMaskFront != gpuPipelineState->dss.stencilReadMaskFront) {
             glStencilFuncSeparate(GL_FRONT,
                                GLES2_CMP_FUNCS[(int)gpuPipelineState->dss.stencilFuncFront],
                                gpuPipelineState->dss.stencilRefFront,
                                gpuPipelineState->dss.stencilReadMaskFront);
             cache->dss.stencilFuncFront = gpuPipelineState->dss.stencilFuncFront;
             cache->dss.stencilRefFront = gpuPipelineState->dss.stencilRefFront;
             cache->dss.stencilReadMaskFront = gpuPipelineState->dss.stencilReadMaskFront;
        }
        if (cache->dss.stencilFailOpFront != gpuPipelineState->dss.stencilFailOpFront ||
             cache->dss.stencilZFailOpFront != gpuPipelineState->dss.stencilZFailOpFront ||
             cache->dss.stencilPassOpFront != gpuPipelineState->dss.stencilPassOpFront) {
             glStencilOpSeparate(GL_FRONT,
                              GLES2_STENCIL_OPS[(int)gpuPipelineState->dss.stencilFailOpFront],
                              GLES2_STENCIL_OPS[(int)gpuPipelineState->dss.stencilZFailOpFront],
                              GLES2_STENCIL_OPS[(int)gpuPipelineState->dss.stencilPassOpFront]);
             cache->dss.stencilFailOpFront = gpuPipelineState->dss.stencilFailOpFront;
             cache->dss.stencilZFailOpFront = gpuPipelineState->dss.stencilZFailOpFront;
             cache->dss.stencilPassOpFront = gpuPipelineState->dss.stencilPassOpFront;
        }
        if (cache->dss.stencilWriteMaskFront != gpuPipelineState->dss.stencilWriteMaskFront) {
          glStencilMaskSeparate(GL_FRONT, gpuPipelineState->dss.stencilWriteMaskFront);
          cache->dss.stencilWriteMaskFront = gpuPipelineState->dss.stencilWriteMaskFront;
        }
        
        // bind depth-stencil state - back
        if (cache->dss.stencilFuncBack != gpuPipelineState->dss.stencilFuncBack ||
            cache->dss.stencilRefBack != gpuPipelineState->dss.stencilRefBack ||
            cache->dss.stencilReadMaskBack != gpuPipelineState->dss.stencilReadMaskBack) {
            glStencilFuncSeparate(GL_BACK,
                                GLES2_CMP_FUNCS[(int)gpuPipelineState->dss.stencilFuncBack],
                                gpuPipelineState->dss.stencilRefBack,
                                gpuPipelineState->dss.stencilReadMaskBack);
            cache->dss.stencilFuncBack = gpuPipelineState->dss.stencilFuncBack;
            cache->dss.stencilRefBack = gpuPipelineState->dss.stencilRefBack;
            cache->dss.stencilReadMaskBack = gpuPipelineState->dss.stencilReadMaskBack;
        }
        if (cache->dss.stencilFailOpBack != gpuPipelineState->dss.stencilFailOpBack ||
            cache->dss.stencilZFailOpBack != gpuPipelineState->dss.stencilZFailOpBack ||
            cache->dss.stencilPassOpBack != gpuPipelineState->dss.stencilPassOpBack) {
            glStencilOpSeparate(GL_BACK,
                              GLES2_STENCIL_OPS[(int)gpuPipelineState->dss.stencilFailOpBack],
                              GLES2_STENCIL_OPS[(int)gpuPipelineState->dss.stencilZFailOpBack],
                              GLES2_STENCIL_OPS[(int)gpuPipelineState->dss.stencilPassOpBack]);
            cache->dss.stencilFailOpBack = gpuPipelineState->dss.stencilFailOpBack;
            cache->dss.stencilZFailOpBack = gpuPipelineState->dss.stencilZFailOpBack;
            cache->dss.stencilPassOpBack = gpuPipelineState->dss.stencilPassOpBack;
        }
        if (cache->dss.stencilWriteMaskBack != gpuPipelineState->dss.stencilWriteMaskBack) {
          glStencilMaskSeparate(GL_BACK, gpuPipelineState->dss.stencilWriteMaskBack);
          cache->dss.stencilWriteMaskBack = gpuPipelineState->dss.stencilWriteMaskBack;
        }
        
        // bind blend state
        if (cache->bs.isA2C != gpuPipelineState->bs.isA2C) {
          if (cache->bs.isA2C) {
            glEnable(GL_SAMPLE_ALPHA_TO_COVERAGE);
          } else {
            glDisable(GL_SAMPLE_ALPHA_TO_COVERAGE);
          }
          cache->bs.isA2C = gpuPipelineState->bs.isA2C;
        }
        if (cache->bs.blendColor.r != gpuPipelineState->bs.blendColor.r ||
            cache->bs.blendColor.g != gpuPipelineState->bs.blendColor.g ||
            cache->bs.blendColor.b != gpuPipelineState->bs.blendColor.b ||
            cache->bs.blendColor.a != gpuPipelineState->bs.blendColor.a) {
          
          glBlendColor(gpuPipelineState->bs.blendColor.r,
                       gpuPipelineState->bs.blendColor.g,
                       gpuPipelineState->bs.blendColor.b,
                       gpuPipelineState->bs.blendColor.a);
          cache->bs.blendColor = gpuPipelineState->bs.blendColor;
        }
        
        GFXBlendTarget& cache_target = cache->bs.targets[0];
        const GFXBlendTarget& target = gpuPipelineState->bs.targets[0];
        if (cache_target.blend != target.blend) {
          if (!cache_target.blend) {
            glEnable(GL_BLEND);
          } else {
            glDisable(GL_BLEND);
          }
          cache_target.blend = target.blend;
        }
        if (cache_target.blendEq != target.blendEq ||
            cache_target.blendAlphaEq != target.blendAlphaEq) {
          glBlendEquationSeparate(GLES2_BLEND_OPS[(int)target.blendEq],
                                  GLES2_BLEND_OPS[(int)target.blendAlphaEq]);
          cache_target.blendEq = target.blendEq;
          cache_target.blendAlphaEq = target.blendAlphaEq;
        }
        if (cache_target.blendSrc != target.blendSrc ||
            cache_target.blendDst != target.blendDst ||
            cache_target.blendSrcAlpha != target.blendSrcAlpha ||
            cache_target.blendDstAlpha != target.blendDstAlpha) {
          glBlendFuncSeparate(GLES2_BLEND_FACTORS[(int)target.blendSrc],
                              GLES2_BLEND_FACTORS[(int)target.blendDst],
                              GLES2_BLEND_FACTORS[(int)target.blendSrcAlpha],
                              GLES2_BLEND_FACTORS[(int)target.blendDstAlpha]);
          cache_target.blendSrc = target.blendSrc;
          cache_target.blendDst = target.blendDst;
          cache_target.blendSrcAlpha = target.blendSrcAlpha;
          cache_target.blendDstAlpha = target.blendDstAlpha;
        }
        if (cache_target.blendColorMask != target.blendColorMask) {
          glColorMask((GLboolean)(target.blendColorMask & GFXColorMask::R),
            (GLboolean)(target.blendColorMask & GFXColorMask::G),
            (GLboolean)(target.blendColorMask & GFXColorMask::B),
            (GLboolean)(target.blendColorMask & GFXColorMask::A));
          cache_target.blendColorMask = target.blendColorMask;
        }
        
        // bind shader resources
        if (cmd->gpuBindingLayout && gpuPipelineState->gpuShader) {
          uint8_t* uniform_buff;

          for(size_t j = 0; j < cmd->gpuBindingLayout->gpuBindings.size(); ++j) {
            const GLES2GPUBinding& gpuBinding = cmd->gpuBindingLayout->gpuBindings[j];
            switch (gpuBinding.type) {
              case GFXBindingType::UNIFORM_BUFFER: {
                if (gpuBinding.gpuBuffer) {
                  for (size_t k = 0; k < gpuPipelineState->gpuShader->glBlocks.size(); ++k) {
                    const GLES2GPUUniformBlock& gpuBlock = gpuPipelineState->gpuShader->glBlocks[k];

                    if (gpuBinding.binding == gpuBlock.binding &&
                        gpuBinding.gpuBuffer &&
                        gpuBinding.gpuBuffer->buffer) {
                      for (size_t u = 0; u < gpuBlock.glActiveUniforms.size(); ++u) {
                        const GLES2GPUUniform& gpuUniform = gpuBlock.glActiveUniforms[u];
                        uniform_buff = (gpuBinding.gpuBuffer->buffer + gpuUniform.offset);
                        switch (gpuUniform.glType)
                        {
                        case GL_BOOL:
                        case GL_INT: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform1iv(gpuUniform.glLoc, gpuUniform.count, (const GLint*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_BOOL_VEC2:
                        case GL_INT_VEC2: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform2iv(gpuUniform.glLoc, gpuUniform.count, (const GLint*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_BOOL_VEC3:
                        case GL_INT_VEC3: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform3iv(gpuUniform.glLoc, gpuUniform.count, (const GLint*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_BOOL_VEC4:
                        case GL_INT_VEC4: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform4iv(gpuUniform.glLoc, gpuUniform.count, (const GLint*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_FLOAT: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform1fv(gpuUniform.glLoc, gpuUniform.count, (const GLfloat*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_FLOAT_VEC2: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform2fv(gpuUniform.glLoc, gpuUniform.count, (const GLfloat*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_FLOAT_VEC3: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform3fv(gpuUniform.glLoc, gpuUniform.count, (const GLfloat*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_FLOAT_VEC4: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniform4fv(gpuUniform.glLoc, gpuUniform.count, (const GLfloat*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_FLOAT_MAT2: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniformMatrix2fv(gpuUniform.glLoc, gpuUniform.count, GL_FALSE, (const GLfloat*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_FLOAT_MAT3: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniformMatrix3fv(gpuUniform.glLoc, gpuUniform.count, GL_FALSE, (const GLfloat*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        case GL_FLOAT_MAT4: {
                          if (memcmp(gpuUniform.buff, uniform_buff, gpuUniform.size) != 0) {
                            glUniformMatrix4fv(gpuUniform.glLoc, gpuUniform.count, GL_FALSE, (const GLfloat*)uniform_buff);
                            memcpy(gpuUniform.buff, uniform_buff, gpuUniform.size);
                          }
                          break;
                        }
                        default:
                          break;
                        }
                      } // for
                      break;
                    }
                  }
                }
                break;
              }
              case GFXBindingType::SAMPLER: {
                if (gpuBinding.gpuSampler) {
                  for (size_t k = 0; k < gpuPipelineState->gpuShader->glSamplers.size(); ++k) {
                    const GLES2GPUUniformSampler& gpuSampler = gpuPipelineState->gpuShader->glSamplers[k];
                    if (gpuSampler.binding == gpuBinding.binding) {
                      for (size_t u = 0; u < gpuSampler.units.size(); ++u) {
                        uint unit = (uint)gpuSampler.units[u];
                        if (gpuBinding.gpuTexView && (gpuBinding.gpuTexView->gpuTexture->size > 0)) {
                          GLES2GPUTexture* gpuTexture = gpuBinding.gpuTexView->gpuTexture;
                          GLuint glTexture = gpuTexture->glTexture;
                          if (cache->glTextures[unit] != glTexture) {
                            if (cache->texUint != unit) {
                              glActiveTexture(GL_TEXTURE0 + unit);
                              cache->texUint = unit;
                            }
                            glBindTexture(gpuTexture->glTarget, glTexture);
                            cache->glTextures[unit] = glTexture;
                          }

                          if (gpuBinding.gpuTexView->gpuTexture->isPowerOf2) {
                            glWrapS = gpuBinding.gpuSampler->glWrapS;
                            glWrapT = gpuBinding.gpuSampler->glWrapT;

                            if (gpuBinding.gpuTexView->gpuTexture->mipLevel <= 1 &&
                                !(gpuBinding.gpuTexView->gpuTexture->flags & GFXTextureFlagBit::GEN_MIPMAP) &&
                                (gpuBinding.gpuSampler->glMinFilter == GL_LINEAR_MIPMAP_NEAREST ||
                                 gpuBinding.gpuSampler->glMinFilter == GL_LINEAR_MIPMAP_LINEAR)) {
                              glMinFilter = GL_LINEAR;
                            } else {
                              glMinFilter = gpuBinding.gpuSampler->glMinFilter;
                            }
                          } else {
                            glWrapS = GL_CLAMP_TO_EDGE;
                            glWrapT = GL_CLAMP_TO_EDGE;

                            if (gpuBinding.gpuSampler->glMinFilter == GL_LINEAR || 
                                gpuBinding.gpuSampler->glMinFilter == GL_LINEAR_MIPMAP_NEAREST ||
                                gpuBinding.gpuSampler->glMinFilter == GL_LINEAR_MIPMAP_LINEAR) {
                              glMinFilter = GL_LINEAR;
                            } else {
                              glMinFilter = GL_NEAREST;
                            }
                          }

                          if (gpuTexture->glWrapS != glWrapS) {
                            if (cache->texUint != unit) {
                              glActiveTexture(GL_TEXTURE0 + unit);
                              cache->texUint = unit;
                            }
                            glTexParameteri(gpuTexture->glTarget, GL_TEXTURE_WRAP_S, glWrapS);
                            gpuTexture->glWrapS = glWrapS;
                          }

                          if (gpuTexture->glWrapT != glWrapT) {
                            if (cache->texUint != unit) {
                              glActiveTexture(GL_TEXTURE0 + unit);
                              cache->texUint = unit;
                            }
                            glTexParameteri(gpuTexture->glTarget, GL_TEXTURE_WRAP_T, glWrapT);
                            gpuTexture->glWrapT = glWrapT;
                          }

                          if (gpuTexture->glMinFilter != glMinFilter) {
                            if (cache->texUint != unit) {
                              glActiveTexture(GL_TEXTURE0 + unit);
                              cache->texUint = unit;
                            }
                            glTexParameteri(gpuTexture->glTarget, GL_TEXTURE_MIN_FILTER, glMinFilter);
                            gpuTexture->glMinFilter = glMinFilter;
                          }

                          if (gpuTexture->glMagFilter != gpuBinding.gpuSampler->glMagFilter) {
                            if (cache->texUint != unit) {
                              glActiveTexture(GL_TEXTURE0 + unit);
                              cache->texUint = unit;
                            }
                            glTexParameteri(gpuTexture->glTarget, GL_TEXTURE_MAG_FILTER, gpuBinding.gpuSampler->glMagFilter);
                            gpuTexture->glMagFilter = gpuBinding.gpuSampler->glMagFilter;
                          }
                        }
                      }
                      break;
                    }
                  }
                }
                break;
              }
              default:
                break;
            }
          }
        } // if
        
        // bind vao
        if (cmd->gpuInputAssembler && gpuPipelineState->gpuShader &&
            (is_shader_changed || gpuInputAssembler != cmd->gpuInputAssembler)) {
          gpuInputAssembler = cmd->gpuInputAssembler;
          if (device->useVAO()) {
            GLuint glVAO = gpuInputAssembler->glVAOs[gpuPipelineState->gpuShader->glProgram];
            if (!glVAO) {
              glGenVertexArraysOES(1, &glVAO);
              gpuInputAssembler->glVAOs[gpuPipelineState->gpuShader->glProgram] = glVAO;
              glBindVertexArrayOES(glVAO);
              glBindBuffer(GL_ARRAY_BUFFER, 0);
              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

              for (size_t j = 0; j < gpuPipelineState->gpuShader->glInputs.size(); ++j) {
                const GLES2GPUInput& gpuInput = gpuPipelineState->gpuShader->glInputs[j];
                for (size_t a = 0; a < gpuInputAssembler->attributes.size(); ++a) {
                  const GLES2GPUAttribute& gpuAttribute = gpuInputAssembler->glAttribs[a];
                  if (gpuAttribute.name == gpuInput.name) {
                    glBindBuffer(GL_ARRAY_BUFFER, gpuAttribute.glBuffer);

                    for (uint c = 0; c < gpuAttribute.componentCount; ++c) {
                      GLint glLoc = gpuInput.glLoc + c;
                      uint attrib_offset = gpuAttribute.offset + gpuAttribute.size * c;
                      glEnableVertexAttribArray(glLoc);

                      cache->glEnabledAttribLocs[glLoc] = true;
                      glVertexAttribPointer(glLoc, gpuAttribute.count, gpuAttribute.glType, gpuAttribute.isNormalized, gpuAttribute.stride, BUFFER_OFFSET(attrib_offset));

                      if (device->useInstancedArrays()) {
                        glVertexAttribDivisorEXT(glLoc, gpuAttribute.isInstanced ? 1 : 0);
                      }
                    }
                    break;
                  }
                } // for
              } // for

              if (gpuInputAssembler->gpuIndexBuffer) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuInputAssembler->gpuIndexBuffer->glBuffer);
              }

              glBindVertexArrayOES(0);
              glBindBuffer(GL_ARRAY_BUFFER, 0);
              glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
              cache->glVAO = 0;
              cache->glArrayBuffer = 0;
              cache->glElementArrayBuffer = 0;
            }

            if (cache->glVAO != glVAO) {
              glBindVertexArrayOES(glVAO);
              cache->glVAO = glVAO;
            }
          } else {
            for (uint a = 0; a < GFX_MAX_VERTEX_ATTRIBUTES; ++a) {
              cache->glCurrentAttribLocs[a] = false;
            }

            for (size_t j = 0; j < gpuPipelineState->gpuShader->glInputs.size(); ++j) {
              const GLES2GPUInput& gpuInput = gpuPipelineState->gpuShader->glInputs[j];
              for (size_t a = 0; a < gpuInputAssembler->attributes.size(); ++a) {
                const GLES2GPUAttribute& gpuAttribute = gpuInputAssembler->glAttribs[a];
                if (gpuAttribute.name == gpuInput.name) {
                  if (cache->glArrayBuffer != gpuAttribute.glBuffer) {
                    glBindBuffer(GL_ARRAY_BUFFER, gpuAttribute.glBuffer);
                    cache->glArrayBuffer = gpuAttribute.glBuffer;
                  }

                  for (uint c = 0; c < gpuAttribute.componentCount; ++c) {
                    GLint glLoc = gpuInput.glLoc + c;
                    uint attrib_offset = gpuAttribute.offset + gpuAttribute.size * c;
                    glEnableVertexAttribArray(glLoc);
                    cache->glEnabledAttribLocs[glLoc] = true;
                    cache->glCurrentAttribLocs[glLoc] = true;
                    glVertexAttribPointer(glLoc, gpuAttribute.count, gpuAttribute.glType, gpuAttribute.isNormalized, gpuAttribute.stride, BUFFER_OFFSET(attrib_offset));

                    if (device->useInstancedArrays()) {
                      glVertexAttribDivisorEXT(glLoc, gpuAttribute.isInstanced ? 1 : 0);
                    }
                  }
                  break;
                }
              } // for
            } // for

            if (gpuInputAssembler->gpuIndexBuffer) {
              if (cache->glElementArrayBuffer != gpuInputAssembler->gpuIndexBuffer->glBuffer) {
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gpuInputAssembler->gpuIndexBuffer->glBuffer);
                cache->glElementArrayBuffer = gpuInputAssembler->gpuIndexBuffer->glBuffer;
              }
            }

            for (uint a = 0; a < GFX_MAX_VERTEX_ATTRIBUTES; ++a) {
              if (cache->glEnabledAttribLocs[a] != cache->glCurrentAttribLocs[a]) {
                glDisableVertexAttribArray(a);
                cache->glEnabledAttribLocs[a] = false;
              }
            }
          } // if
        }
          
        //TODO: implementation setting dynamic states.
        
        break;
      }
      case GFXCmdType::DRAW: {
        GLES2CmdDraw* cmd = cmd_package->drawCmds[cmd_idx];
        if (gpuInputAssembler && gpuPipelineState) {
          if (!gpuInputAssembler->gpuIndirectBuffer) {

            if (gpuInputAssembler->gpuIndexBuffer && cmd->draw_info.indexCount >= 0) {
              uint8_t* offset = 0;
              offset += cmd->draw_info.firstIndex * gpuInputAssembler->gpuIndexBuffer->stride;
              if (cmd->draw_info.instanceCount == 0) {
                glDrawElements(glPrimitive, cmd->draw_info.indexCount, gpuInputAssembler->glIndexType, offset);
              } else {
                if (device->useDrawInstanced()) {
                  glDrawElementsInstancedEXT(glPrimitive, cmd->draw_info.indexCount, gpuInputAssembler->glIndexType, offset, cmd->draw_info.instanceCount);
                }
              }
            } else {
              if (cmd->draw_info.instanceCount == 0) {
                glDrawArrays(glPrimitive, cmd->draw_info.firstIndex, cmd->draw_info.vertexCount);
              } else {
                if (device->useDrawInstanced()) {
                  glDrawArraysInstancedEXT(glPrimitive, cmd->draw_info.firstIndex, cmd->draw_info.vertexCount, cmd->draw_info.instanceCount);
                }
              }
            }
          } else {
            for (size_t j = 0; j < gpuInputAssembler->gpuIndirectBuffer->indirects.size(); ++j) {
              const GFXDrawInfo& draw = gpuInputAssembler->gpuIndirectBuffer->indirects[j];
              if (gpuInputAssembler->gpuIndexBuffer && draw.indexCount >= 0) {
                uint8_t* offset = 0;
                offset += draw.firstIndex * gpuInputAssembler->gpuIndexBuffer->stride;
                if (cmd->draw_info.instanceCount == 0) {
                  glDrawElements(glPrimitive, draw.indexCount, gpuInputAssembler->glIndexType, offset);
                } else {
                  if (device->useDrawInstanced()) {
                    glDrawElementsInstancedEXT(glPrimitive, draw.indexCount, gpuInputAssembler->glIndexType, offset, cmd->draw_info.instanceCount);
                  }
                }
              } else {
                if (cmd->draw_info.instanceCount == 0) {
                  glDrawArrays(glPrimitive, draw.firstIndex, draw.vertexCount);
                } else {
                  if (device->useDrawInstanced()) {
                    glDrawArraysInstancedEXT(glPrimitive, draw.firstIndex, draw.vertexCount, cmd->draw_info.instanceCount);
                  }
                }
              }
            }
          }
        }
        break;
      }
      case GFXCmdType::UPDATE_BUFFER: {
        GLES2CmdUpdateBuffer* cmd = cmd_package->updateBufferCmds[cmd_idx];
        GLES2CmdFuncUpdateBuffer(device, cmd->gpuBuffer, cmd->buffer, cmd->offset, cmd->size);
        break;
      }
      case GFXCmdType::COPY_BUFFER_TO_TEXTURE: {
        GLES2CmdCopyBufferToTexture* cmd = cmd_package->copyBufferToTextureCmds[cmd_idx];
        GLES2CmdFuncCopyBuffersToTexture(device, &cmd->gpuBuffer->buffer, cmd->gpuTexture, cmd->regions);
        break;
      }
      default:
        break;
    }
      cmd_idx++;
  }
}

void GLES2CmdFuncCopyBuffersToTexture(GLES2Device* device, uint8_t* const* buffers, GLES2GPUTexture* gpuTexture, const GFXBufferTextureCopyList& regions) {
  GLuint& glTexture = device->stateCache->glTextures[device->stateCache->texUint];
  if (glTexture != gpuTexture->glTexture) {
    glBindTexture(gpuTexture->glTarget, gpuTexture->glTexture);
    glTexture = gpuTexture->glTexture;
  }

  bool isCompressed = GFX_FORMAT_INFOS[(int)gpuTexture->format].isCompressed;
  uint n = 0;

  switch (gpuTexture->glTarget) {
  case GL_TEXTURE_2D: {
    uint w;
    uint h;
    for (size_t i = 0; i < regions.size(); ++i) {
      const GFXBufferTextureCopy& region = regions[i];
      w = region.texExtent.width;
      h = region.texExtent.height;
      for (uint m = region.texSubres.baseMipLevel; m < region.texSubres.baseMipLevel + region.texSubres.levelCount; ++m) {
        uint8_t* buff = region.buffOffset + region.buffTexHeight * region.buffStride + buffers[n++];
        if (!isCompressed) {
          glTexSubImage2D(GL_TEXTURE_2D, m, region.texOffset.x, region.texOffset.y, w, h, gpuTexture->glFormat, gpuTexture->glType, (GLvoid*)buff);
        } else {
          GLsizei memSize = (GLsizei)GFXFormatSize(gpuTexture->format, w, h, 1);
          glCompressedTexSubImage2D(GL_TEXTURE_2D, m, region.texOffset.x, region.texOffset.y, w, h, gpuTexture->glFormat, memSize, (GLvoid*)buff);
        }

        w = std::max(w >> 1, 1U);
        h = std::max(h >> 1, 1U);
      }
    }
    break;
  }
  case GL_TEXTURE_2D_ARRAY: {
    uint w;
    uint h;
    for (size_t i = 0; i < regions.size(); ++i) {
      const GFXBufferTextureCopy& region = regions[i];
      uint d = region.texSubres.layerCount;
      uint layerCount = d + region.texSubres.baseArrayLayer;

      for (uint z = region.texSubres.baseArrayLayer; z < layerCount; ++z) {
        w = region.texExtent.width;
        h = region.texExtent.height;
        for (uint m = region.texSubres.baseMipLevel; m < region.texSubres.baseMipLevel + region.texSubres.levelCount; ++m) {
          uint8_t* buff = region.buffOffset + region.buffTexHeight * region.buffStride + buffers[n++];
          if (!isCompressed) {
            glTexSubImage3DOES(GL_TEXTURE_2D_ARRAY, m, region.texOffset.x, region.texOffset.y, z, 
              w, h, d, gpuTexture->glFormat, gpuTexture->glType, (GLvoid*)buff);
          }
          else {
            GLsizei memSize = (GLsizei)GFXFormatSize(gpuTexture->format, w, h, 1);
            glCompressedTexSubImage3DOES(GL_TEXTURE_2D_ARRAY, m, region.texOffset.x, region.texOffset.y, z,
              w, h, d, gpuTexture->glFormat, memSize, (GLvoid*)buff);
          }

          w = std::max(w >> 1, 1U);
          h = std::max(h >> 1, 1U);
        }
      }
    }
    break;
  }
  case GL_TEXTURE_3D: {
    uint w;
    uint h;
    uint d;
    for (size_t i = 0; i < regions.size(); ++i) {
      const GFXBufferTextureCopy& region = regions[i];
      w = region.texExtent.width;
      h = region.texExtent.height;
      d = region.texExtent.depth;
      for (uint m = region.texSubres.baseMipLevel; m < region.texSubres.baseMipLevel + region.texSubres.levelCount; ++m) {
        uint8_t* buff = region.buffOffset + region.buffTexHeight * region.buffStride + buffers[n++];
        if (!isCompressed) {
          glTexSubImage3DOES(GL_TEXTURE_3D, m, region.texOffset.x, region.texOffset.y, region.texOffset.z, 
            w, h, d, gpuTexture->glFormat, gpuTexture->glType, (GLvoid*)buff);
        }
        else {
          GLsizei memSize = (GLsizei)GFXFormatSize(gpuTexture->format, w, d + 1, 1);
          glCompressedTexSubImage3DOES(GL_TEXTURE_3D, m, region.texOffset.x, region.texOffset.y, region.texOffset.z, 
            w, h, d, gpuTexture->glFormat, memSize, (GLvoid*)buff);
        }

        w = std::max(w >> 1, 1U);
        h = std::max(h >> 1, 1U);
        d = std::max(d >> 1, 1U);
      }
    }
    break;
  }
  case GL_TEXTURE_CUBE_MAP: {
    uint w;
    uint h;
    uint f;
    for (size_t i = 0; i < regions.size(); ++i) {
      const GFXBufferTextureCopy& region = regions[i];
      uint face_count = region.texSubres.baseArrayLayer + region.texSubres.layerCount;
      for (f = region.texSubres.baseArrayLayer; f < face_count; ++f) {
        w = region.texExtent.width;
        h = region.texExtent.height;

        for (uint m = region.texSubres.baseMipLevel; m < region.texSubres.baseMipLevel + region.texSubres.levelCount; ++m) {
          uint8_t* buff = region.buffOffset + region.buffTexHeight * region.buffStride + buffers[n++];
          if (!isCompressed) {
            glTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, m, region.texOffset.x, region.texOffset.y, w, h, gpuTexture->glFormat, gpuTexture->glType, (GLvoid*)buff);
          }
          else {
            GLsizei memSize = (GLsizei)GFXFormatSize(gpuTexture->format, w, h, 1);
            glCompressedTexSubImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + f, m, region.texOffset.x, region.texOffset.y, w, h, gpuTexture->glFormat, memSize, (GLvoid*)buff);
          }

          w = std::max(w >> 1, 1U);
          h = std::max(h >> 1, 1U);
        }
      }
    }
    break;
  }
  default:
    CCASSERT(false, "Unsupported GFXTextureType, copy buffers to texture failed.");
    break;
  }
    
    if(gpuTexture->flags & GFXTextureFlagBit::GEN_MIPMAP)
    {
        glBindTexture(gpuTexture->glTarget, gpuTexture->glTexture);
        glGenerateMipmap(gpuTexture->glTarget);
    }
}


NS_CC_END

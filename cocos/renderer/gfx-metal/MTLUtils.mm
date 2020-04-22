#include "MTLStd.h"
#include "MTLUtils.h"
#include "shaderc/shaderc.hpp"
#include "shaderc/spvc.hpp"
#include <vector>

NS_CC_BEGIN

namespace mu
{
    MTLLoadAction toMTLLoadAction(GFXLoadOp op)
    {
        switch (op) {
            case GFXLoadOp::CLEAR: return MTLLoadActionClear;
            case GFXLoadOp::LOAD: return MTLLoadActionLoad;
            case GFXLoadOp::DISCARD: return MTLLoadActionDontCare;
            default: return MTLLoadActionDontCare;
        }
    }
    
    MTLStoreAction toMTLStoreAction(GFXStoreOp op)
    {
        switch (op) {
            case GFXStoreOp::STORE: return MTLStoreActionStore;
            case GFXStoreOp::DISCARD: return MTLStoreActionDontCare;
            default: return MTLStoreActionDontCare;
        }
    }
    
    MTLClearColor toMTLClearColor(const GFXColor& clearColor)
    {
        MTLClearColor mtlColor;
        mtlColor = MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
        return MTLClearColorMake(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
    }
    
    MTLVertexFormat toMTLVertexFormat(GFXFormat format, bool isNormalized)
    {
        switch (format) {
            case GFXFormat::R32F: return MTLVertexFormatFloat;
            case GFXFormat::R32I: return MTLVertexFormatInt;
            case GFXFormat::R32UI: return MTLVertexFormatUInt;
            case GFXFormat::RG8: return MTLVertexFormatUChar2;
            case GFXFormat::RG8I: return MTLVertexFormatChar2;
            case GFXFormat::RG16F: return MTLVertexFormatHalf2;
            case GFXFormat::RG16I: return MTLVertexFormatUShort2;
            case GFXFormat::RG32I: return MTLVertexFormatInt2;
            case GFXFormat::RG32UI: return MTLVertexFormatUInt2;
            case GFXFormat::RG32F: return MTLVertexFormatFloat2;
            case GFXFormat::RGB8: return MTLVertexFormatUChar3;
            case GFXFormat::RGB8I: return MTLVertexFormatChar3;
            case GFXFormat::RGB16I: return MTLVertexFormatShort3;
            case GFXFormat::RGB16UI: return MTLVertexFormatUShort3;
            case GFXFormat::RGB16F: return MTLVertexFormatHalf3;
            case GFXFormat::RGB32I: return MTLVertexFormatInt3;
            case GFXFormat::RGB32UI: return MTLVertexFormatUInt3;
            case GFXFormat::RGB32F: return MTLVertexFormatFloat3;
            case GFXFormat::RGBA8: return isNormalized ? MTLVertexFormatUChar4Normalized : MTLVertexFormatUChar4;
            case GFXFormat::RGBA8I: return MTLVertexFormatChar4;
            case GFXFormat::RGBA16I: return MTLVertexFormatShort4;
            case GFXFormat::RGBA16UI: return MTLVertexFormatUShort4;
            case GFXFormat::RGBA16F: return MTLVertexFormatHalf4;
            case GFXFormat::RGBA32I: return MTLVertexFormatInt4;
            case GFXFormat::RGBA32UI: return MTLVertexFormatUInt4;
            case GFXFormat::RGBA32F: return MTLVertexFormatFloat4;
            default:
            {
                CC_LOG_ERROR("Invalid vertex format %u", format);
                return MTLVertexFormatInvalid;
            }
        }
    }
    
    GFXFormat convertGFXPixelFormat(GFXFormat format)
    {
        switch (format) {
            case GFXFormat::RGB8:   return GFXFormat::RGBA8;
            case GFXFormat::RGB32F: return GFXFormat::RGBA32F;
            default: return format;
        }
    }
    
    MTLPixelFormat toMTLPixelFormat(GFXFormat format)
    {
        switch (format) {
            case GFXFormat::A8: return MTLPixelFormatA8Unorm;
            case GFXFormat::R8: return MTLPixelFormatR8Uint;
            case GFXFormat::R8SN: return MTLPixelFormatR8Snorm;
            case GFXFormat::R8UI: return MTLPixelFormatR8Uint;
            case GFXFormat::R16F: return MTLPixelFormatR16Float;
            case GFXFormat::R32F: return MTLPixelFormatR32Float;
            case GFXFormat::R32UI: return MTLPixelFormatR32Uint;
            case GFXFormat::R32I: return MTLPixelFormatR32Sint;
                
            case GFXFormat::RG8: return MTLPixelFormatRG8Unorm;
            case GFXFormat::RG8SN: return MTLPixelFormatRG8Snorm;
            case GFXFormat::RG8UI: return MTLPixelFormatRG8Uint;
            case GFXFormat::RG8I: return MTLPixelFormatRG8Sint;
            case GFXFormat::RG16F: return MTLPixelFormatRG16Float;
            case GFXFormat::RG16UI: return MTLPixelFormatRG16Uint;
            case GFXFormat::RG16I: return MTLPixelFormatRG16Sint;
                
//            case GFXFormat::RGB8SN: return MTLPixelFormatRGBA8Snorm;
//            case GFXFormat::RGB8UI: return MTLPixelFormatRGBA8Uint;
//            case GFXFormat::RGB8I: return MTLPixelFormatRGBA8Sint;
//            case GFXFormat::RGB16F: return MTLPixelFormatRGBA16Float;
//            case GFXFormat::RGB16UI: return MTLPixelFormatRGBA16Uint;
//            case GFXFormat::RGB16I: return MTLPixelFormatRGBA16Sint;
//            case GFXFormat::RGB32F: return MTLPixelFormatRGBA32Float;
//            case GFXFormat::RGB32UI: return MTLPixelFormatRGBA32Uint;
//            case GFXFormat::RGB32I: return MTLPixelFormatRGBA32Sint;
                
            case GFXFormat::RGBA8: return MTLPixelFormatRGBA8Unorm;
            case GFXFormat::RGBA8SN: return MTLPixelFormatRGBA8Snorm;
            case GFXFormat::RGBA8UI: return MTLPixelFormatRGBA8Uint;
            case GFXFormat::RGBA8I: return MTLPixelFormatRGBA8Sint;
            case GFXFormat::RGBA16F: return MTLPixelFormatRGBA16Float;
            case GFXFormat::RGBA16UI: return MTLPixelFormatRGBA16Uint;
            case GFXFormat::RGBA16I: return MTLPixelFormatRGBA16Sint;
            case GFXFormat::RGBA32F: return MTLPixelFormatRGBA32Float;
            case GFXFormat::RGBA32UI: return MTLPixelFormatRGBA32Uint;
            case GFXFormat::RGBA32I: return MTLPixelFormatRGBA32Sint;
            case GFXFormat::BGRA8: return MTLPixelFormatBGRA8Unorm;
                
            // Should convert.
//            case GFXFormat::R5G6B5: return MTLPixelFormatB5G6R5Unorm;
//            case GFXFormat::RGB5A1: return MTLPixelFormatBGR5A1Unorm;
//            case GFXFormat::RGBA4: return MTLPixelFormatABGR4Unorm;
//            case GFXFormat::RGB10A2: return MTLPixelFormatBGR10A2Unorm;
            case GFXFormat::RGB9E5: return MTLPixelFormatRGB9E5Float;
            case GFXFormat::RGB10A2UI: return MTLPixelFormatRGB10A2Uint;
            case GFXFormat::R11G11B10F: return MTLPixelFormatRG11B10Float;
            
            case GFXFormat::D16: return MTLPixelFormatDepth16Unorm;
            case GFXFormat::D24S8: return MTLPixelFormatDepth24Unorm_Stencil8;
            case GFXFormat::D32F: return MTLPixelFormatDepth32Float;
            case GFXFormat::D32F_S8: return MTLPixelFormatDepth32Float_Stencil8;
                
            case GFXFormat::BC1_ALPHA: return MTLPixelFormatBC1_RGBA;
            case GFXFormat::BC1_SRGB_ALPHA: return MTLPixelFormatBC1_RGBA_sRGB;
                
            default:
            {
                CC_LOG_ERROR("Invalid pixel format %u", format);
                return MTLPixelFormatInvalid;
            }
        }
    }
    
    MTLColorWriteMask toMTLColorWriteMask(GFXColorMask mask)
    {
        switch (mask) {
            case GFXColorMask::R: return MTLColorWriteMaskRed;
            case GFXColorMask::G: return MTLColorWriteMaskGreen;
            case GFXColorMask::B: return MTLColorWriteMaskBlue;
            case GFXColorMask::A: return MTLColorWriteMaskAlpha;
            case GFXColorMask::ALL: return MTLColorWriteMaskAll;
            default: return MTLColorWriteMaskNone;
        }
    }
    
    MTLBlendFactor toMTLBlendFactor(GFXBlendFactor factor)
    {
        switch (factor) {
            case GFXBlendFactor::ZERO: return MTLBlendFactorZero;
            case GFXBlendFactor::ONE: return MTLBlendFactorOne;
            case GFXBlendFactor::SRC_ALPHA: return MTLBlendFactorSourceAlpha;
            case GFXBlendFactor::DST_ALPHA: return MTLBlendFactorDestinationAlpha;
            case GFXBlendFactor::ONE_MINUS_SRC_ALPHA: return MTLBlendFactorOneMinusSourceAlpha;
            case GFXBlendFactor::ONE_MINUS_DST_ALPHA: return MTLBlendFactorOneMinusDestinationAlpha;
            case GFXBlendFactor::SRC_COLOR: return MTLBlendFactorSourceColor;
            case GFXBlendFactor::DST_COLOR: return MTLBlendFactorDestinationColor;
            case GFXBlendFactor::ONE_MINUS_SRC_COLOR: return MTLBlendFactorOneMinusSourceColor;
            case GFXBlendFactor::ONE_MINUS_DST_COLOR: return MTLBlendFactorOneMinusDestinationColor;
            case GFXBlendFactor::SRC_ALPHA_SATURATE: return MTLBlendFactorSourceAlphaSaturated;
            default:
            {
                CC_LOG_ERROR("Unsupported blend factor %u", (uint)factor);
                return MTLBlendFactorZero;
            }
        }
    }
    
    MTLBlendOperation toMTLBlendOperation(GFXBlendOp op)
    {
        switch (op) {
            case GFXBlendOp::ADD: return MTLBlendOperationAdd;
            case GFXBlendOp::SUB: return MTLBlendOperationSubtract;
            case GFXBlendOp::REV_SUB: return MTLBlendOperationReverseSubtract;
            case GFXBlendOp::MIN: return MTLBlendOperationMin;
            case GFXBlendOp::MAX: return MTLBlendOperationMax;
        }
    }
    
    MTLCullMode toMTLCullMode(GFXCullMode mode)
    {
        switch (mode) {
            case GFXCullMode::FRONT: return MTLCullModeFront;
            case GFXCullMode::BACK: return MTLCullModeBack;
            case GFXCullMode::NONE: return MTLCullModeNone;
        }
    }
    
    MTLWinding toMTLWinding(bool isFrontFaceCCW)
    {
        if (isFrontFaceCCW)
            return MTLWindingCounterClockwise;
        else
            return MTLWindingClockwise;
    }
    
    MTLViewport toMTLViewport(const GFXViewport& viewport)
    {
        MTLViewport mtlViewport;
        mtlViewport.originX = viewport.left;
        mtlViewport.originY = viewport.top;
        mtlViewport.width = viewport.width;
        mtlViewport.height = viewport.height;
        mtlViewport.znear = viewport.minDepth;
        mtlViewport.zfar = viewport.maxDepth;
        
        return mtlViewport;
    }
    
    MTLScissorRect toMTLScissorRect(const GFXRect& rect)
    {
        MTLScissorRect scissorRect;
        scissorRect.x = rect.x;
        scissorRect.y = rect.y;
        scissorRect.width = rect.width;
        scissorRect.height = rect.height;
        
        return scissorRect;
    }
    
    MTLTriangleFillMode toMTLTriangleFillMode(GFXPolygonMode mode)
    {
        switch (mode) {
            case GFXPolygonMode::FILL: return MTLTriangleFillModeFill;
            case GFXPolygonMode::LINE: return MTLTriangleFillModeLines;
            case GFXPolygonMode::POINT:
            {
                CC_LOG_WARNING("Metal doesn't support GFXPolygonMode::POINT, translate to GFXPolygonMode::LINE.");
                return MTLTriangleFillModeLines;
            }
        }
    }
    
    MTLDepthClipMode toMTLDepthClipMode(bool isClip)
    {
        if (isClip)
            return MTLDepthClipModeClip;
        else
            return MTLDepthClipModeClamp;
    }
    
    MTLCompareFunction toMTLCompareFunction(GFXComparisonFunc func)
    {
        switch (func) {
            case GFXComparisonFunc::NEVER: return MTLCompareFunctionNever;
            case GFXComparisonFunc::LESS: return MTLCompareFunctionLess;
            case GFXComparisonFunc::EQUAL: return MTLCompareFunctionEqual;
            case GFXComparisonFunc::LESS_EQUAL: return MTLCompareFunctionLessEqual;
            case GFXComparisonFunc::GREATER: return MTLCompareFunctionGreater;
            case GFXComparisonFunc::NOT_EQUAL: return MTLCompareFunctionNotEqual;
            case GFXComparisonFunc::GREATER_EQUAL: return MTLCompareFunctionGreaterEqual;
            case GFXComparisonFunc::ALWAYS: return MTLCompareFunctionAlways;
        }
    }
    
    MTLStencilOperation toMTLStencilOperation(GFXStencilOp op)
    {
        switch (op) {
            case GFXStencilOp::ZERO: return MTLStencilOperationZero;
            case GFXStencilOp::KEEP: return MTLStencilOperationKeep;
            case GFXStencilOp::REPLACE: return MTLStencilOperationReplace;
            case GFXStencilOp::INCR: return MTLStencilOperationIncrementClamp;
            case GFXStencilOp::DECR: return MTLStencilOperationDecrementClamp;
            case GFXStencilOp::INVERT: return MTLStencilOperationInvert;
            case GFXStencilOp::INCR_WRAP: return MTLStencilOperationIncrementWrap;
            case GFXStencilOp::DECR_WRAP: return MTLStencilOperationDecrementWrap;
        }
    }
    
    MTLPrimitiveType toMTLPrimitiveType(GFXPrimitiveMode mode)
    {
        switch (mode) {
            case GFXPrimitiveMode::POINT_LIST: return MTLPrimitiveTypePoint;
            case GFXPrimitiveMode::LINE_LIST: return MTLPrimitiveTypeLine;
            case GFXPrimitiveMode::LINE_STRIP: return MTLPrimitiveTypeLineStrip;
            case GFXPrimitiveMode::TRIANGLE_LIST: return MTLPrimitiveTypeTriangle;
            case GFXPrimitiveMode::TRIANGLE_STRIP: return MTLPrimitiveTypeTriangleStrip;
                
            case GFXPrimitiveMode::LINE_LOOP:
            {
                CC_LOG_ERROR("Metal doesn't support GFXPrimitiveMode::LINE_LOOP. Translate to GFXPrimitiveMode::LINE_LIST.");
                return MTLPrimitiveTypeLine;
            }
            default:
            {
                //TODO: how to support these mode?
                CC_ASSERT(false);
                return MTLPrimitiveTypeTriangle;
            }
        }
    }
    
    MTLTextureUsage toMTLTextureUsage(GFXTextureUsage usage)
    {
        if (usage == GFXTextureUsage::NONE)
            return MTLTextureUsageUnknown;
        
        MTLTextureUsage ret = MTLTextureUsageUnknown;
        if (usage & GFXTextureUsage::TRANSFER_SRC)
            ret |= MTLTextureUsageShaderRead;
        if (usage & GFXTextureUsage::TRANSFER_DST)
            ret |= MTLTextureUsageShaderWrite;
        if (usage & GFXTextureUsage::SAMPLED)
            ret |= MTLTextureUsageShaderRead;
        if (usage & GFXTextureUsage::STORAGE)
            ret |= MTLTextureUsageShaderWrite;
        if (usage & GFXTextureUsage::COLOR_ATTACHMENT ||
            usage & GFXTextureUsage::DEPTH_STENCIL_ATTACHMENT ||
            usage & GFXTextureUsage::TRANSIENT_ATTACHMENT ||
            usage & GFXTextureUsage::INPUT_ATTACHMENT)
        {
            ret |= MTLTextureUsageRenderTarget;
        }
        
        return ret;
    }
    
    MTLTextureType toMTLTextureType(GFXTextureType type, uint arrayLength, GFXTextureFlags flags)
    {
        switch(type) {
            case GFXTextureType::TEX1D:
                if (arrayLength <= 1)
                    return MTLTextureType1D;
                else
                    return MTLTextureType1DArray;
            case GFXTextureType::TEX2D:
                if (arrayLength <= 1)
                    return MTLTextureType2D;
                else if (flags & GFXTextureFlagBit::CUBEMAP)
                    return MTLTextureTypeCube;
                else
                    return MTLTextureType2DArray;
            case GFXTextureType::TEX3D:
                return MTLTextureType3D;
            default:
                return MTLTextureType2D;
        }
    }
    
    MTLTextureType toMTLTextureType(GFXTextureViewType type)
    {
        switch (type) {
            case GFXTextureViewType::TV1D:          return MTLTextureType1D;
            case GFXTextureViewType::TV1D_ARRAY:    return MTLTextureType1DArray;
            case GFXTextureViewType::TV2D:          return MTLTextureType2D;
            case GFXTextureViewType::TV2D_ARRAY:    return MTLTextureType2DArray;
            case GFXTextureViewType::CUBE:          return MTLTextureTypeCube;
            case GFXTextureViewType::TV3D:          return MTLTextureType3D;
        }
    }
    
    NSUInteger toMTLSampleCount(GFXSampleCount count)
    {
        switch (count) {
            case GFXSampleCount::X1: return 1;
            case GFXSampleCount::X2: return 2;
            case GFXSampleCount::X4: return 4;
            case GFXSampleCount::X8: return 8;
            case GFXSampleCount::X16: return 16;
            case GFXSampleCount::X32: return 32;
            case GFXSampleCount::X64: return 64;
        }
    }
    
    MTLSamplerAddressMode toMTLSamplerAddressMode(GFXAddress mode)
    {
        switch (mode) {
            case GFXAddress::WRAP: return MTLSamplerAddressModeRepeat;
            case GFXAddress::MIRROR: return MTLSamplerAddressModeMirrorRepeat;
            case GFXAddress::CLAMP: return MTLSamplerAddressModeClampToEdge;
            case GFXAddress::BORDER: return MTLSamplerAddressModeClampToBorderColor;
        }
    }
    
    MTLSamplerBorderColor toMTLSamplerBorderColor(const GFXColor& color)
    {
        float diff = color.r - 0.5f;
        if (math::IsEqualF(color.a, 0.f) )
            return MTLSamplerBorderColorTransparentBlack;
        else if (math::IsEqualF(diff, 0.f) )
            return MTLSamplerBorderColorOpaqueBlack;
        else
            return MTLSamplerBorderColorOpaqueWhite;
    }
    
    MTLSamplerMinMagFilter toMTLSamplerMinMagFilter(GFXFilter filter)
    {
        switch (filter) {
            case GFXFilter::LINEAR:
            case GFXFilter::ANISOTROPIC:
                return MTLSamplerMinMagFilterLinear;
            default:
                return MTLSamplerMinMagFilterNearest;
        }
    }
    
    MTLSamplerMipFilter toMTLSamplerMipFilter(GFXFilter filter)
    {
        switch (filter) {
            case GFXFilter::NONE:
                return MTLSamplerMipFilterNotMipmapped;
            case GFXFilter::LINEAR:
            case GFXFilter::ANISOTROPIC:
                return MTLSamplerMipFilterLinear;
            case GFXFilter::POINT:
                return MTLSamplerMipFilterNearest;
        }
    }
    
    std::string compileGLSLShader2Mtl(const std::string& src, bool isVertexShader)
    {
#if USE_METAL
        std::string shader("#version 310 es\n");
        shader.append(src);
        shaderc::Compiler glslCompiler;
        shaderc_shader_kind shaderKind = isVertexShader ? shaderc_shader_kind::shaderc_vertex_shader
        : shaderc_shader_kind::shaderc_fragment_shader;
        shaderc::SpvCompilationResult module = glslCompiler.CompileGlslToSpv(shader, shaderKind, "");
        if (module.GetCompilationStatus() != shaderc_compilation_status_success)
        {
            CC_LOG_ERROR("Failed to compile GLSL shader to spv. %s", module.GetErrorMessage().c_str() );
            CC_LOG_ERROR("%s", src.c_str());
            return "";
        }
        
        std::vector<uint32_t> spivSource = {module.cbegin(), module.cend()};
        shaderc_spvc::Context context;
        shaderc_spvc::CompileOptions options;
        options.SetMSLPlatform(shaderc_spvc_msl_platform_macos);
        auto status = context.InitializeForMsl(spivSource.data(), spivSource.size(), options);
        if (status != shaderc_spvc_status_success)
        {
            CC_LOG_ERROR("Failed to initialize shaderc_spvc::Context.");
            return "";
        }
        
        shaderc_spvc::CompilationResult result;
        status = context.CompileShader(&result);
        if (status != shaderc_spvc_status_success)
        {
            CC_LOG_ERROR("Failed to compile spv to mtl");
            return "";
        }
        
        std::string output;
        result.GetStringOutput(&output);
        return output;
#else
        return src;
#endif
    }

    uint8_t* convertRGB8ToRGBA8(uint8_t* source, uint length)
    {
        uint finalLength = length * 4;
        uint8* out = (uint8*)CC_MALLOC(finalLength);
        if (!out)
        {
            CC_LOG_WARNING("Failed to alloc memory in convertRGB8ToRGBA8().");
            return source;
        }
        
        uint8_t* src = source;
        uint8_t* dst = out;
        for (uint i = 0; i < length; ++i)
        {
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = 255;
        }
        
        return out;
    }

    uint8_t* convertRGB32FToRGBA32F(uint8_t* source, uint length)
    {
        uint finalLength = length * sizeof(float) * 4;
        uint8* out = (uint8*)CC_MALLOC(finalLength);
        if (!out)
        {
            CC_LOG_WARNING("Failed to alloc memory in convertRGB32FToRGBA32F().");
            return source;
        }
        
        float* src = reinterpret_cast<float*>(source);
        float* dst = reinterpret_cast<float*>(out);
        for (uint i = 0; i < length; ++i)
        {
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = *src++;
            *dst++ = 1.0f;
        }
        
        return out;
    }
} //namespace mu

NS_CC_END

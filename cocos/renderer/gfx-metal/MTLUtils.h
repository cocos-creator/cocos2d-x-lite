#pragma once

#import <Metal/MTLRenderPass.h>
#import <Metal/MTLVertexDescriptor.h>
#import <Metal/MTLRenderPipeline.h>
#import <Metal/MTLDepthStencil.h>
#import <Metal/MTLTexture.h>
#import <Metal/MTLSampler.h>

NS_CC_BEGIN

namespace mu
{
    MTLLoadAction toMTLLoadAction(GFXLoadOp op);
    MTLStoreAction toMTLStoreAction(GFXStoreOp op);
    MTLClearColor toMTLClearColor(const GFXColor& clearColor);
    MTLVertexFormat toMTLVertexFormat(GFXFormat);
    MTLPixelFormat toMTLPixelFormat(GFXFormat);
    MTLColorWriteMask toMTLColorWriteMask(GFXColorMask);
    MTLBlendFactor toMTLBlendFactor(GFXBlendFactor);
    MTLBlendOperation toMTLBlendOperation(GFXBlendOp);
    MTLCullMode toMTLCullMode(GFXCullMode);
    MTLWinding toMTLWinding(bool isFrontFaceCCW);
    MTLViewport toMTLViewport(const GFXViewport&);
    MTLScissorRect toMTLScissorRect(const GFXRect&);
    MTLTriangleFillMode toMTLTriangleFillMode(GFXPolygonMode);
    MTLDepthClipMode toMTLDepthClipMode(bool isClip);
    MTLCompareFunction toMTLCompareFunction(GFXComparisonFunc);
    MTLStencilOperation toMTLStencilOperation(GFXStencilOp);
    MTLPrimitiveType toMTLPrimitiveType(GFXPrimitiveMode);
    MTLTextureUsage toMTLTextureUsage(GFXTextureUsage);
    MTLTextureType toMTLTextureType(GFXTextureType type, uint arrayLength, bool isCube);
    MTLTextureType toMTLTextureType(GFXTextureViewType type);
    NSUInteger toMTLSampleCount(GFXSampleCount);
    MTLSamplerAddressMode toMTLSamplerAddressMode(GFXAddress);
    MTLSamplerBorderColor toMTLSamplerBorderColor(const GFXColor&);
    MTLSamplerMinMagFilter toMTLSamplerMinMagFilter(GFXFilter);
    MTLSamplerMipFilter toMTLSamplerMipFilter(GFXFilter);
}

NS_CC_END

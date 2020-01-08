#include "MTLStd.h"
#include "MTLTextureView.h"
#include "MTLTexture.h"
#include "MTLUtils.h"

NS_CC_BEGIN

CCMTLTextureView::CCMTLTextureView(GFXDevice* device) : GFXTextureView(device) {}
CCMTLTextureView::~CCMTLTextureView() { Destroy(); }

bool CCMTLTextureView::Initialize(const GFXTextureViewInfo& info)
{
    texture_ = info.texture;
    type_ = info.type;
    format_ = info.format;
    base_level_ = info.base_level;
    level_count_ = info.level_count;
    base_layer_ = info.base_layer;
    layer_count_ = info.layer_count;
    
    NSRange levels = NSMakeRange(base_level_, level_count_);
    NSRange slics = NSMakeRange(base_layer_, layer_count_);
    id<MTLTexture> mtlTexture = static_cast<CCMTLTexture*>(texture_)->getMTLTexture();
    _mtlTexture = [mtlTexture newTextureViewWithPixelFormat:mu::toMTLPixelFormat(format_)
                                                textureType:mu::toMTLTextureType(type_)
                                                     levels:levels
                                                     slices:slics];
    
    return _mtlTexture != nil;
}

void CCMTLTextureView::Destroy()
{
    if (_mtlTexture)
    {
        [_mtlTexture release];
        _mtlTexture = nil;
    }
}

NS_CC_END

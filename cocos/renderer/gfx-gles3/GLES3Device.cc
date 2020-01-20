#include "GLES3Std.h"
#include "GLES3Device.h"
#include "GLES3StateCache.h"
#include "GLES3Context.h"
#include "GLES3Window.h"
#include "GLES3Queue.h"
#include "GLES3CommandAllocator.h"
#include "GLES3CommandBuffer.h"
#include "GLES3Buffer.h"
#include "GLES3Texture.h"
#include "GLES3TextureView.h"
#include "GLES3Sampler.h"
#include "GLES3Shader.h"
#include "GLES3InputAssembler.h"
#include "GLES3RenderPass.h"
#include "GLES3Framebuffer.h"
#include "GLES3BindingLayout.h"
#include "GLES3PipelineLayout.h"
#include "GLES3PipelineState.h"

NS_CC_BEGIN

GLES3Device::GLES3Device()
{
}

GLES3Device::~GLES3Device()
{
}

bool GLES3Device::Initialize(const GFXDeviceInfo& info)
{
    _api = GFXAPI::GLES3;
    _width = info.width;
    _height = info.height;
    _nativeWidth = info.native_width;
    _nativeHeight = info.native_height;
    _windowHandle = info.window_handle;

    state_cache = CC_NEW(GLES3StateCache);

    GFXContextInfo ctx_info;
    ctx_info.window_handle = _windowHandle;
    ctx_info.shared_ctx = info.shared_ctx;

    _context = CC_NEW(GLES3Context(this));
    if (!_context->Initialize(ctx_info))
    {
        Destroy();
        return false;
    }

    String extStr = (const char*)glGetString(GL_EXTENSIONS);
    extensions_ = StringUtil::Split(extStr, " ");

    _features[(int)GFXFeature::TEXTURE_FLOAT] = true;
    _features[(int)GFXFeature::TEXTURE_HALF_FLOAT] = true;
    _features[(int)GFXFeature::FORMAT_R11G11B10F] = true;
    _features[(int)GFXFeature::FORMAT_D24S8] = true;
    _features[(int)GFXFeature::MSAA] = true;

    if (CheckExtension("color_buffer_float"))
        _features[(int)GFXFeature::COLOR_FLOAT] = true;
    
    if (CheckExtension("color_buffer_half_float"))
        _features[(int)GFXFeature::COLOR_HALF_FLOAT] = true;
    
    if (CheckExtension("texture_float_linear"))
        _features[(int)GFXFeature::TEXTURE_FLOAT_LINEAR] = true;
    
    if (CheckExtension("texture_half_float_linear"))
        _features[(int)GFXFeature::TEXTURE_HALF_FLOAT_LINEAR] = true;

    String compressed_fmts;

    if (CheckExtension("compressed_ETC1"))
    {
        _features[(int)GFXFeature::FORMAT_ETC1] = true;
        compressed_fmts += "etc1 ";
    }

    _features[(int)GFXFeature::FORMAT_ETC2] = true;
    compressed_fmts += "etc2 ";

    if (CheckExtension("texture_compression_pvrtc"))
    {
        _features[(int)GFXFeature::FORMAT_PVRTC] = true;
        compressed_fmts += "pvrtc ";
    }

    if (CheckExtension("texture_compression_astc"))
    {
        _features[(int)GFXFeature::FORMAT_ASTC] = true;
        compressed_fmts += "astc ";
    }

    _renderer = (const char*)glGetString(GL_RENDERER);
    _vendor = (const char*)glGetString(GL_VENDOR);
    _version = (const char*)glGetString(GL_VERSION);

    CC_LOG_INFO("RENDERER: %s", _renderer.c_str());
    CC_LOG_INFO("VENDOR: %s", _vendor.c_str());
    CC_LOG_INFO("VERSION: %s", _version.c_str());
    CC_LOG_INFO("SCREEN_SIZE: %d x %d", _width, _height);
    CC_LOG_INFO("NATIVE_SIZE: %d x %d", _nativeWidth, _nativeHeight);
    CC_LOG_INFO("USE_VAO: %s", use_vao_ ? "true" : "false");
    CC_LOG_INFO("COMPRESSED_FORMATS: %s", compressed_fmts.c_str());

    GFXWindowInfo window_info;
    window_info.color_fmt = _context->color_fmt();
    window_info.depth_stencil_fmt = _context->depth_stencil_fmt();
    window_info.is_offscreen = false;
    _window = CreateGFXWindow(window_info);

    GFXQueueInfo queue_info;
    queue_info.type = GFXQueueType::GRAPHICS;
    _queue = CreateGFXQueue(queue_info);

    GFXCommandAllocatorInfo cmd_alloc_info;
    _cmdAllocator = CreateGFXCommandAllocator(cmd_alloc_info);

    return true;
}

void GLES3Device::Destroy()
{
    CC_SAFE_DESTROY(_cmdAllocator);
    CC_SAFE_DESTROY(_queue);
    CC_SAFE_DESTROY(_window);
    CC_SAFE_DESTROY(_context);
    CC_SAFE_DELETE(state_cache);
}

void GLES3Device::Resize(uint width, uint height)
{
  
}

void GLES3Device::Present()
{
    ((GLES3CommandAllocator*)_cmdAllocator)->ReleaseCmds();
    GLES3Queue* queue = (GLES3Queue*)_queue;
    _numDrawCalls += queue->_numDrawCalls;
    _numTriangles += queue->_numTriangles;

    _context->Present();

    // Clear queue stats
    queue->_numDrawCalls = 0;
    queue->_numTriangles = 0;
}

GFXWindow* GLES3Device::CreateGFXWindow(const GFXWindowInfo& info)
{
    GFXWindow* gfx_window = CC_NEW(GLES3Window(this));
    if (gfx_window->Initialize(info))
        return gfx_window;

    CC_SAFE_DESTROY(gfx_window);
    return nullptr;
}

GFXQueue* GLES3Device::CreateGFXQueue(const GFXQueueInfo& info)
{
    GFXQueue* gfx_queue = CC_NEW(GLES3Queue(this));
    if (gfx_queue->Initialize(info))
        return gfx_queue;

    CC_SAFE_DESTROY(gfx_queue);
    return nullptr;
}

GFXCommandAllocator* GLES3Device::CreateGFXCommandAllocator(const GFXCommandAllocatorInfo& info)
{
    GFXCommandAllocator* gfx_cmd_allocator = CC_NEW(GLES3CommandAllocator(this));
    if (gfx_cmd_allocator->Initialize(info))
        return gfx_cmd_allocator;

    CC_SAFE_DESTROY(gfx_cmd_allocator);
    return nullptr;
}

GFXCommandBuffer* GLES3Device::CreateGFXCommandBuffer(const GFXCommandBufferInfo& info)
{
    GFXCommandBuffer* gfx_cmd_buff = CC_NEW(GLES3CommandBuffer(this));
    if (gfx_cmd_buff->Initialize(info))
        return gfx_cmd_buff;

    CC_SAFE_DESTROY(gfx_cmd_buff)
    return nullptr;
}

GFXBuffer* GLES3Device::CreateGFXBuffer(const GFXBufferInfo& info)
{
    GFXBuffer* gfx_buffer = CC_NEW(GLES3Buffer(this));
    if (gfx_buffer->Initialize(info))
        return gfx_buffer;

    CC_SAFE_DESTROY(gfx_buffer);
    return nullptr;
}

GFXTexture* GLES3Device::CreateGFXTexture(const GFXTextureInfo& info)
{
    GFXTexture* gfx_texture = CC_NEW(GLES3Texture(this));
    if (gfx_texture->Initialize(info))
        return gfx_texture;

    CC_SAFE_DESTROY(gfx_texture);
    return nullptr;
}

GFXTextureView* GLES3Device::CreateGFXTextureView(const GFXTextureViewInfo& info)
{
    GFXTextureView* gfx_tex_view = CC_NEW(GLES3TextureView(this));
    if (gfx_tex_view->Initialize(info))
        return gfx_tex_view;

    CC_SAFE_DESTROY(gfx_tex_view);
    return nullptr;
}

GFXSampler* GLES3Device::CreateGFXSampler(const GFXSamplerInfo& info)
{
    GFXSampler* gfx_sampler = CC_NEW(GLES3Sampler(this));
    if (gfx_sampler->Initialize(info))
        return gfx_sampler;

    CC_SAFE_DESTROY(gfx_sampler);
    return nullptr;
}

GFXShader* GLES3Device::CreateGFXShader(const GFXShaderInfo& info)
{
    GFXShader* gfx_shader = CC_NEW(GLES3Shader(this));
    if (gfx_shader->Initialize(info))
        return gfx_shader;

    CC_SAFE_DESTROY(gfx_shader);
    return nullptr;
}

GFXInputAssembler* GLES3Device::CreateGFXInputAssembler(const GFXInputAssemblerInfo& info)
{
    GFXInputAssembler* gfx_input_assembler = CC_NEW(GLES3InputAssembler(this));
    if (gfx_input_assembler->Initialize(info))
        return gfx_input_assembler;

    CC_SAFE_DESTROY(gfx_input_assembler);
    return nullptr;
}

GFXRenderPass* GLES3Device::CreateGFXRenderPass(const GFXRenderPassInfo& info)
{
    GFXRenderPass* gfx_render_pass = CC_NEW(GLES3RenderPass(this));
    if (gfx_render_pass->Initialize(info))
        return gfx_render_pass;

    CC_SAFE_DESTROY(gfx_render_pass);
    return nullptr;
}

GFXFramebuffer* GLES3Device::CreateGFXFramebuffer(const GFXFramebufferInfo& info)
{
    GFXFramebuffer* gfx_framebuffer = CC_NEW(GLES3Framebuffer(this));
    if (gfx_framebuffer->Initialize(info))
        return gfx_framebuffer;

    CC_SAFE_DESTROY(gfx_framebuffer);
    return nullptr;
}

GFXBindingLayout* GLES3Device::CreateGFXBindingLayout(const GFXBindingLayoutInfo& info)
{
    GFXBindingLayout* gfx_binding_layout = CC_NEW(GLES3BindingLayout(this));
    if (gfx_binding_layout->Initialize(info))
        return gfx_binding_layout;

    CC_SAFE_DESTROY(gfx_binding_layout);
    return nullptr;
}


GFXPipelineState* GLES3Device::CreateGFXPipelineState(const GFXPipelineStateInfo& info)
{
    GFXPipelineState* pipelineState = CC_NEW(GLES3PipelineState(this));
    if (pipelineState->Initialize(info))
        return pipelineState;

    CC_SAFE_DESTROY(pipelineState);
    return nullptr;
}

GFXPipelineLayout* GLES3Device::CreateGFXPipelieLayout(const GFXPipelineLayoutInfo& info)
{
    GFXPipelineLayout* layout = CC_NEW(GLES3PipelineLayout(this));
    if (layout->Initialize(info))
        return layout;

    CC_SAFE_DESTROY(layout);
    return nullptr;
}

void GLES3Device::CopyBuffersToTexture(GFXBuffer* src, GFXTexture* dst, const GFXBufferTextureCopyList& regions)
{
    
    GLES3CmdFuncCopyBuffersToTexture(this, &((GLES3Buffer*)src)->gpu_buffer()->buffer, 1, ((GLES3Texture*)dst)->gpu_texture(), regions);
}

NS_CC_END

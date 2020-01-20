#include "GLES2Std.h"
#include "GLES2Context.h"
#include "gles2w.h"
#import <QuartzCore/CAEAGLLayer.h>
#import <UIKit/UIScreen.h>
#import <QuartzCore/QuartzCore.h>

NS_CC_BEGIN

#if (CC_PLATFORM == CC_PLATFORM_MAC_IOS)

bool GLES2Context::Initialize(const GFXContextInfo &info) {
  
    vsync_mode_ = info.vsync_mode;
    window_handle_ = info.window_handle;

    //////////////////////////////////////////////////////////////////////////

    if (!info.shared_ctx) {
        is_primary_ctx_ = true;
        window_handle_ = info.window_handle;
        
        CAEAGLLayer* eaglLayer = (CAEAGLLayer*)( ((UIView*)(window_handle_)).layer);
        eaglLayer.opaque = TRUE;
        
        EAGLContext* eagl_context = [[EAGLContext alloc]initWithAPI:kEAGLRenderingAPIOpenGLES2];
        if (!eagl_context) {
            CC_LOG_ERROR("Create EAGL context failed.");
            return false;
        }

        eagl_context_ = (intptr_t)eagl_context;
        eagl_shared_ctx_ = (intptr_t)eagl_context;

        if (!gles2wInit())
            return false;
    } else {
        GLES2Context* shared_ctx = (GLES2Context*)info.shared_ctx;
        EAGLContext* eagl_shared_context = (EAGLContext*)shared_ctx->eagl_shared_ctx();
        EAGLContext* eagl_context = [[EAGLContext alloc] initWithAPI: [eagl_shared_context API] sharegroup: [eagl_shared_context sharegroup]];
        if (!eagl_context) {
            CC_LOG_ERROR("Create EGL context with share context [0x%p] failed.", eagl_shared_context);

            eagl_context_ = (intptr_t)eagl_context;
            eagl_shared_ctx_ = (intptr_t)eagl_shared_context;

            return false;
        }
    }
    
    color_fmt_ = GFXFormat::RGBA8;
    depth_stencil_fmt_ = GFXFormat::D24S8;

    if (!MakeCurrent())
        return false;

    return createCustomFrameBuffer();
}

bool GLES2Context::createCustomFrameBuffer()
{
    glGenFramebuffers(1, &_defaultFBO);
    if (0 == _defaultFBO)
    {
        CC_LOG_ERROR("Can not create default frame buffer");
        return false;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, _defaultFBO);
    
    glGenRenderbuffers(1, &_defaultColorBuffer);
    if (0 == _defaultColorBuffer)
    {
        CC_LOG_ERROR("Can not create default color buffer");
        return false;
    }
    glBindRenderbuffer(GL_RENDERBUFFER, _defaultColorBuffer);
    
    CAEAGLLayer* eaglLayer = (CAEAGLLayer*)( ((UIView*)(window_handle_)).layer);
    
    //get the storage from iOS so it can be displayed in the view
    if (! [(EAGLContext*)eagl_context_ renderbufferStorage:GL_RENDERBUFFER
                                              fromDrawable:eaglLayer])
    {
        CC_LOG_ERROR("Attaches EAGLDrawable as storage for the OpenGL ES renderbuffer object failed.");
        glBindRenderbuffer(GL_RENDERBUFFER, 0);
        glDeleteRenderbuffers(1, &_defaultColorBuffer);
        return false;
    }
    
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, _defaultColorBuffer);
    
    //get the frame's width and height
    GLint framebufferWidth = 0, framebufferHeight = 0;
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_WIDTH, &framebufferWidth);
    glGetRenderbufferParameteriv(GL_RENDERBUFFER, GL_RENDERBUFFER_HEIGHT, &framebufferHeight);
    glGenRenderbuffers(1, &_defaultDepthStencilBuffer);
    if (_defaultDepthStencilBuffer == 0)
    {
        // Application can run without depth/stencil buffer, so don't return false here.
        CC_LOG_ERROR("Can not create default depth/stencil buffer");
    }
    glBindRenderbuffer(GL_RENDERBUFFER, _defaultDepthStencilBuffer);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8_OES, framebufferWidth, framebufferHeight);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, _defaultDepthStencilBuffer);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_STENCIL_ATTACHMENT, GL_RENDERBUFFER, _defaultDepthStencilBuffer);
    
    GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
    if (status != GL_FRAMEBUFFER_COMPLETE)
    {
        switch (status)
        {
            case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
            {
                CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_INCOMPLETE_ATTACHMENT");
                break;
            }
            case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
            {
                CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT");
                break;
            }
            case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS:
            {
                CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_INCOMPLETE_DIMENSIONS");
                break;
            }
            case GL_FRAMEBUFFER_UNSUPPORTED:
            {
                CC_LOG_ERROR("glCheckFramebufferStatus() - FRAMEBUFFER_UNSUPPORTED");
                break;
            }
            default:;
        }
        destroyCustomFrameBuffer();
        return false;
    }

    glBindRenderbuffer(GL_RENDERBUFFER, _defaultColorBuffer);
    return true;
}

void GLES2Context::destroyCustomFrameBuffer()
{
    if (_defaultColorBuffer)
    {
        glDeleteRenderbuffers(1, &_defaultColorBuffer);
        _defaultColorBuffer = 0;
    }
    if (_defaultDepthStencilBuffer)
    {
        glDeleteRenderbuffers(1, &_defaultDepthStencilBuffer);
        _defaultDepthStencilBuffer = 0;
    }
    
    if (_defaultFBO)
    {
        glDeleteFramebuffers(1, &_defaultFBO);
        _defaultFBO = 0;
    }
}

void GLES2Context::destroy() {
    destroyCustomFrameBuffer();
  
  if (eagl_context_) {
    [(EAGLContext*)eagl_context_ release];
  }

  is_primary_ctx_ = false;
  window_handle_ = 0;
  vsync_mode_ = GFXVsyncMode::OFF;
  is_initialized = false;
}

void GLES2Context::Present() {
  if (! [(EAGLContext*)eagl_context_ presentRenderbuffer:GL_RENDERBUFFER] )
  {
      CC_LOG_ERROR("Failed to present content.");
  }
}

bool GLES2Context::MakeCurrentImpl() {
  return [EAGLContext setCurrentContext:(EAGLContext*)eagl_context_];
}

#endif

NS_CC_END

/****************************************************************************
Copyright (c) 2010-2012 cocos2d-x.org
Copyright (c) 2013-2016 Chukong Technologies Inc.
Copyright (c) 2017-2018 Xiamen Yaji Software Co., Ltd.

http://www.cocos2d-x.org

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
****************************************************************************/
#include "platform/CCApplication.h"
#include <android/log.h>
#include <jni.h>
#include <EGL/egl.h>
#include <cstring>
#include "platform/android/jni/JniImp.h"
#include "platform/Android/CCGL-android.h"
#include "base/CCScheduler.h"

#define  LOG_TAG    "CCApplication_android Debug"
#define  LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG,__VA_ARGS__)

// FIXME: using ndk-r10c will cause the next function could not be found. It may be a bug of ndk-r10c.
// Here is the workaround method to fix the problem.
#ifdef __aarch64__
extern "C" size_t __ctype_get_mb_cur_max(void) 
{
    return (size_t) sizeof(wchar_t);
}
#endif

PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT = 0;
PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT = 0;
PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT = 0;

NS_CC_BEGIN

Application* Application::_instance = nullptr;

Application::Application(const std::string& name, int width, int height)
{
    Application::_instance = this;
    _scheduler = new Scheduler();

    PFNGLGENVERTEXARRAYSOESPROC glGenVertexArraysOESEXT = (PFNGLGENVERTEXARRAYSOESPROC)eglGetProcAddress("glGenVertexArraysOES");
    PFNGLBINDVERTEXARRAYOESPROC glBindVertexArrayOESEXT = (PFNGLBINDVERTEXARRAYOESPROC)eglGetProcAddress("glBindVertexArrayOES");
    PFNGLDELETEVERTEXARRAYSOESPROC glDeleteVertexArraysOESEXT = (PFNGLDELETEVERTEXARRAYSOESPROC)eglGetProcAddress("glDeleteVertexArraysOES");

    // Shoule create _renderTexture here, then we don't have to consider how to
    // revert the GL states. It is difficult to revert the states.
    _renderTexture = new RenderTexture(width, height, 2);
}

Application::~Application()
{
    delete _scheduler;
    _scheduler = nullptr;

    delete _renderTexture;
    _renderTexture = nullptr;

    Application::_instance = nullptr;
}

void Application::start()
{
    if(!applicationDidFinishLaunching())
        return;
}

void Application::setMultitouch(bool /*value*/)
{

}

bool Application::applicationDidFinishLaunching()
{
    return true;
}

void Application::applicationDidEnterBackground()
{

}

void Application::applicationWillEnterForeground()
{

}

void Application::setPreferredFramesPerSecond(int fps) 
{
    _fps = fps;
    setPreferredFramesPerSecondJNI(_fps);
}

std::string Application::getCurrentLanguageCode() const
{
    std::string language = getCurrentLanguageJNI();
    return language.substr(0, 2);
}

Application::LanguageType Application::getCurrentLanguage() const
{
    std::string languageName = getCurrentLanguageJNI();
    const char* pLanguageName = languageName.c_str();
    LanguageType ret = LanguageType::ENGLISH;

    if (0 == strcmp("zh", pLanguageName))
    {
        ret = LanguageType::CHINESE;
    }
    else if (0 == strcmp("en", pLanguageName))
    {
        ret = LanguageType::ENGLISH;
    }
    else if (0 == strcmp("fr", pLanguageName))
    {
        ret = LanguageType::FRENCH;
    }
    else if (0 == strcmp("it", pLanguageName))
    {
        ret = LanguageType::ITALIAN;
    }
    else if (0 == strcmp("de", pLanguageName))
    {
        ret = LanguageType::GERMAN;
    }
    else if (0 == strcmp("es", pLanguageName))
    {
        ret = LanguageType::SPANISH;
    }
    else if (0 == strcmp("ru", pLanguageName))
    {
        ret = LanguageType::RUSSIAN;
    }
    else if (0 == strcmp("nl", pLanguageName))
    {
        ret = LanguageType::DUTCH;
    }
    else if (0 == strcmp("ko", pLanguageName))
    {
        ret = LanguageType::KOREAN;
    }
    else if (0 == strcmp("ja", pLanguageName))
    {
        ret = LanguageType::JAPANESE;
    }
    else if (0 == strcmp("hu", pLanguageName))
    {
        ret = LanguageType::HUNGARIAN;
    }
    else if (0 == strcmp("pt", pLanguageName))
    {
        ret = LanguageType::PORTUGUESE;
    }
    else if (0 == strcmp("ar", pLanguageName))
    {
        ret = LanguageType::ARABIC;
    }
    else if (0 == strcmp("nb", pLanguageName))
    {
        ret = LanguageType::NORWEGIAN;
    }
    else if (0 == strcmp("pl", pLanguageName))
    {
        ret = LanguageType::POLISH;
    }
    else if (0 == strcmp("tr", pLanguageName))
    {
        ret = LanguageType::TURKISH;
    }
    else if (0 == strcmp("uk", pLanguageName))
    {
        ret = LanguageType::UKRAINIAN;
    }
    else if (0 == strcmp("ro", pLanguageName))
    {
        ret = LanguageType::ROMANIAN;
    }
    else if (0 == strcmp("bg", pLanguageName))
    {
        ret = LanguageType::BULGARIAN;
    }
    return ret;
}

Application::Platform Application::getPlatform() const
{
    return Platform::ANDROIDOS;
}

void Application::onCreateView(int& /*x*/, int& /*y*/, int& /*width*/, int& /*height*/, 
                              PixelFormat& /*pixelformat*/, DepthFormat& /*depthFormat*/, int& /*multisamplingCount*/)
{

}

bool Application::openURL(const std::string &url)
{
    return openURLJNI(url);
}

std::string Application::getSystemVersion()
{
    return getSystemVersionJNI();
}

NS_CC_END

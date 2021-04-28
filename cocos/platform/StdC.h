/****************************************************************************
 Copyright (c) 2020 cocos.com Xiamen Yaji Software Co., Ltd.
 Copyright (c) undefined-2021 Xiamen Yaji Software Co., Ltd.

 http://www.cocos.com

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated engine source code (the "Software"), a limited,
 worldwide, royalty-free, non-assignable, revocable and non-exclusive license
 to use Cocos Creator solely to develop games on your target platforms. You shall
 not use Cocos Creator software for developing other software or tools that's
 used for developing games. You are not granted to publish, distribute,
 sublicense, and/or sell copies of Cocos Creator.

 The software or tools in this License Agreement are licensed, not sold.
 Xiamen Yaji Software Co., Ltd. reserves all rights not expressly granted to you.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 THE SOFTWARE.
****************************************************************************/

#pragma once

#if CC_PLATFORM == CC_PLATFORM_WINDOWS

    #define WIN32_LEAN_AND_MEAN
    #include <Windows.h>

    #if _MSC_VER < 1800
        #if !defined(isnan)
            #define isnan _isnan
        #endif
    #endif

    #include <math.h>
    #include <stdarg.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <time.h>

    #if _MSC_VER >= 1600
        #include <stdint.h>
    #else
        #include "platform/win32/compat/stdint.h"
    #endif

    // Conflicted with ParticleSystem::PositionType::RELATIVE, so we need to undef it.
    #ifdef RELATIVE
        #undef RELATIVE
    #endif

    // Conflicted with CCBReader::SizeType::RELATIVE and CCBReader::ScaleType::RELATIVE, so we need to undef it.
    #ifdef ABSOLUTE
        #undef ABSOLUTE
    #endif

    // Conflicted with HttpRequest::Type::DELETE, so we need to undef it.
    #ifdef DELETE
        #undef DELETE
    #endif

    #undef min
    #undef max
#else

    #include <float.h>
    #include <math.h>
    #include <stdarg.h>
    #include <stdint.h>
    #include <stdio.h>
    #include <stdlib.h>
    #include <string.h>
    #include <sys/time.h>
    #include <time.h>

#endif // CC_PLATFORM == CC_PLATFORM_WINDOWS

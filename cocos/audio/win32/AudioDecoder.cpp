/****************************************************************************
 Copyright (c) 2016 Chukong Technologies Inc.
 Copyright (c) 2017-2021 Xiamen Yaji Software Co., Ltd.

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

#include "audio/win32/AudioDecoder.h"
#include "audio/win32/AudioMacros.h"
#include "platform/FileUtils.h"

#define LOG_TAG "AudioDecoder"

namespace cc {

AudioDecoder::AudioDecoder()
: _isOpened(false), _totalFrames(0), _bytesPerFrame(0), _sampleRate(0), _channelCount(0) {
}

AudioDecoder::~AudioDecoder() {
}

bool AudioDecoder::isOpened() const {
    return _isOpened;
}

uint32_t AudioDecoder::readFixedFrames(uint32_t framesToRead, char *pcmBuf) {
    uint32_t framesRead     = 0;
    uint32_t framesReadOnce = 0;
    do {
        framesReadOnce = read(framesToRead - framesRead, pcmBuf + framesRead * _bytesPerFrame);
        framesRead += framesReadOnce;
    } while (framesReadOnce != 0 && framesRead < framesToRead);

    if (framesRead < framesToRead) {
        memset(pcmBuf + framesRead * _bytesPerFrame, 0x00, (framesToRead - framesRead) * _bytesPerFrame);
    }

    return framesRead;
}

uint32_t AudioDecoder::getTotalFrames() const {
    return _totalFrames;
}

uint32_t AudioDecoder::getBytesPerFrame() const {
    return _bytesPerFrame;
}

uint32_t AudioDecoder::getSampleRate() const {
    return _sampleRate;
}

uint32_t AudioDecoder::getChannelCount() const {
    return _channelCount;
}

} // namespace cc

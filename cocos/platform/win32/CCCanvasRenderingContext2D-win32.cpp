#include "platform/CCCanvasRenderingContext2D.h"
#include "base/ccTypes.h"
#include "base/csscolorparser.hpp"

#include "cocos/scripting/js-bindings/jswrapper/SeApi.h"

#include "platform/CCFileUtils.h"
#include "platform/desktop/CCGLView-desktop.h"

#include <regex>

using namespace cocos2d;

enum class CanvasTextAlign {
    LEFT,
    CENTER,
    RIGHT
};

enum class CanvasTextBaseline {
    TOP,
    MIDDLE,
    BOTTOM
};

namespace {
  void fillRectWithColor(uint8_t* buf, uint32_t totalWidth, uint32_t totalHeight, uint32_t x, uint32_t y, uint32_t width, uint32_t height, uint8_t r, uint8_t g, uint8_t b)
  {
    assert(x + width <= totalWidth);
    assert(y + height <= totalHeight);

    uint32_t y0 = totalHeight - (y + height);
    uint32_t y1 = totalHeight - y;
    uint8_t* p;
    for (uint32_t offsetY = y0; offsetY < y1; ++offsetY)
    {
      for (uint32_t offsetX = x; offsetX < (x + width); ++offsetX)
      {
        p = buf + (totalWidth * offsetY + offsetX) * 3;
        *p++ = r;
        *p++ = g;
        *p++ = b;
      }
    }
  }

  HWND getWin32Window()
  {
    auto glfwWindow = ((cocos2d::GLView*)cocos2d::Application::getInstance()->getView())->getGLFWWindow();
    return glfwGetWin32Window(glfwWindow);
  }
}

class CanvasRenderingContext2DImpl
{
public:
  CanvasRenderingContext2DImpl(HWND hWnd = nullptr) : _DC(nullptr)
    , _bmp(nullptr)
    , _font((HFONT)GetStockObject(DEFAULT_GUI_FONT))
    , _wnd(nullptr)
    , _savedDC(0)
    {
      _wnd = hWnd;
      HDC hdc = GetDC(_wnd);
      _DC = CreateCompatibleDC(hdc);
      ReleaseDC(_wnd, hdc);
    }

    ~CanvasRenderingContext2DImpl()
    {
      if (_DC)
      {
        DeleteDC(_DC);
      }
    }

    void recreateBuffer(float w, float h)
    {
        _bufferWidth = w;
        _bufferHeight = h;
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;

        int textureSize = _bufferWidth * _bufferHeight * 4;
        uint8_t* data = (uint8_t*)malloc(sizeof(uint8_t) * textureSize);
        memset(data, 0, textureSize);
        _imageData.fastSet(data, textureSize);
        // todo, create bitmap use data above
    }

    void beginPath()
    {
      _DC = BeginPaint(_wnd, &_paintStruct);
    }

    void closePath()
    {
      EndPaint(_wnd, &_paintStruct);
    }

    void moveTo(float x, float y)
    {
      MoveToEx(_DC, x, y, nullptr);
    }

    void lineTo(float x, float y)
    {
      LineTo(_DC, x, y);
    }

    void stroke()
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        //
    }

    void saveContext()
    {
      _savedDC = SaveDC(_DC);
    }

    void restoreContext()
    {
      RestoreDC(_DC, _savedDC);
    }

    void clearRect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        if (_imageData.isNull())
          return;

        uint8_t* buffer = _imageData.getBytes();
        if (buffer)
        {
          uint8_t r = 0;
          uint8_t g = 0;
          uint8_t b = 0;
          fillRectWithColor(buffer, (uint32_t)_bufferWidth, (uint32_t)_bufferHeight, (uint32_t)x, (uint32_t)y, (uint32_t)w, (uint32_t)h, r, g, b);
        }
    }

    void fillRect(float x, float y, float w, float h)
    {
        if (_bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;

        uint8_t* buffer = _imageData.getBytes();
        if (buffer)
        {
          uint8_t r = _fillStyle.r * 255.0f;
          uint8_t g = _fillStyle.g * 255.0f;
          uint8_t b = _fillStyle.b * 255.0f;
          fillRectWithColor(buffer, (uint32_t)_bufferWidth, (uint32_t)_bufferHeight, (uint32_t)x, (uint32_t)y, (uint32_t)w, (uint32_t)h, r, g, b);
        }
    }

    void fillText(const std::string& text, float x, float y, float maxWidth)
    {
        if (text.empty() || _bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        // check
        //_imageData = _getTextureDataForText(text.c_str(), (int)x, (int)y, false);
        
    }

    void strokeText(const std::string& text, float x, float y, float maxWidth)
    {
        if (text.empty() || _bufferWidth < 1.0f || _bufferHeight < 1.0f)
            return;
        //
    }

    cocos2d::Size measureText(const std::string& text)
    {
        if (text.empty())
            return Size(0.0f, 0.0f);

        SIZE size;
        GetTextExtentPoint32(_DC, _utf8ToUtf16(text), text.size(), &size);
        SE_LOGD("CanvasRenderingContext2DImpl::measureText: %s, %d, %d\n", text.c_str(), size.cx, size.cy);
        return Size(size.cx, size.cx);
    }

    void updateFont(const std::string& fontName, float fontSize, bool bold = false)
    {
      bool bRet = false;
      do
      {
        _fontName = fontName;
        _fontSize = fontSize;
        std::string fontPath;
        HFONT       hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
        LOGFONTA    tNewFont = { 0 };
        LOGFONTA    tOldFont = { 0 };
        GetObjectA(hDefFont, sizeof(tNewFont), &tNewFont);
        if (!_fontName.empty())
        {
          // create font from ttf file
          if (FileUtils::getInstance()->getFileExtension(_fontName) == ".ttf")
          {
            fontPath = FileUtils::getInstance()->fullPathForFilename(_fontName.c_str());
            int nFindPos = _fontName.rfind("/");
            _fontName = &_fontName[nFindPos + 1];
            nFindPos = _fontName.rfind(".");
            _fontName = _fontName.substr(0, nFindPos);
          }
          else
          {
            auto nFindPos = fontName.rfind("/");
            if (nFindPos != fontName.npos)
            {
              if (fontName.length() == nFindPos + 1)
              {
                _fontName = "";
              }
              else
              {
                _fontName = &_fontName[nFindPos + 1];
              }
            }
          }
          tNewFont.lfCharSet = DEFAULT_CHARSET;
          strcpy_s(tNewFont.lfFaceName, LF_FACESIZE, _fontName.c_str());
        }

        if (_fontSize)
        {
          tNewFont.lfHeight = -_fontSize;
        }

        if (bold)
        {
          tNewFont.lfWeight = FW_BOLD;
        }
        else
        {
          tNewFont.lfWeight = FW_NORMAL;
        }

        GetObjectA(_font, sizeof(tOldFont), &tOldFont);

        if (tOldFont.lfHeight == tNewFont.lfHeight
          && tOldFont.lfWeight == tNewFont.lfWeight
          && 0 == strcmp(tOldFont.lfFaceName, tNewFont.lfFaceName))
        {
          bRet = true;
          break;
        }

        // delete old font
        _removeCustomFont();

        if (fontPath.size() > 0)
        {
          _curFontPath = fontPath;
          wchar_t * pwszBuffer = _utf8ToUtf16(_curFontPath);
          if (pwszBuffer)
          {
            if (AddFontResource(pwszBuffer))
            {
              SendMessage(_wnd, WM_FONTCHANGE, 0, 0);
            }
            delete[] pwszBuffer;
            pwszBuffer = nullptr;
          }
        }

        _font = nullptr;

        // disable Cleartype
        tNewFont.lfQuality = ANTIALIASED_QUALITY;

        // create new font
        _font = CreateFontIndirectA(&tNewFont);
        if (!_font)
        {
          // create failed, use default font
          _font = hDefFont;
          break;
        }

        bRet = true;
      } while (0);
    }

    void setTextAlign(CanvasTextAlign align)
    {
      _textAlign = align;
    }

    void setTextBaseline(CanvasTextBaseline baseline)
    {
      _textBaseLine = baseline;
    }

    void setFillStyle(float r, float g, float b, float a)
    {
        _fillStyle.r = r;
        _fillStyle.g = g;
        _fillStyle.b = b;
        _fillStyle.a = a;
    }

    void setStrokeStyle(float r, float g, float b, float a)
    {
        _strokeStyle.r = r;
        _strokeStyle.g = g;
        _strokeStyle.b = b;
        _strokeStyle.a = a;
    }

    void setLineWidth(float lineWidth)
    {
      //check, use Pen to support this
    }

    const Data& getDataRef() const
    {
        return _imageData;
    }

    HDC _DC;
    HBITMAP _bmp;
private:

    Data _imageData;
    HFONT   _font;
    HWND    _wnd;
    PAINTSTRUCT _paintStruct;
    std::string _curFontPath;
    int _savedDC;
    float _bufferWidth = 0.0f;
    float _bufferHeight = 0.0f;

    std::string _fontName;
    int _fontSize;
    CanvasTextAlign _textAlign;
    CanvasTextBaseline _textBaseLine;
    cocos2d::Color4F _fillStyle;
    cocos2d::Color4F _strokeStyle;

    wchar_t * _utf8ToUtf16(const std::string& str)
    {
      wchar_t * pwszBuffer = nullptr;
      do
      {
        if (str.empty())
        {
          break;
        }
        // utf-8 to utf-16
        int nLen = str.size();
        int nBufLen = nLen + 1;
        pwszBuffer = new wchar_t[nBufLen];
        CC_BREAK_IF(!pwszBuffer);
        memset(pwszBuffer, 0, nBufLen);
        nLen = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), nLen, pwszBuffer, nBufLen);
        pwszBuffer[nLen] = '\0';
      } while (0);
      return pwszBuffer;

    }

    void _removeCustomFont()
    {
      HFONT hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
      if (hDefFont != _font)
      {
        DeleteObject(_font);
        _font = hDefFont;
      }
      // release temp font resource
      if (_curFontPath.size() > 0)
      {
        wchar_t * pwszBuffer = _utf8ToUtf16(_curFontPath);
        if (pwszBuffer)
        {
          RemoveFontResource(pwszBuffer);
          SendMessage(_wnd, WM_FONTCHANGE, 0, 0);
          delete[] pwszBuffer;
          pwszBuffer = nullptr;
        }
        _curFontPath.clear();
      }
    }

    int _drawText(const char * pszText, SIZE& tSize, bool enableWrap, int overflow)
    {
      int nRet = 0;
      wchar_t * pwszBuffer = nullptr;
      wchar_t* fixedText = nullptr;
      do
      {
        CC_BREAK_IF(!pszText);

        DWORD dwFmt = DT_WORDBREAK;
        if (!enableWrap) {
          dwFmt |= DT_SINGLELINE;
        }
        DWORD dwHoriFlag = (int)_textAlign & 0x0f;
        DWORD dwVertFlag = ((int)_textBaseLine & 0xf0) >> 4;

        switch (dwHoriFlag)
        {
        case 1: // left
          dwFmt |= DT_LEFT;
          break;
        case 2: // right
          dwFmt |= DT_RIGHT;
          break;
        case 3: // center
          dwFmt |= DT_CENTER;
          break;
        }

        int nLen = strlen(pszText);
        // utf-8 to utf-16
        int nBufLen = nLen + 1;
        pwszBuffer = new wchar_t[nBufLen];
        CC_BREAK_IF(!pwszBuffer);

        memset(pwszBuffer, 0, sizeof(wchar_t)*nBufLen);
        nLen = MultiByteToWideChar(CP_UTF8, 0, pszText, nLen, pwszBuffer, nBufLen);

        if (strchr(pszText, '&'))
        {
          fixedText = new wchar_t[nLen * 2 + 1];
          int fixedIndex = 0;
          for (int index = 0; index < nLen; ++index)
          {
            if (pwszBuffer[index] == '&')
            {
              fixedText[fixedIndex] = '&';
              fixedText[fixedIndex + 1] = '&';
              fixedIndex += 2;
            }
            else
            {
              fixedText[fixedIndex] = pwszBuffer[index];
              fixedIndex += 1;
            }
          }
          fixedText[fixedIndex] = '\0';
          nLen = fixedIndex;
        }

        SIZE newSize;
        if (fixedText)
        {
          newSize = _sizeWithText(fixedText, nLen, dwFmt, tSize.cx, tSize.cy, enableWrap, overflow);
        }
        else
        {
          newSize = _sizeWithText(pwszBuffer, nLen, dwFmt, tSize.cx, tSize.cy, enableWrap, overflow);
        }

        RECT rcText = { 0 };
        // if content width is 0, use text size as content size
        if (tSize.cx <= 0)
        {
          tSize = newSize;
          rcText.right = newSize.cx;
          rcText.bottom = newSize.cy;
        }
        else
        {

          LONG offsetX = 0;
          LONG offsetY = 0;
          rcText.right = newSize.cx; // store the text width to rectangle

                                     // calculate text horizontal offset
          if (1 != dwHoriFlag          // and text isn't align to left
            && newSize.cx < tSize.cx)   // and text's width less then content width,
          {                               // then need adjust offset of X.
            offsetX = (2 == dwHoriFlag) ? tSize.cx - newSize.cx     // align to right
              : (tSize.cx - newSize.cx) / 2;                      // align to center
          }

          // if content height is 0, use text height as content height
          // else if content height less than text height, use content height to draw text
          if (tSize.cy <= 0)
          {
            tSize.cy = newSize.cy;
            dwFmt |= DT_NOCLIP;
            rcText.bottom = newSize.cy; // store the text height to rectangle
          }
          else if (tSize.cy < newSize.cy)
          {
            // content height larger than text height need, clip text to rect
            rcText.bottom = tSize.cy;
          }
          else
          {
            rcText.bottom = newSize.cy; // store the text height to rectangle

                                        // content larger than text, need adjust vertical position
            dwFmt |= DT_NOCLIP;

            // calculate text vertical offset
            offsetY = (2 == dwVertFlag) ? tSize.cy - newSize.cy     // align to bottom
              : (3 == dwVertFlag) ? (tSize.cy - newSize.cy) / 2   // align to middle
              : 0;                                                // align to top
          }

          if (offsetX || offsetY)
          {
            OffsetRect(&rcText, offsetX, offsetY);
          }
        }

        CC_BREAK_IF(!_prepareBitmap(tSize.cx, tSize.cy));

        // draw text
        HGDIOBJ hOldFont = SelectObject(_DC, _font);
        HGDIOBJ hOldBmp = SelectObject(_DC, _bmp);

        SetBkMode(_DC, TRANSPARENT);
        SetTextColor(_DC, RGB(255, 255, 255)); // white color

                                               // draw text
        if (fixedText)
        {
          nRet = DrawTextW(_DC, fixedText, nLen, &rcText, dwFmt);
        }
        else
        {
          nRet = DrawTextW(_DC, pwszBuffer, nLen, &rcText, dwFmt);
        }

        SelectObject(_DC, hOldBmp);
        SelectObject(_DC, hOldFont);
      } while (0);
      CC_SAFE_DELETE_ARRAY(pwszBuffer);
      delete[] fixedText;

      return nRet;
    }

    SIZE _sizeWithText(const wchar_t * pszText,
      int nLen,
      DWORD dwFmt,
      LONG nWidthLimit,
      LONG nHeightLimit,
      bool enableWrap,
      int overflow)
    {
      SIZE tRet = { 0 };
      do
      {
        CC_BREAK_IF(!pszText || nLen <= 0);

        RECT rc = { 0, 0, 0, 0 };
        DWORD dwCalcFmt = DT_CALCRECT;
        if (!enableWrap)
        {
          dwCalcFmt |= DT_SINGLELINE;
        }

        if (nWidthLimit > 0)
        {
          rc.right = nWidthLimit;
          dwCalcFmt |= DT_WORDBREAK
            | (dwFmt & DT_CENTER)
            | (dwFmt & DT_RIGHT);
        }
        if (overflow == 2)
        {
          LONG actualWidth = nWidthLimit + 1;
          LONG actualHeight = nHeightLimit + 1;
          int newFontSize = _fontSize + 1;

          while (actualWidth > nWidthLimit || actualHeight > nHeightLimit)
          {
            if (newFontSize <= 0)
            {
              break;
            }
            this->updateFont(_fontName, newFontSize);
            // use current font to measure text extent
            HGDIOBJ hOld = SelectObject(_DC, _font);
            rc.right = nWidthLimit;
            // measure text size
            DrawTextW(_DC, pszText, nLen, &rc, dwCalcFmt);
            SelectObject(_DC, hOld);

            actualWidth = rc.right;
            actualHeight = rc.bottom;
            newFontSize = newFontSize - 1;
          }
        }
        else
        {
          // use current font to measure text extent
          HGDIOBJ hOld = SelectObject(_DC, _font);

          // measure text size
          DrawTextW(_DC, pszText, nLen, &rc, dwCalcFmt);
          SelectObject(_DC, hOld);
        }

        tRet.cx = rc.right;
        tRet.cy = rc.bottom;

      } while (0);

      return tRet;
    }

    bool _prepareBitmap(int nWidth, int nHeight)
    {
      // release bitmap
      if (_bmp)
      {
        DeleteObject(_bmp);
        _bmp = nullptr;
      }
      if (nWidth > 0 && nHeight > 0)
      {
        _bmp = CreateBitmap(nWidth, nHeight, 1, 32, nullptr);
        if (!_bmp)
        {
          return false;
        }
      }
      return true;
    }

    Data _getTextureDataForText(const char * text, int width, int height, bool hasPremultipliedAlpha)
    {
      Data ret;
      do
      {

        updateFont(_fontName, _fontSize, false);

        // draw text
        // does changing to SIZE here affects the font size by rounding from float?
        SIZE size = { (LONG)width,(LONG)height };
        _drawText(text, size, false, false);

        int dataLen = size.cx * size.cy * 4;
        unsigned char* dataBuf = (unsigned char*)malloc(sizeof(unsigned char) * dataLen);
        CC_BREAK_IF(!dataBuf);

        struct
        {
          BITMAPINFOHEADER bmiHeader;
          int mask[4];
        } bi = { 0 };
        bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
        CC_BREAK_IF(!GetDIBits(_DC, _bmp, 0, 0,
          nullptr, (LPBITMAPINFO)&bi, DIB_RGB_COLORS));

        width = (short)size.cx;
        height = (short)size.cy;

        // copy pixel data
        bi.bmiHeader.biHeight = (bi.bmiHeader.biHeight > 0)
          ? -bi.bmiHeader.biHeight : bi.bmiHeader.biHeight;
        GetDIBits(_DC, _bmp, 0, height, dataBuf,
          (LPBITMAPINFO)&bi, DIB_RGB_COLORS);

        uint8_t r = _fillStyle.r * 255.0f;
        uint8_t g = _fillStyle.g * 255.0f;
        uint8_t b = _fillStyle.b * 255.0f;
        COLORREF textColor = (b << 16 | g << 8 | r) & 0x00ffffff;
        // check
        float alpha = 128 / 255.0f;
        COLORREF * pPixel = nullptr;
        for (int y = 0; y < height; ++y)
        {
          pPixel = (COLORREF *)dataBuf + y * width;
          for (int x = 0; x < width; ++x)
          {
            COLORREF& clr = *pPixel;
            clr = ((BYTE)(GetRValue(clr) * alpha) << 24) | textColor;
            ++pPixel;
          }
        }

        ret.fastSet(dataBuf, dataLen);
        hasPremultipliedAlpha = false;
      } while (0);

      return ret;
    }
};

NS_CC_BEGIN

CanvasGradient::CanvasGradient()
{
    SE_LOGD("CanvasGradient constructor: %p\n", this);
}

CanvasGradient::~CanvasGradient()
{
    SE_LOGD("CanvasGradient destructor: %p\n", this);
}

void CanvasGradient::addColorStop(float offset, const std::string& color)
{
    SE_LOGD("CanvasGradient::addColorStop: %p\n", this);
}

// CanvasRenderingContext2D

CanvasRenderingContext2D::CanvasRenderingContext2D(float width, float height)
: __width(width)
, __height(height)
{
    SE_LOGD("CanvasRenderingContext2D constructor: %p, width: %f, height: %f\n", this, width, height);
    _impl = new CanvasRenderingContext2DImpl(getWin32Window());
    recreateBufferIfNeeded();
}

CanvasRenderingContext2D::~CanvasRenderingContext2D()
{
    SE_LOGD("CanvasRenderingContext2D destructor: %p\n", this);
    delete _impl;
}

void CanvasRenderingContext2D::recreateBufferIfNeeded()
{
    if (_isBufferSizeDirty)
    {
        _isBufferSizeDirty = false;
        SE_LOGD("Recreate buffer %p, w: %f, h:%f\n", this, __width, __height);
        _impl->recreateBuffer(__width, __height);
        if (_canvasBufferUpdatedCB != nullptr)
            _canvasBufferUpdatedCB(_impl->getDataRef());
    }
}

void CanvasRenderingContext2D::clearRect(float x, float y, float width, float height)
{
    SE_LOGD("CanvasRenderingContext2D::clearRect: %p, %f, %f, %f, %f\n", this, x, y, width, height);
    recreateBufferIfNeeded();
    _impl->clearRect(x, y, width, height);
}

void CanvasRenderingContext2D::fillRect(float x, float y, float width, float height)
{
    recreateBufferIfNeeded();
    _impl->fillRect(x, y, width, height);

    if (_canvasBufferUpdatedCB != nullptr)
        _canvasBufferUpdatedCB(_impl->getDataRef());
}

void CanvasRenderingContext2D::fillText(const std::string& text, float x, float y, float maxWidth)
{
    SE_LOGD("CanvasRenderingContext2D::fillText: %s, %f, %f, %f\n", text.c_str(), x, y, maxWidth);
    if (text.empty())
        return;
    recreateBufferIfNeeded();

    _impl->fillText(text, x, y, maxWidth);
    if (_canvasBufferUpdatedCB != nullptr)
        _canvasBufferUpdatedCB(_impl->getDataRef());
}

void CanvasRenderingContext2D::strokeText(const std::string& text, float x, float y, float maxWidth)
{
    SE_LOGD("CanvasRenderingContext2D::strokeText: %s, %f, %f, %f\n", text.c_str(), x, y, maxWidth);
    if (text.empty())
        return;
    recreateBufferIfNeeded();

    _impl->strokeText(text, x, y, maxWidth);

   if (_canvasBufferUpdatedCB != nullptr)
       _canvasBufferUpdatedCB(_impl->getDataRef());
}

cocos2d::Size CanvasRenderingContext2D::measureText(const std::string& text)
{
    SE_LOGD("CanvasRenderingContext2D::measureText: %s\n", text.c_str());
    return _impl->measureText(text);
}

CanvasGradient* CanvasRenderingContext2D::createLinearGradient(float x0, float y0, float x1, float y1)
{
    return nullptr;
}

void CanvasRenderingContext2D::save()
{
    _impl->saveContext();
}

void CanvasRenderingContext2D::beginPath()
{
    _impl->beginPath();
}

void CanvasRenderingContext2D::closePath()
{
    _impl->closePath();
}

void CanvasRenderingContext2D::moveTo(float x, float y)
{
    _impl->moveTo(x, y);
}

void CanvasRenderingContext2D::lineTo(float x, float y)
{
    _impl->lineTo(x, y);
}

void CanvasRenderingContext2D::stroke()
{
    _impl->stroke();

    if (_canvasBufferUpdatedCB != nullptr)
        _canvasBufferUpdatedCB(_impl->getDataRef());
}

void CanvasRenderingContext2D::restore()
{
    _impl->restoreContext();
}

void CanvasRenderingContext2D::setCanvasBufferUpdatedCallback(const CanvasBufferUpdatedCallback& cb)
{
    _canvasBufferUpdatedCB = cb;
}

void CanvasRenderingContext2D::set__width(float width)
{
    SE_LOGD("CanvasRenderingContext2D::set__width: %f\n", width);
    __width = width;
    _isBufferSizeDirty = true;
}

void CanvasRenderingContext2D::set__height(float height)
{
    SE_LOGD("CanvasRenderingContext2D::set__height: %f\n", height);
    __height = height;
    _isBufferSizeDirty = true;
}

void CanvasRenderingContext2D::set_lineWidth(float lineWidth)
{
    _lineWidth = lineWidth;
    _impl->setLineWidth(lineWidth);
}

void CanvasRenderingContext2D::set_lineJoin(const std::string& lineJoin)
{
     SE_LOGE("%s isn't implemented!\n", __FUNCTION__);
}

void CanvasRenderingContext2D::set_font(const std::string& font)
{
    if (_font != font)
    {
        _font = font;

        std::string boldStr;
        std::string fontName = "Arial";
        std::string fontSizeStr = "30";

        std::regex re("(bold)?\\s*(\\d+)px\\s+(\\w+)");
        std::match_results<std::string::const_iterator> results;
        if (std::regex_search(_font.cbegin(), _font.cend(), results, re))
        {
            boldStr = results[1].str();
            fontSizeStr = results[2].str();
            fontName = results[3].str();
        }

        float fontSize = atof(fontSizeStr.c_str());
        _impl->updateFont(fontName, fontSize, !boldStr.empty());
    }
}

void CanvasRenderingContext2D::set_textAlign(const std::string& textAlign)
{
    SE_LOGD("CanvasRenderingContext2D::set_textAlign: %s\n", textAlign.c_str());
    if (textAlign == "left")
    {
        _impl->setTextAlign(CanvasTextAlign::LEFT);
    }
    else if (textAlign == "center" || textAlign == "middle")
    {
        _impl->setTextAlign(CanvasTextAlign::CENTER);
    }
    else if (textAlign == "right")
    {
        _impl->setTextAlign(CanvasTextAlign::RIGHT);
    }
    else
    {
        assert(false);
    }
}

void CanvasRenderingContext2D::set_textBaseline(const std::string& textBaseline)
{
    SE_LOGD("CanvasRenderingContext2D::set_textBaseline: %s\n", textBaseline.c_str());
    if (textBaseline == "top")
    {
        _impl->setTextBaseline(CanvasTextBaseline::TOP);
    }
    else if (textBaseline == "middle")
    {
        _impl->setTextBaseline(CanvasTextBaseline::MIDDLE);
    }
    else if (textBaseline == "bottom" || textBaseline == "alphabetic") //TODO:cjh, how to deal with alphabetic, currently we handle it as bottom mode.
    {
        _impl->setTextBaseline(CanvasTextBaseline::BOTTOM);
    }
    else
    {
        assert(false);
    }
}

void CanvasRenderingContext2D::set_fillStyle(const std::string& fillStyle)
{
    CSSColorParser::Color color = CSSColorParser::parse(fillStyle);
    _impl->setFillStyle(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a);
    SE_LOGD("CanvasRenderingContext2D::set_fillStyle: %s, (%d, %d, %d, %f)\n", fillStyle.c_str(), color.r, color.g, color.b, color.a);
}

void CanvasRenderingContext2D::set_strokeStyle(const std::string& strokeStyle)
{
    CSSColorParser::Color color = CSSColorParser::parse(strokeStyle);
    _impl->setStrokeStyle(color.r/255.0f, color.g/255.0f, color.b/255.0f, color.a);
}

void CanvasRenderingContext2D::set_globalCompositeOperation(const std::string& globalCompositeOperation)
{
    SE_LOGE("%s isn't implemented!\n", __FUNCTION__);
}

// transform
//TODO:

void CanvasRenderingContext2D::translate(float x, float y)
{
    SE_LOGE("%s isn't implemented!\n", __FUNCTION__);
}

void CanvasRenderingContext2D::scale(float x, float y)
{
    SE_LOGE("%s isn't implemented!\n", __FUNCTION__);
}

void CanvasRenderingContext2D::rotate(float angle)
{
    SE_LOGE("%s isn't implemented!\n", __FUNCTION__);
}

void CanvasRenderingContext2D::transform(float a, float b, float c, float d, float e, float f)
{
    SE_LOGE("%s isn't implemented!\n", __FUNCTION__);
}

void CanvasRenderingContext2D::setTransform(float a, float b, float c, float d, float e, float f)
{
    SE_LOGE("%s isn't implemented!\n", __FUNCTION__);
}

NS_CC_END

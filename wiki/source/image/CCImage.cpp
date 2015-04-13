/****************************************************************************
Copyright (c) 2010 cocos2d-x.org

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
#define __CC_PLATFORM_IMAGE_CPP__
#include "platform/CCImageCommon_cpp.h"
#include <map>

#define TP_FAMILY_NAME 1
#define TP_FULL_FONT_NAME 4

NS_CC_BEGIN

std::map<const char*, const char*> map_font;

struct TableDirectory {
	char name[5]; //table name
	int checkSum; //Check sum
	int offset; //Offset from beginning of file
	int length; //length of the table in bytes
};

struct NameTableHeader {
	short fSelector; //format selector. Always 0
	short nRCount; //Name Records count
	short storageOffset; //Offset for strings storage,
};

struct NameRecord {
	short platformID;
	short encodingID;
	short languageID;
	short nameID;
	short stringLength;
	short stringOffset; //from start of storage area
};

inline char* strlwr(char* str)
{
	char* orig = str;
	for (; *str != '\0'; str++)
		*str = tolower(*str);
	return orig;
}

void freadShort(short &i, FILE *f) {
	fread(&i, sizeof(short), 1, f);

	short b, c;
	b = (i >> 8) & 0x00FF;
	c = (i << 8) & (~0x00FF);
	i = b | c;
}

void freadInt(int &i, FILE *f) {
	fread(&i, sizeof(int), 1, f);

	unsigned char b[4];
	b[0] = (byte)(i);
	b[1] = (byte)(i >> 8);
	b[2] = (byte)(i >> 16);
	b[3] = (byte)(i >> 24);
	i = (b[3] | b[2] << 8 | b[1] << 16 | b[0] << 24);
}

bool parse(const char* file_path) {
	FILE *f = fopen(file_path, "rb");
	if (f == NULL) {
		return false;
	}

	short majorVersion, minorVersion, numOfTables;
	freadShort(majorVersion, f);
	freadShort(minorVersion, f);
	freadShort(numOfTables, f);

	if (majorVersion != 1 || minorVersion != 0) {
		return false;
	}

	fseek(f, 12, SEEK_SET);

	bool found = false;

	TableDirectory tableDirectory;
	memset(tableDirectory.name, 0, 5);
	for (int i = 0; i < numOfTables; i++) {
		fread(tableDirectory.name, 4, 1, f);

		freadInt(tableDirectory.checkSum, f);
		freadInt(tableDirectory.offset, f);
		freadInt(tableDirectory.length, f);

		if (!strcmp(strlwr(tableDirectory.name), "name")) {
			found = true;
			break;
		}
		else if (tableDirectory.name == NULL || strlen(tableDirectory.name) == 0) {
			break;
		}
	}

	// not found table of name
	if (!found) {
		return false;
	}

	fseek(f, tableDirectory.offset, SEEK_SET);
	NameTableHeader nameTableHeader;
	freadShort(nameTableHeader.fSelector, f);
	freadShort(nameTableHeader.nRCount, f);
	freadShort(nameTableHeader.storageOffset, f);

	char *font_name = new char[32];
	char *family_name = new char[32];
	memset(font_name, 0, 32);
	memset(family_name, 0, 32);

	NameRecord nameRecord;
	char flag = 0;
	for (int i = 0; i < nameTableHeader.nRCount; i++) {
		freadShort(nameRecord.platformID, f);
		freadShort(nameRecord.encodingID, f);
		freadShort(nameRecord.languageID, f);
		freadShort(nameRecord.nameID, f);
		freadShort(nameRecord.stringLength, f);
		freadShort(nameRecord.stringOffset, f);

		long pos = ftell(f);
		char *buf = new char[nameRecord.stringLength + 1];
		memset(buf, 0, nameRecord.stringLength + 1);

		long vpos = tableDirectory.offset + nameRecord.stringOffset + nameTableHeader.storageOffset;
		fseek(f, vpos, SEEK_SET);
		fread(buf, nameRecord.stringLength, 1, f);

		if (nameRecord.nameID == TP_FULL_FONT_NAME) {
			if (!strlen(font_name)) {
				strcpy(font_name, buf);
				flag |= 2;
				break;
			}
		}
		else if (nameRecord.nameID == TP_FAMILY_NAME) {
			if (!strlen(family_name)) {
				strcpy(family_name, buf);
				flag |= 1;
			}
		}

		fseek(f, pos, SEEK_SET);
	}

	if (flag & 2) {
		map_font[file_path] = font_name;
	}
	else if (flag & 1) {
		map_font[file_path] = family_name;
	}
	else {
		return false;
	}

	return true;
}

const char* getFontName(const char* file_name) {
	if (!map_font.count(file_name)) {
		if (!parse(file_name))
			return NULL;
	}

	return map_font[file_name];
}

/**
@brief    A memory DC which uses to draw text on bitmap.
*/
class BitmapDC
{
public:
	BitmapDC(HWND hWnd = NULL)
		: m_hDC(NULL)
		, m_hBmp(NULL)
		, m_hFont((HFONT)GetStockObject(DEFAULT_GUI_FONT))
		, m_hWnd(NULL)
	{
		m_hWnd = hWnd;
		HDC hdc = GetDC(hWnd);
		m_hDC = CreateCompatibleDC(hdc);
		ReleaseDC(hWnd, hdc);
	}

	~BitmapDC()
	{
		prepareBitmap(0, 0);
		if (m_hDC)
		{
			DeleteDC(m_hDC);
		}
		HFONT hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
		if (hDefFont != m_hFont)
		{
			DeleteObject(m_hFont);
			m_hFont = hDefFont;
		}
		// release temp font resource   
		if (m_curFontPath.size() > 0)
		{
			wchar_t * pwszBuffer = utf8ToUtf16(m_curFontPath);
			if (pwszBuffer)
			{
				RemoveFontResource(pwszBuffer);
				SendMessage(m_hWnd, WM_FONTCHANGE, 0, 0);
				delete[] pwszBuffer;
				pwszBuffer = NULL;
			}
		}
	}

	wchar_t * utf8ToUtf16(std::string nString)
	{
		wchar_t * pwszBuffer = NULL;
		do
		{
			if (nString.size() < 0)
			{
				break;
			}
			// utf-8 to utf-16
			int nLen = nString.size();
			int nBufLen = nLen + 1;
			pwszBuffer = new wchar_t[nBufLen];
			CC_BREAK_IF(!pwszBuffer);
			memset(pwszBuffer, 0, nBufLen);
			nLen = MultiByteToWideChar(CP_UTF8, 0, nString.c_str(), nLen, pwszBuffer, nBufLen);
			pwszBuffer[nLen] = '\0';
		} while (0);
		return pwszBuffer;

	}

	bool setFont(const char * pFontName = NULL, int nSize = 0)
	{
		bool bRet = false;
		do
		{
			std::string fontName = pFontName;
			std::string fontPath;
			HFONT       hDefFont = (HFONT)GetStockObject(DEFAULT_GUI_FONT);
			LOGFONTA    tNewFont = { 0 };
			LOGFONTA    tOldFont = { 0 };
			GetObjectA(hDefFont, sizeof(tNewFont), &tNewFont);
			if (fontName.c_str())
			{
				// create font from ttf file
				int nFindttf = fontName.find(".ttf");
				int nFindTTF = fontName.find(".TTF");
				if (nFindttf >= 0 || nFindTTF >= 0)
				{

					fontPath = CCFileUtils::sharedFileUtils()->fullPathForFilename(fontName.c_str());
					/*
					int nFindPos = fontName.rfind("/");
					fontName = &fontName[nFindPos+1];
					nFindPos = fontName.rfind(".");
					fontName = fontName.substr(0,nFindPos);*/

					fontName = getFontName(fontPath.c_str());
				}
				tNewFont.lfCharSet = DEFAULT_CHARSET;

				strcpy_s(tNewFont.lfFaceName, LF_FACESIZE, fontName.c_str());
			}
			if (nSize)
			{
				tNewFont.lfHeight = -nSize;
			}
			GetObjectA(m_hFont, sizeof(tOldFont), &tOldFont);

			if (tOldFont.lfHeight == tNewFont.lfHeight
				&& 0 == strcmp(tOldFont.lfFaceName, tNewFont.lfFaceName))
			{
				// already has the font 
				bRet = true;
				break;
			}

			// delete old font
			if (m_hFont != hDefFont)
			{
				DeleteObject(m_hFont);
				// release old font register
				if (m_curFontPath.size() > 0)
				{
					wchar_t * pwszBuffer = utf8ToUtf16(m_curFontPath);
					if (pwszBuffer)
					{
						if (RemoveFontResource(pwszBuffer))
						{
							SendMessage(m_hWnd, WM_FONTCHANGE, 0, 0);
						}
						delete[] pwszBuffer;
						pwszBuffer = NULL;
					}
				}
				if (fontPath.size() > 0)
					m_curFontPath = fontPath;
				else
					m_curFontPath.clear();
				// register temp font
				if (m_curFontPath.size() > 0)
				{
					wchar_t * pwszBuffer = utf8ToUtf16(m_curFontPath);
					if (pwszBuffer)
					{
						if (AddFontResource(pwszBuffer))
						{
							SendMessage(m_hWnd, WM_FONTCHANGE, 0, 0);
						}
						delete[] pwszBuffer;
						pwszBuffer = NULL;
					}
				}
			}
			m_hFont = NULL;

			// disable Cleartype
			tNewFont.lfQuality = ANTIALIASED_QUALITY;

			// create new font
			m_hFont = CreateFontIndirectA(&tNewFont);
			if (!m_hFont)
			{
				// create failed, use default font
				m_hFont = hDefFont;
				break;
			}

			bRet = true;
		} while (0);
		return bRet;
	}

	SIZE sizeWithText(const wchar_t * pszText, int nLen, DWORD dwFmt, LONG nWidthLimit)
	{
		SIZE tRet = { 0 };
		do
		{
			CC_BREAK_IF(!pszText || nLen <= 0);

			RECT rc = { 0, 0, 0, 0 };
			DWORD dwCalcFmt = DT_CALCRECT;

			if (nWidthLimit > 0)
			{
				rc.right = nWidthLimit;
				dwCalcFmt |= DT_WORDBREAK
					| (dwFmt & DT_CENTER)
					| (dwFmt & DT_RIGHT);
			}
			// use current font to measure text extent
			HGDIOBJ hOld = SelectObject(m_hDC, m_hFont);

			// measure text size
			DrawTextW(m_hDC, pszText, nLen, &rc, dwCalcFmt);
			SelectObject(m_hDC, hOld);

			tRet.cx = rc.right;
			tRet.cy = rc.bottom;
		} while (0);

		return tRet;
	}

	bool prepareBitmap(int nWidth, int nHeight)
	{
		// release bitmap
		if (m_hBmp)
		{
			DeleteObject(m_hBmp);
			m_hBmp = NULL;
		}
		if (nWidth > 0 && nHeight > 0)
		{
			m_hBmp = CreateBitmap(nWidth, nHeight, 1, 32, NULL);
			if (!m_hBmp)
			{
				return false;
			}
		}
		return true;
	}

	int drawText(const char * pszText, SIZE& tSize, CCImage::ETextAlign eAlign)
	{
		int nRet = 0;
		wchar_t * pwszBuffer = 0;
		do
		{
			CC_BREAK_IF(!pszText);

			DWORD dwFmt = DT_WORDBREAK;
			DWORD dwHoriFlag = eAlign & 0x0f;
			DWORD dwVertFlag = (eAlign & 0xf0) >> 4;

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

			SIZE newSize = sizeWithText(pwszBuffer, nLen, dwFmt, tSize.cx);

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

			CC_BREAK_IF(!prepareBitmap(tSize.cx, tSize.cy));

			// draw text
			HGDIOBJ hOldFont = SelectObject(m_hDC, m_hFont);
			HGDIOBJ hOldBmp = SelectObject(m_hDC, m_hBmp);

			SetBkMode(m_hDC, TRANSPARENT);
			SetTextColor(m_hDC, RGB(255, 255, 255)); // white color

			// draw text
			nRet = DrawTextW(m_hDC, pwszBuffer, nLen, &rcText, dwFmt);
			//DrawTextA(m_hDC, pszText, nLen, &rcText, dwFmt);

			SelectObject(m_hDC, hOldBmp);
			SelectObject(m_hDC, hOldFont);
		} while (0);
		CC_SAFE_DELETE_ARRAY(pwszBuffer);
		return nRet;
	}

	CC_SYNTHESIZE_READONLY(HDC, m_hDC, DC);
	CC_SYNTHESIZE_READONLY(HBITMAP, m_hBmp, Bitmap);
private:
	friend class CCImage;
	HFONT   m_hFont;
	HWND    m_hWnd;
	std::string m_curFontPath;
};

static BitmapDC& sharedBitmapDC()
{
	static BitmapDC s_BmpDC;
	return s_BmpDC;
}

bool CCImage::initWithString(
	const char *    pText,
	int             nWidth/* = 0*/,
	int             nHeight/* = 0*/,
	ETextAlign      eAlignMask/* = kAlignCenter*/,
	const char *    pFontName/* = nil*/,
	int             nSize/* = 0*/)
{
	bool bRet = false;
	do
	{
		CC_BREAK_IF(!pText);

		BitmapDC& dc = sharedBitmapDC();

		if (!dc.setFont(pFontName, nSize))
		{
			CCLog("Can't found font(%s), use system default", pFontName);
		}

		// draw text
		SIZE size = { nWidth, nHeight };
		CC_BREAK_IF(!dc.drawText(pText, size, eAlignMask));

		m_pData = new unsigned char[size.cx * size.cy * 4];
		CC_BREAK_IF(!m_pData);

		struct
		{
			BITMAPINFOHEADER bmiHeader;
			int mask[4];
		} bi = { 0 };
		bi.bmiHeader.biSize = sizeof(bi.bmiHeader);
		CC_BREAK_IF(!GetDIBits(dc.getDC(), dc.getBitmap(), 0, 0,
			NULL, (LPBITMAPINFO)&bi, DIB_RGB_COLORS));

		m_nWidth = (short)size.cx;
		m_nHeight = (short)size.cy;
		m_bHasAlpha = true;
		m_bPreMulti = false;
		m_nBitsPerComponent = 8;
		// copy pixed data
		bi.bmiHeader.biHeight = (bi.bmiHeader.biHeight > 0)
			? -bi.bmiHeader.biHeight : bi.bmiHeader.biHeight;
		GetDIBits(dc.getDC(), dc.getBitmap(), 0, m_nHeight, m_pData,
			(LPBITMAPINFO)&bi, DIB_RGB_COLORS);

		// change pixel's alpha value to 255, when it's RGB != 0

		COLORREF * pPixel = NULL;
		for (int y = 0; y < m_nHeight; ++y)
		{
			pPixel = (COLORREF *)m_pData + y * m_nWidth;
			for (int x = 0; x < m_nWidth; ++x)
			{
				COLORREF& clr = *pPixel;
				if (GetRValue(clr) || GetGValue(clr) || GetBValue(clr))
				{
					long mask = GetRValue(clr) << 24;
					//clr |= 0xff000000;
					clr |= mask;
				}
				++pPixel;
			}
		}

		bRet = true;
	} while (0);

	return bRet;
}

NS_CC_END

#pragma once
// client
#include "common.h"
#include <Windows.h>
#include <atlimage.h>

class ctool
{
public:
	static void dump(uchar* pdata, int nsize)
	{
		for (int i = 0; i < nsize; i++)
		{
			if (i && i % 16 == 0) puts("");
			printf("%02X ", pdata[i]);
		}
		puts("");
	}

	static int btoimage(CImage& image, string& strbuf)
	{
		uchar* pdata = (uchar*)strbuf.c_str();
		HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, 0);
		if (hmem == 0)
		{
			cout << "ÄÚ´æ²»×ã" << endl;
			Sleep(1);
			return -1;
		}
		IStream* pstream = 0;
		HRESULT hres = CreateStreamOnHGlobal(hmem, 1, &pstream);
		if (hres == S_OK)
		{
			ULONG len = 0;
			pstream->Write(pdata, strbuf.size(), &len);
			LARGE_INTEGER bg = { 0 };
			pstream->Seek(bg, STREAM_SEEK_SET, NULL);
			image.Destroy();
			image.Load(pstream);
			//m_image.Save(_T("b.jpeg"), Gdiplus::ImageFormatJPEG);
		}
		return hres;
	}



};


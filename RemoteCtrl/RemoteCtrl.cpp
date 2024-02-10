// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "serversocket.h"
#include <direct.h>
#include <io.h>
#include <atlimage.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// 唯一的应用程序对象

CWinApp theApp;
using namespace std;

csocket *pserver = csocket::getsocket();

void dump(uchar* pdata, int nsize)
{
	for (int i = 0; i < nsize; i++)
	{
		printf("%02X ", pdata[i]);
	}

	/*string st;
	for (int i = 0; i < nsize; i++)
	{
		char buf[8] = "";
		if (i > 0 && (i % 16) == 0) st += '\n';
		snprintf(buf, sizeof(buf), "%02X ", pdata[i]);
		st += buf;
	}
	st += '\n';
	OutputDebugStringA(st.c_str());*/
}

int makedriverinfo()
{
	string res;
	for (int i = 1; i <= 26; i++)
	{
		if (_chdrive(i) == 0)
		{
			if (res.size()) res += ',';
			res += 'A' + i - 1;
		}
	}

	cpacket pack = cpacket(1, (uchar*)res.c_str(), res.size());

	dump((uchar*)pack.data(), pack.size());
	//csocket::getsocket()->sendate(pack);
	return 0;
}

struct FILEINFO
{
	bool isvalid = 0;
	bool isdirectory = 0;
	bool hasnext = 1;
	char filename[256] = { 0 };
};

int makedirectoryinfo()
{
	string strpath = csocket::getsocket()->getfilepath();

	if (_chdir(strpath.c_str()) != 0)
	{

		return -2;
	}
	_finddata_t fdata;
	int hfind = _findfirst("*", &fdata);
	if (hfind == -1)
	{
		return -3;
	}

	do
	{
		FILEINFO finfo;
		finfo.isdirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.filename, fdata.name, strlen(fdata.name));
		cpacket pack(2, (uchar*)&finfo, sizeof(finfo));
		csocket::getsocket()->sendate(pack);
	} while (!_findnext(hfind, &fdata));

	FILEINFO finfo;
	finfo.hasnext = 0;
	cpacket pack(2, (uchar*)&finfo, sizeof(finfo));
	csocket::getsocket()->sendate(pack);

	return 0;
}

int runfile()
{
	string strpath = pserver->getfilepath();
	ShellExecuteA(NULL, NULL, strpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
	pserver->sendate(cpacket(3, 0, 0));
	return 0;
}

int downloadfile()
{
	string strpath = pserver->getfilepath();
	FILE* pfile = fopen(strpath.c_str(), "rb");
	ll dlen = 0;
	if (pfile == 0)
	{
		pserver->sendate(cpacket(4, 0, 0));
		return -1;
	}
	fseek(pfile, 0, SEEK_END);
	dlen = _ftelli64(pfile);
	pserver->sendate(cpacket(4, (uchar*)&dlen, 8));
	fseek(pfile, 0, SEEK_SET);
	char buf[1024] = "";
	while (1)
	{
		int len = fread(buf, 1, 1024, pfile);
		pserver->sendate(cpacket(4, (uchar*)buf, len));
		if (len < 1024) break;
	}

	pserver->sendate(cpacket(4, 0, 0));
	fclose(pfile);
}

// 0 左键 1 右键 2 中间 3 移动
// 高4位 0 单击 1 双击 2 按下 3 放开

void mouseld()
{
	mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, GetMessageExtraInfo());
}
void mouselu()
{
	mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, GetMessageExtraInfo());
}

void mouserd()
{
	mouse_event(MOUSEEVENTF_RIGHTDOWN, 0, 0, 0, GetMessageExtraInfo());
}
void mouseru()
{
	mouse_event(MOUSEEVENTF_RIGHTUP, 0, 0, 0, GetMessageExtraInfo());
}

void mousemd()
{
	mouse_event(MOUSEEVENTF_MIDDLEDOWN, 0, 0, 0, GetMessageExtraInfo());
}
void mousemu()
{
	mouse_event(MOUSEEVENTF_MIDDLEUP, 0, 0, 0, GetMessageExtraInfo());
}


void mousevcheck(int flag, int x, int y)
{
	if (flag != 8) 	SetCursorPos(x, y);
	if (flag == 0x11) mouseld(), mouselu();
	if (flag == 0x21) mouseld(), mouselu(), mouseld(), mouselu();
	if (flag == 0x41) mouseld();
	if (flag == 0x81) mouselu();
	if (flag == 0x12) mouserd(), mouseru();
	if (flag == 0x22) mouserd(), mouseru(), mouserd(), mouseru();
	if (flag == 0x42) mouserd();
	if (flag == 0x82) mouseru();
	if (flag == 0x14) mousemd(), mousemu();
	if (flag == 0x24) mousemd(), mousemu(), mousemd(), mousemu();
	if (flag == 0x44) mousemd();
	if (flag == 0x84) mousemu();
	if (flag == 0x08) mouse_event(MOUSEEVENTF_MOVE, x, y, 0, GetMessageExtraInfo());
}

int mousevent()
{
	MOUSEV ev = pserver->getmousevent();
	int flag = (1 << ev.sbtn) | (1 << ev.sact << 4);
	mousevcheck(flag, ev.pt.x, ev.pt.y);
	pserver->sendate(cpacket(4, 0, 0));
	
	return 0;
}

int sendscreen()
{
	CImage screen;
	HDC hscreen = ::GetDC(NULL);
	int nbitpixel = GetDeviceCaps(hscreen, BITSPIXEL);
	int nwidth = GetDeviceCaps(hscreen, HORZRES);
	int nheight = GetDeviceCaps(hscreen, VERTRES);
	screen.Create(nwidth, nheight, nbitpixel);
	BitBlt(screen.GetDC(), 0, 0, 1920, 1080, hscreen, 0, 0, SRCCOPY);
	ReleaseDC(NULL, hscreen);
	HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, 0);
	IStream* pstream = NULL;
	HRESULT ret = CreateStreamOnHGlobal(hmem, true, &pstream);

	screen.Save(pstream, Gdiplus::ImageFormatJPEG);
	LARGE_INTEGER bg = { 0 };
	pstream->Seek(bg, STREAM_SEEK_SET, NULL);
	uchar* pdata = (uchar*)GlobalLock(hmem);
	uint nsize = GlobalSize(hmem);
	pserver->sendate(cpacket(6, pdata, nsize));
	GlobalUnlock(hmem);
	pstream->Release();
	GlobalFree(hmem);
	//screen.Save(_T("test2024.jpg"), Gdiplus::ImageFormatJPEG);
	screen.ReleaseDC();

	return 0;
}


int main()
{
	int nRetCode = 0;

	HMODULE hModule = ::GetModuleHandle(nullptr);

	if (hModule != nullptr)
	{
		// 初始化 MFC 并在失败时显示错误
		if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
		{
			// TODO: 在此处为应用程序的行为编写代码。
			wprintf(L"错误: MFC 初始化失败\n");
			nRetCode = 1;
		}
		else
		{
			/*int cnt = 0;
			csocket* pserver = csocket::getsocket();
			if (pserver->init() == 0)
			{
				MessageBox(0, _T("网络初始化异常，未能成功初始化，请检查网络设备"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
				exit(0);
			}
			while (csocket::getsocket())
			{
				if (pserver)
				{
					if (pserver->acceptclient() == 0)
					{
						if (cnt > 3)
						{
							MessageBox(0, _T("多次无法正常接入用户，程序结束"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
							exit(0);
						}
						MessageBox(0, _T("无法正常接入用户，自动重试"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
						cnt++;
					}
				}

				
			}*/
			//makedriverinfo(); // 查看磁盘分区
			//makedirectoryinfo(); // 查看指定目录下的文件
			//runfile(); // 打开文件
			//downloadfile(); // 下载文件
			//mousevent(); // 鼠标事件
			sendscreen();
		}
	}
	else
	{
		// TODO: 更改错误代码以符合需要
		wprintf(L"错误: GetModuleHandle 失败\n");
		nRetCode = 1;
	}



	while (1)
	{

	}
	return nRetCode;
}

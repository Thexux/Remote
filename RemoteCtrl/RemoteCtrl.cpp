// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include "pch.h"
#include "framework.h"
#include "RemoteCtrl.h"
#include "serversocket.h"
#include <direct.h>
#include <io.h>
#include <atlimage.h>
#include "lockdialog.h"

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
}

int makedriverinfo()
{
	string res;
	for (int i = 1; i <= 26; i++)
	{
		if (_chdrive(i) == 0) res += 'A' + i - 1, res += ',';
	}

	cpacket pack = cpacket(1, (uchar*)res.c_str(), res.size());

	dump((uchar*)pack.data(), pack.size());
	csocket::getsocket()->sendate(pack);
	return 0;
}

int makedirectoryinfo()
{
	string strpath = csocket::getsocket()->getfilepath();

	if (_chdir(strpath.c_str()) != 0) //将当前工作目录更改为strpath
	{
		return -2;
	}
	_finddata_t fdata;
	int hfind = _findfirst("*", &fdata); //获取符合指定条件的文件
	if (hfind == -1)
	{
		return -3;
	}
	int cnt = 5;
	do
	{
		FILEINFO finfo;
		finfo.isdirectory = (fdata.attrib & _A_SUBDIR) != 0;
		memcpy(finfo.filename, fdata.name, strlen(fdata.name));
		csocket::getsocket()->sendate(cpacket(2, (uchar*)&finfo, sizeof finfo));
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
	cout << strpath << endl;
	if (pfile == 0)
	{
		pserver->sendate(cpacket(4, 0, 0));
		return -1;
	}
	/*ll dlen = 0;
	fseek(pfile, 0, SEEK_END);
	dlen = _ftelli64(pfile);
	pserver->sendate(cpacket(4, (uchar*)&dlen, 8));
	fseek(pfile, 0, SEEK_SET);*/
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

clockdialog dlg;
uint threadid;

unsigned __stdcall threadlockdialog(void* arg)
{
	dlg.Create(IDD_DIALOG_INFO, NULL);
	dlg.ShowWindow(SW_SHOW);
	//遮蔽后台窗口
	CRect rect;
	rect.left = 0, rect.top = 0;
	//rect.right = GetSystemMetrics(SM_CXFULLSCREEN);
	//rect.bottom = GetSystemMetrics(SM_CYFULLSCREEN) * 1.1;
	//dlg.MoveWindow(rect);
	// 窗口置顶
	dlg.SetWindowPos(&dlg.wndTopMost, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	// 限制鼠标功能和移动范围
	ShowCursor(0);
	rect.right = rect.left + 1;
	rect.bottom = rect.top + 1;
	ClipCursor(rect);
	//隐藏任务栏
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_HIDE);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_KEYDOWN)
		{
			cout << msg.message << ' ' << msg.wParam << ' ' << msg.lParam << endl;
			printf("0X%X 0X%X\n", msg.wParam, msg.lParam);

			if (msg.wParam == 0X1B) break;
		}
	}
	ShowCursor(1);
	::ShowWindow(::FindWindow(_T("Shell_TrayWnd"), NULL), SW_SHOW);
	dlg.DestroyWindow();
	_endthreadex(0);
	return 0;
}

int lockmachine()
{
	if (dlg.m_hWnd == 0 || dlg.m_hWnd == INVALID_HANDLE_VALUE) _beginthreadex(0, 0, threadlockdialog, 0, 0, &threadid);

	pserver->sendate(cpacket(7, 0, 0));
	return 0;
}

int unlockmachine()
{
	PostThreadMessage(threadid, WM_KEYDOWN, 0X1B, 0X10001);
	pserver->sendate(cpacket(8, 0, 0));
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

			int cnt = 0;
			csocket* pserver = csocket::getsocket();
			if (pserver->init() == 0)
			{
				MessageBox(0, _T("网络初始化异常，未能成功初始化，请检查网络设备"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
				exit(0);
			}
			while (csocket::getsocket())
			{
				cout << "============" << endl;
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
				int res = pserver->dealcommand();
				cout << "====" << res << endl;
				if (res == 1) makedriverinfo();
				if (res == 2) makedirectoryinfo();
				if (res == 4) downloadfile();
 				//pserver->sendate(cpacket(res, 0, 0));
				pserver->closeclient();
			}

			

			//makedriverinfo(); // 查看磁盘分区
			//makedirectoryinfo(); // 查看指定目录下的文件
			//runfile(); // 打开文件
			//downloadfile(); // 下载文件
			//mousevent(); // 鼠标事件
			//sendscreen(); // 发送屏幕截图
			//lockmachine(); // 锁机
			//unlockmachine(); // 解锁
			//Sleep(1000);
			//unlockmachine();
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
		//cout << dlg.m_hWnd << endl;
		//Sleep(1000);
	}
	return nRetCode;
}

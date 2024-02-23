#pragma once
#include "resource.h"
#include "packet.h"
#include <direct.h>
#include <io.h>
#include <atlimage.h>
#include "lockdialog.h"
#include "tool.h"

class ccommand
{
public:
	ccommand();
	int execommand(int ncmd, list<cpacket>&, cpacket&);
	static void runcommand(void* arg, int status, list<cpacket>& lstpacket, cpacket& inpacket)
	{
		ccommand* pcmd = (ccommand*)arg;
		if (pcmd->execommand(status, lstpacket, inpacket)) cout << status << ' ' << "error" << endl; // TODO：处理错误
	}

protected:
	typedef int(ccommand::*cmdfunc)(list<cpacket>&, cpacket&);
	map<int, cmdfunc> m_mpfunction;
	clockdialog dlg;
	uint threadid;

	int makedriverinfo(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		string res;
		for (int i = 1; i <= 26; i++)
		{
			if (_chdrive(i) == 0) res += 'A' + i - 1, res += ',';
		}

		lstpacket.push_back(cpacket(1, (uchar*)res.c_str(), res.size()));
		return 0;
	}

	int makedirectoryinfo(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		string strpath = inpacket.strbuf;

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
		do
		{
			FILEINFO finfo;
			finfo.isdirectory = (fdata.attrib & _A_SUBDIR) != 0;
			memcpy(finfo.filename, fdata.name, strlen(fdata.name));
			lstpacket.push_back(cpacket(2, (uchar*)&finfo, sizeof finfo));
		} while (!_findnext(hfind, &fdata));

		FILEINFO finfo;
		finfo.hasnext = 0;
		lstpacket.push_back(cpacket(2, (uchar*)&finfo, sizeof(finfo)));

		return 0;
	}

	int runfile(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		string strpath = inpacket.strbuf;

		ShellExecuteA(NULL, NULL, strpath.c_str(), NULL, NULL, SW_SHOWNORMAL);
		lstpacket.push_back(cpacket(3, 0, 0));

		return 0;
	}

	int downloadfile(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		string strpath = inpacket.strbuf;
		FILE* pfile = fopen(strpath.c_str(), "rb");

		if (pfile == 0)
		{
			lstpacket.push_back(cpacket(4, 0, 0));
			return -1;
		}

		ll dlen = 0;
		fseek(pfile, 0, SEEK_END);
		dlen = _ftelli64(pfile);
		lstpacket.push_back(cpacket(4, (uchar*)&dlen, 8));
		fseek(pfile, 0, SEEK_SET);

		char buf[1024] = "";
		while (dlen)
		{
			int len = fread(buf, 1, 1024, pfile);
			lstpacket.push_back(cpacket(4, (uchar*)buf, len));
			if (len < 1024) break;
		}
		lstpacket.push_back(cpacket(4, 0, 0));
		fclose(pfile);
		return 0;
	}

	int deletelocalfile(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		string strpath = inpacket.strbuf;
		TCHAR path[MAX_PATH] = _T("");
		mbstowcs(path, strpath.c_str(), strpath.size());
		DeleteFileA(strpath.c_str());
		lstpacket.push_back(cpacket(5, 0, 0));
		return 0;
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

	int mousevent(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		MOUSEV ev = *(MOUSEV*)inpacket.strbuf.c_str();
		int flag = (1 << ev.sbtn) | (1 << ev.sact << 4);
		mousevcheck(flag, ev.pt.x, ev.pt.y);
		lstpacket.push_back(cpacket(7, 0, 0));

		return 0;
	}

	int sendscreen(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		CImage screen;
		HDC hscreen = ::GetDC(NULL);
		int nbitpixel = GetDeviceCaps(hscreen, BITSPIXEL);
		int nwidth = GetDeviceCaps(hscreen, HORZRES);
		int nheight = GetDeviceCaps(hscreen, VERTRES);
		screen.Create(nwidth, nheight, nbitpixel);
		BitBlt(screen.GetDC(), 0, 0, nwidth, nheight, hscreen, 0, 0, SRCCOPY);
		ReleaseDC(NULL, hscreen);
		HGLOBAL hmem = GlobalAlloc(GMEM_MOVEABLE, 0);
		IStream* pstream = NULL;
		HRESULT ret = CreateStreamOnHGlobal(hmem, true, &pstream);

		screen.Save(pstream, Gdiplus::ImageFormatJPEG);
		//screen.Save(pstream, Gdiplus::ImageFormatPNG);
		//screen.Save(_T("a.jpeg"), Gdiplus::ImageFormatJPEG);
		LARGE_INTEGER bg = { 0 };
		pstream->Seek(bg, STREAM_SEEK_SET, NULL);
		uchar* pdata = (uchar*)GlobalLock(hmem);
		uint nsize = GlobalSize(hmem);
		lstpacket.push_back(cpacket(6, pdata, nsize));
		GlobalUnlock(hmem);
		pstream->Release();
		GlobalFree(hmem);
		//screen.Save(_T("test2024.jpg"), Gdiplus::ImageFormatJPEG);
		screen.ReleaseDC();

		return 0;
	}

	static unsigned __stdcall threadlockdialog(void* arg)
	{
		ccommand* pcmd = (ccommand*)arg;
		pcmd->threadlockdlgmain();
		_endthreadex(0);
		return 0;
	}

	void threadlockdlgmain()
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
	}



	int lockmachine(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		if (dlg.m_hWnd == 0 || dlg.m_hWnd == INVALID_HANDLE_VALUE)
			_beginthreadex(0, 0, &ccommand::threadlockdialog, this, 0, &threadid);

		lstpacket.push_back(cpacket(7, 0, 0));
		return 0;
	}

	int unlockmachine(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		PostThreadMessage(threadid, WM_KEYDOWN, 0X1B, 0X10001);
		lstpacket.push_back(cpacket(8, 0, 0));
		return 0;
	}
	
	int testconnect(list<cpacket>& lstpacket, cpacket& inpacket)
	{
		lstpacket.push_back(cpacket(505, 0, 0));
		return 0;
	}

};


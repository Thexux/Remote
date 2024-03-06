// RemoteCtrl.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//
#include "pch.h"
#include "framework.h"
#include "serversocket.h"
#include "command.h"
#include "cpqueue.h"
#include "iocpserver.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

CWinApp theApp;
using namespace std;

//void chooseinvoke()
//{
//	CString strsubkey = _T("HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Run");
//	CString strtinfo =_T("该机器会被监控");
//	int res = MessageBox(NULL, strtinfo, _T("警告"), MB_YESNOCANCEL | MB_ICONWARNING | MB_TOPMOST);
//	if (res == IDOK)
//	{
//		char spath[MAX_PATH] = "";
//		char sys[MAX_PATH] = "";
//		string strexe = "\\RemoteCtrl.exe";
//		GetCurrentDirectoryA(MAX_PATH, spath);
//		
//		string strcmd = "mklink" +string(sys) + strexe + string(spath) + strexe;
//		
//		system(strcmd.c_str());
//		HKEY hkey = 0;
//		res = RegOpenKeyEx(HKEY_LOCAL_MACHINE, strsubkey, 0, KEY_WRITE, &hkey);
//		if (res != ERROR_SUCCESS)
//		{
//			RegCloseKey(hkey);
//			MessageBox(NULL, _T("设置自动开机启动失败！是否权限不足？\r\n程序启动失败"),
//				_T("错误"), MB_ICONERROR | MB_TOPMOST);
//			exit(0);
//		}
//		RegSetValueEx(hkey, _T("RemoteCtrl"), 0, REG_SZ, _T(""), );
//	}
//	return;
//}

void test()
{
	cpqueue<string> lststr;
	ULONGLONG tick = GetTickCount64();
	int cnt = 0, cnt0 = 0;
	while (GetTickCount64() - tick <= 1000) lststr.pushback("hello world");
	printf("push done size is %d\r\n", lststr.size());

	ULONGLONG tick0 = GetTickCount64();
	while (GetTickCount64() - tick <= 1000)
	{
		string st;
		lststr.popfront(st);
	}
	printf("pop done size is %d\r\n", lststr.size());

	//while ()
	//{
	//	//if (GetTickCount64() - tick0 > 13)
	//	{
	//		lststr.pushback("hello world"); cnt++;
	//		//tick0 = GetTickCount64();
	//	}
	//	//if (GetTickCount64() - tick > 20)
	//	{
	//		string st;
	//		lststr.popfront(st);
	//		//tick = GetTickCount64();
	//		cout << st << ' ' << ++cnt0 << endl;
	//	}
	//	Sleep(1);
	//}
	//printf("exit done size is %d\r\n", lststr.size());
	//lststr.clear();
	//printf("exit done size is %d\r\n", lststr.size());
	//cout << cnt << ' ' << cnt0 << endl;
}

bool Init()
{
	HMODULE hModule = ::GetModuleHandle(nullptr);
	if (hModule == nullptr)
	{
		wprintf(L"错误: GetModuleHandle 失败\n");
		return false;
	}
	if (!AfxWinInit(hModule, nullptr, ::GetCommandLine(), 0))
	{
		wprintf(L"错误: MFC 初始化失败\n");
		return false;
	}
	return true;
}

int main()
{
	if (!Init()) return 1;

	ccommand cmd;
	int res = csocket::getsocket()->run(&ccommand::runcommand, &cmd);
	if (res == -1) MessageBox(0, _T("网络初始化异常，未能成功初始化，请检查网络设备"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
	if (res == -2) MessageBox(0, _T("多次无法正常接入用户，程序结束"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
	
	return 0;
}

void iocp()
{
	ciocpserver server;
	server.startservice();
	getchar();
}
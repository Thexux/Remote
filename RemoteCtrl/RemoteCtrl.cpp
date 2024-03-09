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

void iocp()
{
	ciocpserver server;
	server.startservice();
	getchar();
}

int main()
{
	if (!Init()) return 1;

	//iocp();

	ccommand cmd;
	int res = csocket::getsocket()->run(&ccommand::runcommand, &cmd);
	if (res == -1) MessageBox(0, _T("网络初始化异常，未能成功初始化，请检查网络设备"), _T("网络初始化失败"), MB_OK | MB_ICONERROR);
	if (res == -2) MessageBox(0, _T("多次无法正常接入用户，程序结束"), _T("接入用户失败"), MB_OK | MB_ICONERROR);
	
	return 0;
}

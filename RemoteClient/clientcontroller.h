#pragma once
#include "resource.h"
#include "clientsocket.h"
#include "watchdlg.h"
#include "RemoteClientDlg.h"
#include "statusdlg.h"
#include "clientsocket.h"

#define WM_SNED_PACK (WM_USER + 1) // 发送包数据
#define WM_SNED_DATA (WM_USER + 2) // 发送数据
#define WM_SNED_STATUS (WM_USER + 3) // 展示状态
#define WM_SNED_WATCH (WM_USER + 4) // 远程监控
#define WM_SNED_MESSAGE (WM_USER + 0x1000) // 自定义消息处理

class cclientcontroller
{
public:
	
	static cclientcontroller* getinstance(); // 获取全局唯一对象
	int init(); // 初始化操作
	int invoke(CWnd*& pMainWnd); // 启动
	LRESULT SendMessage(MSG nmsg); // v层桥接
	void updateaddr(int nip, int nport); //更新服务器地址
	int dealcommand();
	void closesock();
	int sendcommandpacket(int ncmd,
		uchar* pdata = NULL,
		int nlen = 0,
		list<cpacket>*lstpack = NULL);
	int getimage(CImage& image);
	int downflie(string strpath);
	void startwatchscreen();

protected:
	cclientcontroller();
	~cclientcontroller();
	void threadfunc();
	static unsigned __stdcall threadentry(void*);
	static void releaseinstance();

	/*LRESULT onsendpack(UINT nmsg, WPARAM wparam, LPARAM lparam);
	LRESULT onsenddata(UINT nmsg, WPARAM wparam, LPARAM lparam);*/
	LRESULT onshowstatus(UINT nmsg, WPARAM wparam, LPARAM lparam);
	LRESULT onshowwatcher(UINT nmsg, WPARAM wparam, LPARAM lparam);

	void threaddownloadfile();
	static void threaddownloadentry(void* arg);

	void threadwatchscreen();
	static void threadwatchentry(void* arg);

private:
	struct MSGINFO
	{
		MSG msg = { 0 };
		LRESULT res = 0;
		MSGINFO(MSG m)
		{
			res = 0;
			memcpy(&msg, &m, sizeof(MSG));
			CreateEvent(0, 1, 0, 0);
		}
		MSGINFO(const MSGINFO& m)
		{
			res = m.res;
			memcpy(&msg, &m.msg, sizeof(MSG));
		}
		MSGINFO& operator=(const MSGINFO& m)
		{
			if (this != &m)
				res = m.res, memcpy(&msg, &m.msg, sizeof(MSG));
			return *this;
		}
	};
	class chelper
	{
	public:
		chelper()
		{
			//getinstance();
		}
		~chelper()
		{
			releaseinstance();
		}
	};
	static chelper m_helper;

	typedef LRESULT(cclientcontroller::* MSGFUNC)
		(UINT nmsg, WPARAM wparam, LPARAM lparam);
	static map<UINT, MSGFUNC> m_mpfunc;
	cwatchdlg m_watchdlg;
	CRemoteClientDlg m_remotedig;
	cstatusdlg m_statusdlg;
	HANDLE m_hthread;
	unsigned m_nthreadid;
	static cclientcontroller* m_instance;
	HANDLE m_threaddownload;
	HANDLE m_threadwatch;
	bool m_isclose; // 监控线程状态
	string m_strremote; // 远程文件路径
	string m_strlocal; // 本地文件路径
};
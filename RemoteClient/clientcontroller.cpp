#include "pch.h"
#include "clientcontroller.h"
#include "tool.h"

map<UINT, cclientcontroller::MSGFUNC> cclientcontroller::m_mpfunc;
cclientcontroller* cclientcontroller::m_instance = NULL;
cclientcontroller::chelper cclientcontroller::m_helper;

cclientcontroller* cclientcontroller::getinstance()
{
	if (m_instance == 0)
	{
		m_instance = new cclientcontroller();
		struct
		{
			UINT nmsg;
			MSGFUNC func;
		} msga[] =
		{
			{WM_SNED_STATUS, &onshowstatus},
			{WM_SNED_WATCH, &onshowwatcher},
			{(UINT)-1, 0}
		};

		for (int i = 0; msga[i].nmsg != -1; i++)
		{
			m_mpfunc[msga[i].nmsg] = msga[i].func;
		}
	}
	return m_instance;
}

cclientcontroller::cclientcontroller():
	m_statusdlg(&m_remotedig),
	m_watchdlg(&m_remotedig)
{
	m_isclose = 1;
	m_threadwatch = INVALID_HANDLE_VALUE;
	m_hthread = INVALID_HANDLE_VALUE;
	m_nthreadid = -1;
}


int cclientcontroller::init()
{
	m_hthread = (HANDLE)_beginthreadex(0, 0, threadentry,
		this, 0, &m_nthreadid);
	m_statusdlg.Create(IDD_DLG_STATUS, &m_remotedig);
	return 0;
}

int cclientcontroller::invoke(CWnd*& pMainWnd)
{
	pMainWnd = &m_remotedig;
	return m_remotedig.DoModal();
}

void cclientcontroller::updateaddr(int nip, int nport)
{
	csocket::getsocket()->updateaddr(nip, nport);
}

bool cclientcontroller::sendcommandpacket(HWND hwnd, 
	int ncmd,uchar* pdata, int nlen, WPARAM wparam)
{
	csocket* pclient = csocket::getsocket();
	return pclient->sendpacket(hwnd, cpacket(ncmd, pdata, nlen), wparam);
}

void cclientcontroller::startwatchscreen()
{
	m_isclose = 0;
	HANDLE hthread = (HANDLE)_beginthread(cclientcontroller::threadwatchentry, 0, this);
	m_watchdlg.DoModal();
	m_isclose = 1;
	WaitForSingleObject(hthread, 500);
}

void cclientcontroller::threadwatchscreen()
{
	Sleep(50);
	ULONGLONG ntick = GetTickCount64();
	while (m_isclose == 0)
	{
		if (m_watchdlg.isfull() == 0) // 更新数据到缓存
		{
			if (GetTickCount64() - ntick < 50) Sleep(50 - GetTickCount64() + ntick);
			
			if (sendcommandpacket(m_watchdlg.GetSafeHwnd(), 6, 0, 0) == 0)
			{
				Sleep(1);
				continue;
			}
			m_watchdlg.setimagestatus(1);
			ntick = GetTickCount64();
		}
		else Sleep(1);
	}
}

void cclientcontroller::threadwatchentry(void* arg)
{
	cclientcontroller* pctl = (cclientcontroller*)arg;
	pctl->threadwatchscreen();
	_endthread();
}

int cclientcontroller::downflie(string strpath)
{

	CFileDialog dlg(FALSE, NULL, strpath.c_str(), // TODO: 保存文件名处理
		OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, 0, &m_remotedig);

	if (dlg.DoModal() != IDOK) return -1; // TODO: 返回值处理

	m_strremote = strpath;
	m_strlocal = dlg.GetPathName();
	FILE* pfile = fopen(m_strlocal.c_str(), "wb+");
	if (pfile == 0)
	{
		AfxMessageBox("本地没有权限保存该文件，或这文件无法创建！！！");
		return -1;
	}

	bool ok = sendcommandpacket(m_remotedig, 4, (uchar*)m_strremote.c_str(), m_strremote.size(), (WPARAM)pfile);

	m_remotedig.BeginWaitCursor();
	m_statusdlg.m_info.SetWindowText(_T("命令正在执行中"));
	m_statusdlg.ShowWindow(SW_SHOW);
	m_statusdlg.CenterWindow(&m_remotedig);
	m_statusdlg.SetActiveWindow();

	return 0;
}
	
void cclientcontroller::threadfunc()
{
	MSG msg;
	while (::GetMessage(&msg, 0, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (msg.message == WM_SNED_MESSAGE)
		{
			MSGINFO* pmsg = (MSGINFO*)msg.wParam;
			HANDLE* hevent = (HANDLE*)msg.lParam;
			if (m_mpfunc.find(pmsg->msg.message) != m_mpfunc.end())
				pmsg->res = (this->*m_mpfunc[pmsg->msg.message])
				(pmsg->msg.message, pmsg->msg.wParam, pmsg->msg.lParam);
			else pmsg->res = -1;
			SetEvent(hevent);
		}
		else
		{
			if (m_mpfunc.find(msg.message) == m_mpfunc.end()) continue;
			(this->*m_mpfunc[msg.message])(msg.message, msg.wParam, msg.lParam);
		}
	}
}

unsigned __stdcall cclientcontroller::threadentry(void* arg)
{
	cclientcontroller* pthis = (cclientcontroller*)arg;
	pthis->threadfunc();
	_endthreadex(0);
	return 0;
}

void cclientcontroller::releaseinstance()
{
	if (m_instance) delete m_instance, m_instance = 0;
}

LRESULT cclientcontroller::onshowstatus(UINT nmsg, WPARAM wparam, LPARAM lparam)
{
	return m_statusdlg.ShowWindow(SW_SHOW);
}

LRESULT cclientcontroller::onshowwatcher(UINT nmsg, WPARAM wparam, LPARAM lparam)
{
	return m_watchdlg.DoModal();
}

cclientcontroller::~cclientcontroller()
{
	WaitForSingleObject(m_hthread, 100);
}
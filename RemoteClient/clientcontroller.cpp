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
		/*	{WM_SNED_PACK, &onsendpack},
			{WM_SNED_DATA, &onsenddata},*/
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
	m_threaddownload = INVALID_HANDLE_VALUE;
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

LRESULT cclientcontroller::SendMessage(MSG nmsg)
{
	HANDLE hevent = CreateEvent(0, 1, 0, 0);
	if (hevent == 0) return -2;
	MSGINFO info(nmsg);
	PostThreadMessage(m_nthreadid, WM_SNED_MESSAGE, 
		(WPARAM)&info, (LPARAM)&hevent);
	WaitForSingleObject(hevent, -1);
	return info.res;
}

void cclientcontroller::updateaddr(int nip, int nport)
{
	csocket::getsocket()->updateaddr(nip, nport);
}

int cclientcontroller::dealcommand()
{
	return csocket::getsocket()->dealcommand();
}

void cclientcontroller::closesock()
{
	csocket::getsocket()->closesock();
}

bool cclientcontroller::sendcommandpacket(HWND hwnd, 
	int ncmd,uchar* pdata, int nlen, WPARAM wparam)
{
	csocket* pclient = csocket::getsocket();
	return pclient->sendpacket(hwnd, cpacket(ncmd, pdata, nlen), wparam);
}

int cclientcontroller::getimage(CImage& image)
{
	csocket* pclient = csocket::getsocket();
	return ctool::btoimage(image, pclient->getpacket().strbuf);
}

void cclientcontroller::startwatchscreen()
{
	m_isclose = 0;
	//m_watchdlg.SetParent(&m_remotedig); //cwatchdlg dlg(&m_remotedig);
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

void cclientcontroller::threaddownloadfile()
{
	FILE* pfile = fopen(m_strlocal.c_str(), "wb+");
	if (pfile == 0)
	{
		AfxMessageBox("本地没有权限保存该文件，或这文件无法创建！！！");
		m_statusdlg.ShowWindow(SW_HIDE);
		m_remotedig.EndWaitCursor();
		return;
	}

	int res = sendcommandpacket(m_statusdlg, 4, (uchar*)m_strremote.c_str(), m_strremote.size());
	if (res != 4)
	{
		AfxMessageBox("执行下载命令失败！！"), fclose(pfile), closesock();
		return;
	}

	csocket* pclient = csocket::getsocket();
	ll llen = *(ll*)pclient->getpacket().strbuf.c_str();

	while (1)
	{
		us cmd = pclient->dealcommand();
		if (cmd != 4)
		{
			AfxMessageBox("传输失败！！");
			break;
		}
		if (pclient->getpacket().strbuf.size() == 0) break;
		fwrite(pclient->getpacket().strbuf.c_str(), 1, pclient->getpacket().strbuf.size(), pfile);
	}

	fclose(pfile);
	pclient->closesock();
	m_statusdlg.ShowWindow(SW_HIDE);
	m_remotedig.EndWaitCursor();
	m_remotedig.MessageBox(_T("下载完成！！"), _T("完成"));
}

void cclientcontroller::threaddownloadentry(void* arg)
{
	cclientcontroller* pctl = (cclientcontroller*)arg;
	pctl->threaddownloadfile();
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


	//m_threaddownload = (HANDLE)_beginthread(&cclientcontroller::threaddownloadentry, 0, this);
	//if (WaitForSingleObject(m_threaddownload, 0) != WAIT_TIMEOUT) return -2; // 返回值处理

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

//LRESULT cclientcontroller::onsendpack(UINT nmsg, WPARAM wparam, LPARAM lparam)
//{
//	csocket* pclient = csocket::getsocket();
//	return pclient->sendate(*(cpacket*)wparam);
//}
//
//LRESULT cclientcontroller::onsenddata(UINT nmsg, WPARAM wparam, LPARAM lparam)
//{
//	csocket* pclient = csocket::getsocket();
//	return pclient->sendate((char*)wparam, (int)lparam);
//}

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
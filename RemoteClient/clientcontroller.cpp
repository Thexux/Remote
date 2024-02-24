#include "pch.h"
#include "clientcontroller.h"

map<UINT, cclientcontroller::MSGFUNC> cclientcontroller::m_mpfunc;
cclientcontroller* cclientcontroller::m_instance = NULL;

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
			{WM_SNED_PACK, &onsendpack},
			{WM_SNED_DATA, &onsenddata},
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

LRESULT cclientcontroller::onsendpack(UINT nmsg, WPARAM wparam, LPARAM lparam)
{
	return LRESULT();
}

LRESULT cclientcontroller::onsenddata(UINT nmsg, WPARAM wparam, LPARAM lparam)
{
	return LRESULT();
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
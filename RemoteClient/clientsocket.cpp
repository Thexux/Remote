#include "pch.h"
#include "clientsocket.h"
#include "common.h"

csocket* csocket::m_csock = 0;
//csocket* pclient = csocket::getsocket();
csocket::chelper csocket::m_help;

csocket* csocket::getsocket()
{
	if (m_csock == 0) m_csock = new csocket();
	return m_csock;
}

unsigned csocket::threadentry(void* arg)
{
	csocket* psock = (csocket*)arg;
	psock->threadfunc();
	_endthreadex(0);
	return 0;
}

void csocket::threadfunc()
{
	SetEvent(m_eventinvoke);
	MSG msg;
	while (::GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		if (m_mpfun.find(msg.message) == m_mpfun.end()) continue; //TODO:
		(this->*m_mpfun[msg.message])(msg.message, msg.wParam, msg.lParam);
	}
}

void csocket::sendpackmsg(UINT nmsg, WPARAM wparam, LPARAM lparam)
{
	packdata strpack = *(packdata*)wparam;
	delete (packdata*)wparam;
	HWND hwnd = (HWND)lparam;
	if (init())
	{
		int res = send(m_sock, (char*)strpack.strdata.c_str(), (int)strpack.strdata.size(), 0);
		if (res > 0)
		{
			char* buf = vbuf.data();
			while (1)
			{
				int len = nbufidx;
				cpacket pack((uchar*)buf, len);
				if (len)
				{
					memmove(buf, buf + len, BUF_SIZE - len);
					nbufidx -= len;
					::SendMessage(hwnd, WM_SEND_PACK_ACK, (WPARAM)new cpacket(pack), strpack.extend);
					continue;
				}

				len = recv(m_sock, buf + nbufidx, BUF_SIZE - nbufidx, 0);
				if (len <= 0)
				{
					closesock();
					::SendMessage(hwnd, WM_SEND_PACK_ACK, 0, 1);
					return;
				}
				nbufidx += len;
			}
		}
		else
		{
			closesock();
			::SendMessage(hwnd, WM_SEND_PACK_ACK, NULL, -1);
		}
	}
	else ::SendMessage(hwnd, WM_SEND_PACK_ACK, NULL, -2);
}


csocket::csocket()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data))
	{
		MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	vbuf.resize(BUF_SIZE);
	nbufidx = 0;
	m_nip = INADDR_ANY;
	m_nport = 0;
	m_sock = -1;
	m_hthread = INVALID_HANDLE_VALUE;

	m_eventinvoke = CreateEvent(NULL, TRUE, FALSE, NULL);
	m_hthread = (HANDLE)_beginthreadex(NULL, 0, threadentry, this, 0, &m_nthreadid);
	if (WaitForSingleObject(m_eventinvoke, 100) == WAIT_TIMEOUT)
		TRACE("网络消息处理线程启动失败！\r\n");
	CloseHandle(m_eventinvoke);

	struct
	{
		UINT message;
		MSGFUNC func;
	} funcs[] =
	{
		{WM_SEND_PACK, &csocket::sendpackmsg},
		{0, 0}
	};
	for (int i = 0; funcs[i].message; i++)
	{
		m_mpfun[funcs[i].message] = funcs[i].func;
	}

}

csocket::csocket(const csocket& cs)
{
	m_nip = cs.m_nip;
	m_nport = cs.m_nport;
	m_sock = cs.m_sock;
	m_mpfun = cs.m_mpfun;
}

csocket::~csocket()
{
	closesocket(m_sock);
	WSACleanup();
}

bool csocket::init()
{
	if (m_sock != -1) closesock();
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == -1) return 0;
	//cout << "m_sock:" << m_sock << endl;
	sockaddr_in sev_addr;
	memset(&sev_addr, 0, sizeof sev_addr);
	sev_addr.sin_family = AF_INET;
	sev_addr.sin_addr.s_addr = htonl(m_nip);
	sev_addr.sin_port = htons(m_nport);
	if (sev_addr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("指定IP地址，不存在");
		return 0;
	}

	int res = connect(m_sock, (sockaddr*)&sev_addr, sizeof sev_addr);
	if (res == -1)
	{
		AfxMessageBox("连接失败");
		TRACE("连接失败，%d\r\n", WSAGetLastError());
		return 0;
	}
	return 1;
}

void csocket::updateaddr(int nip, int nport)
{
	m_nip = nip, m_nport = nport;
}

void csocket::closesock()
{
	closesocket(m_sock);
	m_sock = -1;
}

bool csocket::sendpacket(HWND hwnd, cpacket pack, WPARAM wparam)
{
	packdata* pdata = new packdata(pack.data(), pack.size(), wparam);
	bool res = PostThreadMessage(m_nthreadid, WM_SEND_PACK, (WPARAM)pdata, (LPARAM)hwnd);
	if (res == 0) delete pdata;
	return res;
}

int csocket::dealcommand()
{
	if (m_sock == -1) return -1;

	char* buf = vbuf.data();
	while (1)
	{
		int len = nbufidx;
		m_packet = cpacket((uchar*)buf, len);
		if (len)
		{
			memmove(buf, buf + len, BUF_SIZE - len);
			nbufidx -= len;
			return m_packet.scmd;
		}

		len = recv(m_sock, buf + nbufidx, BUF_SIZE - nbufidx, 0);
		if (len <= 0) return -1;
		nbufidx += len;
	}
}

bool csocket::sendate(const char* pdata, uint nsize)
{
	if (m_sock == -1) return 0;
	return send(m_sock, pdata, nsize, 0) > 0;

}

bool csocket::sendate(cpacket pack)
{
	if (m_sock == -1) return 0;
	return send(m_sock, pack.data(), pack.size(), 0) > 0;
}

cpacket& csocket::getpacket()
{
	return m_packet;
}

string csocket::getfilepath()
{
	return m_packet.strbuf;
}

MOUSEV csocket::getmousevent()
{
	return *(MOUSEV*)m_packet.strbuf.c_str();
}

void csocket::releasesock()
{
	if (m_csock)
	{
		csocket* t = m_csock;
		delete t;
		m_csock = 0;
	}
}

csocket::chelper::chelper()
{
	csocket::getsocket();
}

csocket::chelper::~chelper()
{
	csocket::releasesock();
}

cpacket::cpacket()
{
}

cpacket::cpacket(const uchar* pdata, int& nsize)
{
	int idx = 0, n = nsize;
	nsize = 0;

	if (idx + 2 >= n) return;
	while (idx < n) if (*(us*)(pdata + idx++) == 0xFEFF) break;

	if (idx + 4 >= n) return;
	nlen = *(int*)&pdata[++idx], idx += 4;
	int nstsize = nlen - 6;

	if (idx + nlen > n) return;

	scmd = *(us*)&pdata[idx], idx += 2;

	int sum = 0;
	strbuf = "";

	for (int i = 0; i < nstsize; i++) sum += pdata[idx + i], strbuf += pdata[idx + i];
	idx += nstsize;

	nsum = *(int*)&pdata[idx], idx += 4;

	if (nsum == sum) nsize = idx;
}


cpacket::cpacket(const cpacket& cp)
{
	shead = cp.shead;
	nlen = cp.nlen;
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	nsum = cp.nsum;
}

cpacket::cpacket(us cmd, const uchar* pdata, int nsize)
{
	shead = 0xFEFF, nlen = nsize + 6, scmd = cmd, nsum = 0, strbuf = "";
	for (int i = 0; i < nsize; i++) nsum += pdata[i], strbuf += pdata[i];
	//cout << "======" << nlen << ' ' << scmd << ' ' << pdata << ' ' << strbuf << ' ' << nsum << endl;
}

cpacket& cpacket::operator=(const cpacket& cp)
{
	shead = cp.shead;
	nlen = cp.nlen;
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	nsum = cp.nsum;
	return *this;
}

int cpacket::size()
{
	return nlen + 6;
}

const char* cpacket::data()
{
	strout.resize(nlen + 6);
	uchar* pdata = (uchar*)strout.c_str();
	*(us*)pdata = shead, pdata += 2;
	*(int*)pdata = nlen, pdata += 4;
	*(us*)pdata = scmd, pdata += 2;
	memcpy(pdata, strbuf.c_str(), strbuf.size()), pdata += strbuf.size();
	*(int*)pdata = nsum;
	return strout.c_str();
}

cpacket::~cpacket()
{
}

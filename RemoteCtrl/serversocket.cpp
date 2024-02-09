#include "pch.h"
#include "serversocket.h"
#include"common.h"

csocket* csocket::m_csock = 0;
//csocket* pserver = csocket::getsocket();
csocket::chelper csocket::m_help;

csocket* csocket::getsocket()
{
	if (m_csock == 0) m_csock = new csocket();
	return m_csock;
}

csocket::csocket()
{
	m_sock = -1;
	m_client = -1;
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data))
	{
		MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
}

csocket::csocket(const csocket& cs)
{

}

csocket::~csocket()
{
	closesocket(m_sock);
	WSACleanup();
}

bool csocket::init()
{
	if (m_sock == -1) return 0;

	sockaddr_in sev_addr;
	memset(&sev_addr, 0, sizeof sev_addr);
	sev_addr.sin_family = AF_INET;
	sev_addr.sin_addr.s_addr = INADDR_ANY;
	sev_addr.sin_port = htons(9527);
	if (bind(m_sock, (sockaddr*)&sev_addr, sizeof sev_addr) == SOCKET_ERROR) return 0;
	if (listen(m_sock, 1) == -1) return 0;

	return 1;
}

bool csocket::acceptclient()
{
	sockaddr_in cli_addr;
	int cli_len = sizeof(cli_addr);
	m_client = accept(m_sock, (sockaddr*)&cli_addr, &cli_len);
	cout << (uint)m_client << ' ' << (m_client != -1) << endl;
	return m_client != -1;
}

const int BUF_SIZE = 4096;
int csocket::dealcommand()
{
	if (m_client == -1) return -1;
	char* buf = new char[BUF_SIZE];
	memset(buf, 0, sizeof buf);
	uint idx = 0;
	while (1)
	{
		int len = recv(m_client, buf + idx, BUF_SIZE - idx, 0);
		if (len <= 0) return -1;
		idx += len; 
		m_packet = cpacket((uchar*)buf, len);
		if (len)
		{
			memmove(buf, buf + len, BUF_SIZE - len);
			idx -= len;
			return m_packet.scmd;
		}
	}
}

bool csocket::sendate(const char* pdata, uint nsize)
{
	if (m_client == -1) return 0;
	return send(m_client, pdata, nsize, 0) > 0;
	
}

void csocket::releasesock()
{
	if (m_csock)
	{
		csocket *t = m_csock;
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
	uint idx = 0;
	while (idx < nsize) if ((us)pdata[idx++] == 0xFEFF) break;
	if (++idx + 4 + 4 + 2 >= nsize)
	{
		nsize = 0;
		return;
	}
	nlen = (int)pdata[idx], idx += 4;
	if (nlen + idx - 1 > nsize)
	{
		nsize = 0;
		return;
	}
	scmd = (us)pdata[idx], idx += 2;
	int sum = 0;
	strbuf = "";
	while (idx + 4 <= nsize) sum += pdata[idx], strbuf += pdata[idx++];
	lsum = (ll)pdata[idx], idx += 4;

	if (sum == lsum) nsize = idx;
	else nsize = 0;
}

cpacket::cpacket(const cpacket& cp)
{
	shead = cp.shead;
	nlen = cp.nlen;
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	lsum = cp.lsum;
}

cpacket& cpacket::operator=(const cpacket& cp)
{
	shead =cp.shead; 
	nlen = cp.nlen; 
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	lsum = cp.lsum;
	return *this;
}

cpacket::~cpacket()
{
}

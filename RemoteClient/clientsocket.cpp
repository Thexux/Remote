#include "pch.h"
#include "clientsocket.h"
#include"common.h"

csocket* csocket::m_csock = 0;
//csocket* pclient = csocket::getsocket();
csocket::chelper csocket::m_help;

csocket* csocket::getsocket()
{
	if (m_csock == 0) m_csock = new csocket();
	return m_csock;
}

const int BUF_SIZE = 4096;
csocket::csocket()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data))
	{
		MessageBox(NULL, _T("无法初始化套接字环境，请检查网络设置"), _T("初始化错误"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	vbuf.resize(BUF_SIZE);
}

csocket::csocket(const csocket& cs)
{

}

csocket::~csocket()
{
	closesocket(m_sock);
	WSACleanup();
}

bool csocket::init(const string& strip)
{
	if (m_sock != -1) closesock();
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == -1) return 0;

	sockaddr_in sev_addr;
	memset(&sev_addr, 0, sizeof sev_addr);
	sev_addr.sin_family = AF_INET;
	sev_addr.sin_addr.s_addr = inet_addr(strip.c_str());
	sev_addr.sin_port = htons(9527);
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

void csocket::closesock()
{
	closesocket(m_sock);
	m_sock = -1;
}

int csocket::dealcommand()
{
	if (m_sock == -1) return -1;
	char* buf = vbuf.data();
	memset(buf, 0, sizeof buf);
	uint idx = 0;
	while (1)
	{
		int len = recv(m_sock, buf + idx, BUF_SIZE - idx, 0);
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
	if (m_sock == -1) return 0;
	return send(m_sock, pdata, nsize, 0) > 0;

}

//bool csocket::sendate(cpacket& pack)
//{
//	if (m_client == -1) return 0;
//	return send(m_client, pack.data(), pack.size(), 0) > 0;
//}

bool csocket::sendate(cpacket pack)
{
	if (m_sock == -1) return 0;
	int res = send(m_sock, pack.data(), pack.size(), 0);
	return 1;
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
	uint idx = 1;
	while (idx < nsize) if (pdata[idx] << 8 | pdata[idx - 1] == 0xFEFF) break;

	if (++idx + 4 + 4 + 2 > nsize)
	{
		nsize = 0;
		return;
	}

	nlen = *(int*)&pdata[idx], idx += 4;

	if (nlen + idx - 1 > nsize)
	{
		nsize = 0;
		return;
	}

	scmd = *(us*)&pdata[idx], idx += 2;

	int sum = 0;
	strbuf = "";
	while (idx + 4 < nsize) sum += pdata[idx], strbuf += pdata[idx++];

	nsum = *(int*)&pdata[idx], idx += 4;

	if (sum == nsum) nsize = idx;
	else nsize = 0;
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

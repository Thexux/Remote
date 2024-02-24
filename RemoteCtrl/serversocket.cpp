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

bool csocket::init(short port)
{
	if (m_sock == -1) return 0;

	sockaddr_in sev_addr;
	memset(&sev_addr, 0, sizeof sev_addr);
	sev_addr.sin_family = AF_INET;
	sev_addr.sin_addr.s_addr = INADDR_ANY;
	sev_addr.sin_port = htons(port);
	if (bind(m_sock, (sockaddr*)&sev_addr, sizeof sev_addr) == SOCKET_ERROR) return 0;
	if (listen(m_sock, 1) == -1) return 0;
	return 1;
}

int csocket::run(SOCKET_CALLBACK callback, void* arg, short port)
{
	m_callback = callback;
	m_arg = arg;
	if (init(port) == 0) return -1;
	list<cpacket> lstpacket;
	int cnt = 0;
	while (1)
	{
		cout << "==========开始等待连接=============" << endl;
		if (acceptclient() == 0)
		{
			if (cnt > 3) return -2;
			cnt++;
			continue;
		}
		cnt = 0;
		int res = dealcommand();
		if (res > 0)
		{
			m_callback(m_arg, res, lstpacket, m_packet);
			while (lstpacket.size()) sendate(lstpacket.front()), lstpacket.pop_front();
		}
		closeclient();
	}

	return 0;
}

bool csocket::acceptclient()
{
	sockaddr_in cli_addr;
	int cli_len = sizeof(cli_addr);
	m_client = accept(m_sock, (sockaddr*)&cli_addr, &cli_len);
	cout << "client socket:" << (uint)m_client << endl;
	return m_client != -1;
}

const int BUF_SIZE = 4096;
int csocket::dealcommand()
{
	if (m_client == -1) return -1;
	char* buf = new char[BUF_SIZE];
	if (buf == 0) return -2; //内存不足
	memset(buf, 0, sizeof buf);
	uint idx = 0;
	while (1)
	{
		int len = recv(m_client, buf + idx, BUF_SIZE - idx, 0);
		if (len <= 0)
		{
			delete[] buf;
			return -1;
		}
		idx += len; 
		m_packet = cpacket((uchar*)buf, len);
		if (len)
		{
			memmove(buf, buf + len, BUF_SIZE - len);
			idx -= len;
			delete[] buf;
			return m_packet.scmd;
		}
	}
}

void csocket::closeclient()
{
	if (m_client == INVALID_SOCKET) return;
	closesocket(m_client);
	m_client = INVALID_SOCKET;
}

bool csocket::sendate(const char* pdata, uint nsize)
{
	if (m_client == -1) return 0;
	return send(m_client, pdata, nsize, 0) > 0;
	
}

bool csocket::sendate(cpacket& pack)
{
	if (m_client == -1) return 0;
	return send(m_client, pack.data(), pack.size(), 0) > 0;
}

//cpacket& csocket::getpacket()
//{
//	return m_packet;
//}
//
//string csocket::getfilepath()
//{
//	return m_packet.strbuf;
//}
//
//MOUSEV csocket::getmousevent()
//{
//	return *(MOUSEV*)m_packet.strbuf.c_str();
//}
 
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
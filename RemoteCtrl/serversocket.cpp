#include "pch.h"
#include "serversocket.h"

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
	SOCKET m_sock = -1;
	SOCKET m_client = -1;
	if (initsock() == 0)
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
	if (bind(m_sock, (sockaddr*)&sev_addr, sizeof sev_addr) == -1) return 1;
	if (listen(m_sock, 1) == -1) return 1;

	return 1;
}

bool csocket::initsock()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data)) return 0;
	return 1;
}

bool csocket::acceptclient()
{
	sockaddr_in cli_addr;
	int cli_len = sizeof(cli_addr);
	m_client = accept(m_sock, (sockaddr*)&cli_addr, &cli_len);
	return m_client != -1;
}

int csocket::dealcommand()
{
	if (m_client == -1) return 0;
	char buf[1024] = "";
	while (1)
	{
		int len = recv(m_client, buf, sizeof buf, 0);
		if (len < 0) return -1;
		//todo: 处理命令
	}
}

bool csocket::sendate(const char* data, uint size)
{
	if (m_client == -1) return 0;
	return send(m_client, data, size, 0) > 0;
	
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

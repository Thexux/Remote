#include "pch.h"
#include "iocpserver.h"

cacceptoverlapped::cacceptoverlapped()
{
	m_operator = EAccept;
	m_work = threadwork(this, (FUNCTYPE)&cacceptoverlapped::acceptwork);
	memset(&m_overlapped, 0, sizeof m_overlapped);
	m_buf.reserve(1024);
	m_server = NULL;
}

int cacceptoverlapped::acceptwork()
{
	int llen = 0, rlen = 0;
	if (*(LPDWORD)*m_client > 0)
	{
		GetAcceptExSockaddrs(*m_client, 0,
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			(sockaddr**)m_client->getlocaladdr(), &llen, // 本地地址
			(sockaddr**)m_client->getremoteaddr(), &rlen); // 远程地址
		if (!m_server->newaccept()) return -1;
		return 0;
	}
	return -2;
}

iocpclient::iocpclient() : m_isbusy(0), m_overlapped(new cacceptoverlapped)
{
	m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	m_buf.resize(1024);
	memset(&m_laddr, 0, sizeof m_laddr);
	memset(&m_raddr, 0, sizeof m_raddr);
}

void iocpclient::setoverlapped(PCLIENT& ptr)
{
	m_overlapped->m_client = ptr;
}

iocpclient::operator LPOVERLAPPED()
{
	return &m_overlapped->m_overlapped;
}
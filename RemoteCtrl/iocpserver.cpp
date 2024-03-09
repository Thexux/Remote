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

		WSARecv(*m_client, m_client->recvwsabuf(), 1, *m_client, m_client->flags(), *m_client, 0);





		if (!m_server->newaccept()) return -1;
		return 0;
	}
	return -2;
}

iocpclient::iocpclient() :
	m_isbusy(0), m_flags(0), 
	m_overlapped(new cacceptoverlapped),
	m_recvolp(new crecvoverlapped),
	m_sendolp(new csendoverlapped),
	m_vecsend(this, (SENDCALLBACK)&iocpclient::sendpack)
{
	m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
	m_buf.resize(1024);
	memset(&m_laddr, 0, sizeof m_laddr);
	memset(&m_raddr, 0, sizeof m_raddr);
}

void iocpclient::setoverlapped(PCLIENT& ptr)
{
	m_overlapped->m_client = ptr.get();
	m_recvolp->m_client = ptr.get();
	m_sendolp->m_client = ptr.get();
}

iocpclient::operator LPOVERLAPPED()
{
	return &m_overlapped->m_overlapped;
}

LPWSABUF iocpclient::recvwsabuf()
{
	return &m_recvolp->m_wsabuf;
}

LPWSABUF iocpclient::sendwsabuf()
{
	return &m_sendolp->m_wsabuf;
}

int iocpclient::sendpack(std::vector<char>& data)
{
	if (m_vecsend.size() > 0)
	{
		int res = WSASend(m_sock, sendwsabuf(), 1, &m_recv, m_flags, &m_sendolp->m_overlapped, NULL);
		if (res != 0 && (WSAGetLastError() != WSA_IO_PENDING))
		{
			return -1;
		}
	}
	return 0;
}
crecvoverlapped::crecvoverlapped()
{
	m_operator = ERecv;
	m_work = threadwork(this, (FUNCTYPE)&crecvoverlapped::recvwork);
	memset(&m_overlapped, 0, sizeof m_overlapped);
	m_buf.reserve(1024 * 256);
}

int crecvoverlapped::recvwork()
{
	int res = m_client->receive();
	return res;
}

ciocpserver::~ciocpserver()
{
	closesocket(m_sock);
	for (auto it = m_client.begin(); it != m_client.end(); it++)
		it->second.reset();
	m_client.clear();
	CloseHandle(m_hiocp);
	m_pool.stop();
}

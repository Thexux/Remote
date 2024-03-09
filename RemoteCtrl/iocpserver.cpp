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
	if (m_client->getbufsize() > 0)
	{
		sockaddr *plocal = NULL, *premote = NULL;
		GetAcceptExSockaddrs(*m_client, 0,
			sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
			(sockaddr**)&plocal, &llen, // 本地地址
			(sockaddr**)&premote, &rlen); // 远程地址

		memcpy(m_client->getlocaladdr(), plocal, sizeof(sockaddr_in));
		memcpy(m_client->getremoteaddr(), premote, sizeof(sockaddr_in));

		m_server->bindnewsock(*m_client);
		int res = WSARecv(*m_client, m_client->recvwsabuf(), 1, *m_client, m_client->flags(), m_client->recvoverlapped(), 0);
		if (res == -1 && WSAGetLastError() != WSA_IO_PENDING)
		{
			//TODO: 错误处理
		}




		if (!m_server->newaccept()) return -2;
	}
	return -1;
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

LPOVERLAPPED iocpclient::recvoverlapped()
{
	return &m_recvolp->m_overlapped;
}

LPOVERLAPPED iocpclient::sendoverlapped()
{
	return &m_sendolp->m_overlapped;
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

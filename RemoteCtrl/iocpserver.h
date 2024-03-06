#pragma once
#include "threadpool.h"
#include <MSWSock.h>
#include <map>
#include "cpqueue.h"

enum iocpoperator
{
	ENone,
	EAccept,
	ERecv,
	ESend,
	EError
};

class ciocpserver;
class cacceptoverlapped;
class iocpclient;
typedef std::shared_ptr<iocpclient> PCLIENT;
class iocpclient
{
public:
	iocpclient();
	~iocpclient()
	{
		closesocket(m_sock);
	}
	void setoverlapped(PCLIENT& ptr);
	operator SOCKET()
	{
		return m_sock;
	}
	operator PVOID()
	{
		return m_buf.data();
	}
	operator LPOVERLAPPED();
	operator LPDWORD()
	{
		return &m_recv;
	}
	sockaddr_in* getlocaladdr() { return &m_laddr; }
	sockaddr_in* getremoteaddr() { return &m_raddr; }

private:
	SOCKET m_sock;
	DWORD m_recv;
	std::shared_ptr<cacceptoverlapped> m_overlapped;
	std::vector<char> m_buf;
	sockaddr_in m_laddr;
	sockaddr_in m_raddr;
	bool m_isbusy;
};

class coverlapped
{
public:
	OVERLAPPED m_overlapped;
	int m_operator; // 操作 
	std::vector<char> m_buf; // 缓冲区
	threadwork m_work; // 处理函数
	ciocpserver* m_server; // 服务器对象
	PCLIENT m_client;
};

class cacceptoverlapped : public coverlapped, threadfuncbase
{
public:
	cacceptoverlapped();
	int acceptwork();
};

class crecvoverlapped : public coverlapped, threadfuncbase
{
public:
	crecvoverlapped()
	{
		m_operator = ERecv;
		m_work = threadwork(this, (FUNCTYPE)&crecvoverlapped::recvwork);
		memset(&m_overlapped, 0, sizeof m_overlapped);
		m_buf.reserve(1024 * 256);
	}
	int recvwork()
	{
		//TODO:
	}
};
class csendoverlapped : public coverlapped, threadfuncbase
{
public:
	csendoverlapped()
	{
		m_operator = ESend;
		m_work = threadwork(this, (FUNCTYPE)&csendoverlapped::sendwork);
		memset(&m_overlapped, 0, sizeof m_overlapped);
		m_buf.reserve(1024 * 256);
	}
	int sendwork()
	{
		//TODO:
	}
};
class cerroroverlapped : public coverlapped, threadfuncbase
{
public:
	cerroroverlapped()
	{
		m_operator = EError;
		m_work = threadwork(this, (FUNCTYPE)&cerroroverlapped::errorwork);
		memset(&m_overlapped, 0, sizeof m_overlapped);
		m_buf.reserve(1024);
	}
	int errorwork()
	{
		//TODO:
	}
};

class ciocpserver :
	public threadfuncbase
{
public:
	ciocpserver(const std::string& ip = "0,0,0,0", short port = 9527) : m_pool(10)
	{
		m_hiocp = INVALID_HANDLE_VALUE;
		m_sock = INVALID_SOCKET;

		m_addr.sin_family = AF_INET;
		m_addr.sin_port = htons(port);
		m_addr.sin_addr.s_addr = inet_addr(ip.c_str());
	}

	bool startservice()
	{
		createsocket();

		if (bind(m_sock, (sockaddr*)&m_addr, sizeof m_addr) == -1)
		{
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			return false;
		}
		if (listen(m_sock, 3) == -1)
		{
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			return false;
		}

		m_hiocp = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 4);
		if (m_hiocp == NULL)
		{
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			m_hiocp = INVALID_HANDLE_VALUE;
			return false;
		}
		CreateIoCompletionPort((HANDLE)m_sock, m_hiocp, 0, 0);
		m_pool.invoke();
		m_pool.dispatchwork(threadwork(this, (FUNCTYPE)&ciocpserver::threadiocp));
		if (!newaccept()) return false;

		
		return true;
	}

	bool newaccept()
	{
		PCLIENT pclient(new iocpclient);
		pclient->setoverlapped(pclient);
		m_client[*pclient] = pclient;
		if (!AcceptEx(m_sock, *pclient, *pclient, 0, sizeof(sockaddr_in) + 16,
			sizeof(sockaddr_in) + 16, *pclient, *pclient))
		{
			closesocket(m_sock);
			m_sock = INVALID_SOCKET;
			m_hiocp = INVALID_HANDLE_VALUE;
			return false;
		}
		return true;
	}


	~ciocpserver() {}

private:
	void createsocket()
	{
		m_sock = WSASocket(AF_INET, SOCK_STREAM, 0, 0, 0, WSA_FLAG_OVERLAPPED);
		int opt = 1;
		setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt, sizeof opt);
	}

	int threadiocp()
	{
		DWORD tranferrid = 0;
		ULONG_PTR completionkey = 0;
		OVERLAPPED* lpoverlapped = NULL;
		if (GetQueuedCompletionStatus(m_hiocp, &tranferrid, &completionkey, &lpoverlapped, INFINITE))
		{
			if (tranferrid > 0 && completionkey)
			{
				coverlapped* poverlapped = CONTAINING_RECORD(lpoverlapped, coverlapped, m_overlapped);
				switch (poverlapped->m_operator)
				{
				case EAccept:
				{
					cacceptoverlapped* paccept = (cacceptoverlapped*)poverlapped;
					m_pool.dispatchwork(paccept->m_work);
				}
				break;
				case ERecv:
				{
					crecvoverlapped* precv = (crecvoverlapped*)poverlapped;
					m_pool.dispatchwork(precv->m_work);
				}
				break;
				case ESend:
				{
					csendoverlapped* psend = (csendoverlapped*)poverlapped;
					m_pool.dispatchwork(psend->m_work);
				}
				break;
				case EError:
				{
					cerroroverlapped* perror = (cerroroverlapped*)poverlapped;
					m_pool.dispatchwork(perror->m_work);
				}
				break;
				}
			}
			return -1;
		}
		return 0;
	}

private:
	cthreadpool m_pool;
	HANDLE m_hiocp;
	SOCKET m_sock;
	sockaddr_in m_addr;
	std::map<SOCKET, std::shared_ptr<iocpclient>> m_client;
};


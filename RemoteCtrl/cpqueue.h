#pragma once
#include "pch.h"
#include <atomic>
#include <list>

template<class T>
class cpqueue
{
public:
	enum
	{
		cqnone, cqpush, cqpop, cqsize, cqclear
	};
	struct iocparam
	{
		size_t noperator; //操作
		T data; //数据
		_beginthread_proc_type cbfunc; //回调
		HANDLE hevent; //pop操作需要
		iocparam(int op, const T& datax, HANDLE heve = NULL)
		{
			noperator = op;
			data = datax;
			hevent = heve;
		}
		iocparam()
		{
			noperator = cqnone;
		}
	};
public:
	cpqueue()
	{
		m_lock = 0;
		m_hcompeletionport = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, NULL, 1);
		m_hthread = INVALID_HANDLE_VALUE;
		if (m_hcompeletionport)
			m_hthread = (HANDLE)_beginthread(
				&cpqueue<T>::threadrntry,
				0, this);
	}
	~cpqueue()
	{
		if (m_lock) return;
		m_lock = 1;
		PostQueuedCompletionStatus(m_hcompeletionport, 0, NULL, NULL);
		WaitForSingleObject(m_hthread, INFINITE);
		HANDLE ht = m_hcompeletionport;
		m_hcompeletionport = NULL;
		CloseHandle(ht);
	}
	bool pushback(const T& data)
	{
		iocparam* param = new iocparam(cqpush, data);
		if (m_lock)
		{
			delete param;
			return false;
		}
		bool res = PostQueuedCompletionStatus(m_hcompeletionport,
			sizeof(iocparam), (ULONG_PTR)param, 0);
		if (res == 0) delete param;
		return res;
	}
	bool popfront(T& data)
	{
		HANDLE hevent = CreateEvent(NULL, TRUE, FALSE, NULL);
		iocparam param(cqpop, data, hevent);
		if (m_lock)
		{
			if (hevent) CloseHandle(hevent);
			return false;
		}
		bool res = PostQueuedCompletionStatus(m_hcompeletionport,
			sizeof(iocparam), (ULONG_PTR)&param, 0);
		if (res == 0)
		{
			CloseHandle(hevent);
			return false;
		}
		res = WaitForSingleObject(hevent, INFINITE) == WAIT_OBJECT_0;
		if (res) data = param.data;
		return res;
	}
	size_t size()
	{
		HANDLE hevent = CreateEvent(NULL, TRUE, FALSE, NULL);
		iocparam param(cqsize, T(), hevent);
		if (m_lock)
		{
			if (hevent) CloseHandle(hevent);
			return -1;
		}
		bool res = PostQueuedCompletionStatus(m_hcompeletionport,
			sizeof(iocparam), (ULONG_PTR)&param, 0);
		if (res == 0)
		{
			CloseHandle(hevent);
			return -1;
		}
		res = WaitForSingleObject(hevent, INFINITE) == WAIT_OBJECT_0;
		if (res) return param.noperator;

		return -1;
	}
	bool clear()
	{
		if (m_lock) return false;
		iocparam* param = new iocparam(cqclear, T());
		bool res = PostQueuedCompletionStatus(m_hcompeletionport,
			sizeof(iocparam), (ULONG_PTR)param, 0);
		if (res == 0) delete param;
		return res;
	}
private:
	static void threadrntry(void* arg)
	{
		cpqueue<T>* pthis = (cpqueue<T>*) arg;
		pthis->threadmain();
		_endthread();
	}
	void dealparam(iocparam* param)
	{
		if (param->noperator == cqpush) m_lstdata.push_back(param->data), delete param;
		else if (param->noperator == cqpop)
		{
			if (m_lstdata.size() == 0) return;
			param->data = m_lstdata.front();
			m_lstdata.pop_front();
			SetEvent(param->hevent);
		}
		else if (param->noperator == cqsize)
		{
			param->noperator = m_lstdata.size();
			SetEvent(param->hevent);
		}
		else if (param->noperator == cqclear) m_lstdata.clear(), delete param;
		else OutputDebugString(_T("unknow operator!\r\n"));
	}
	void threadmain()
	{
		iocparam* param = 0;
		DWORD dwtransferred = 0;
		ULONG_PTR completionkey = 0;
		OVERLAPPED* poverlapped = 0;

		while (GetQueuedCompletionStatus(m_hcompeletionport, &dwtransferred,
			&completionkey, &poverlapped, INFINITE))
		{
			if (dwtransferred == 0 || completionkey == 0) break;

			param = (iocparam*)completionkey;
			dealparam(param);
		}
		while (GetQueuedCompletionStatus(m_hcompeletionport, &dwtransferred,
			&completionkey, &poverlapped, 0))
		{
			if (dwtransferred == 0 || completionkey == 0) continue;

			param = (iocparam*)completionkey;
			dealparam(param);
		}
		//CloseHandle(m_hcompeletionport);
	}
private:
	std::list<T> m_lstdata;
	HANDLE m_hcompeletionport;
	HANDLE m_hthread;
	std::atomic<bool> m_lock;
};
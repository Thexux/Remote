#pragma once
#include "pch.h"
#include <atomic>
#include <vector>
#include <mutex>
#include <Windows.h>


class threadfuncbase {};
typedef int (threadfuncbase::* FUNCTYPE)();

class threadwork
{
public:
	threadwork() :pfunb(NULL), func(NULL) {}
	threadwork(void* obj, FUNCTYPE f) :pfunb((threadfuncbase*)obj), func(f) {}
	threadwork(const threadwork& work)
	{
		pfunb = work.pfunb, func = work.func;
	}
	threadwork& operator=(const threadwork& work)
	{
		if (this == &work) return *this;
		pfunb = work.pfunb, func = work.func;
		return *this;
	}
	int operator()()
	{
		if (isvaild()) return (pfunb->*func)();
		return -1;
	}
	bool isvaild() const
	{
		return pfunb && func;
	}

private:
	threadfuncbase* pfunb;
	FUNCTYPE func;
};

class cthread
{
public:
	cthread() : m_hthread(NULL), m_bstatus(0) {}
	~cthread()
	{
		stop();
	}

	bool start()
	{
		m_bstatus = true;
		m_hthread = (HANDLE)_beginthread(&cthread::threadentry, 0, this);
		if (isvalid()) return true;
		return m_bstatus = false;
	}

	bool stop()
	{
		if (m_bstatus == false) return true;
		m_bstatus = false;
		int res = WaitForSingleObject(m_hthread, INFINITE) == WAIT_OBJECT_0;
		updatework();
		return res;
	}

	bool isvalid() 
	{
		if (m_hthread == NULL || m_hthread == INVALID_HANDLE_VALUE) return false;
		return WaitForSingleObject(m_hthread, 0) == WAIT_TIMEOUT;
	}

	bool isidle() // true 表示空闲，false 表示已分配
	{
		if (m_work.load() == NULL) return true;
		return !m_work.load()->isvaild();
	}

	void updatework(const ::threadwork& work = ::threadwork())
	{
		if (m_work.load())
		{
			::threadwork* pwork = m_work.load();
			m_work.store(NULL);
			delete pwork;
		}
		if (!work.isvaild()) m_work.store(NULL);
		else m_work.store(new ::threadwork(work));
	}

private:
	void threadwork()
	{
		while (m_bstatus)
		{
			if (m_work.load() == NULL)
			{
				Sleep(1);
				continue;
			}
			::threadwork work = *m_work.load();
			if (work.isvaild())
			{
				int res = work();
				if (res)
				{
					CString str;
					str.Format(_T("thread found warning code %d\r\n"), res);
					OutputDebugString(str);
				}
				if (res < 0)
				{
					::threadwork* pwork = m_work.load();
					m_work.store(NULL);
					delete pwork;
				}
			}
			else Sleep(1);
		}
	}
	static void threadentry(void* arg)
	{
		cthread* pthis = (cthread*)arg;
		if (pthis) pthis->threadwork();
		_endthread();
	}

private:
	HANDLE m_hthread;
	bool m_bstatus;   
	std::atomic<::threadwork*> m_work;
};

class cthreadpool
{
public:

	cthreadpool() {}
	cthreadpool(int size)
	{
		m_threads.resize(size);
		for (int i = 0; i < size; i++) m_threads[i] = new cthread();
	}
	~cthreadpool()
	{
		stop();
		for (int i = 0; i < m_threads.size(); i++) delete m_threads[i];
		m_threads.clear();
	}
	bool invoke()
	{
		bool ok = true;
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i]->start() == false)
			{
				ok = false;
				break;
			}
		}
		if (ok == false) for (size_t i = 0; i < m_threads.size(); i++) m_threads[i]->stop();
		return ok;
	}
	void stop()
	{
		for (size_t i = 0; i < m_threads.size(); i++) m_threads[i]->stop();
	}

	int dispatchwork(const threadwork& worker)
	{
		int idx = -1;
		m_lock.lock();
		for (size_t i = 0; i < m_threads.size(); i++)
		{
			if (m_threads[i]->isidle())
			{
				m_threads[i]->updatework(worker), idx = i;
				break;
			}
		}
		m_lock.unlock();
		return idx;
	}

	bool checkthreadvalid(int idx)
	{
		if (idx >= 0 && idx < (int)m_threads.size()) return m_threads[idx]->isvalid();
		return false;
	}

private:
	std::mutex m_lock;
	std::vector<cthread*> m_threads;
};
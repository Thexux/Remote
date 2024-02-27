#pragma once
#include"common.h"
#include<mutex>
#include "tool.h"
const int FILESIZE = 256;

struct FILEINFO
{
	bool isvalid = 0;
	bool isdirectory = 0;
	bool hasnext = 1;
	char filename[FILESIZE] = { 0 };
};

//#pragma pack(push)
//#pragma pack(1)
class cpacket
{
public:
	cpacket();
	cpacket(const uchar* pdata, int& nsize);
	cpacket(const cpacket& cp);
	cpacket(us cmd, const uchar* pdata, int nsize, HANDLE hevent);
	cpacket& operator=(const cpacket& cp);
	int size();
	const char* data();
	~cpacket();
	us shead = 0XFEFF; //包头固定位 FE FF
	int nlen = 0; // 包长度（从命令开始）
	us scmd = 0; // 控制命令
	string strbuf; // 包数据
	int nsum = 0; // 和校验
	string strout; // 整个包数据
	HANDLE hevent;
};
//#pragma pack(pop)

struct MOUSEV
{
	us sact = 0; // 点击、移动、双击
	us sbtn = -1; // 左键、右键、中建 
	POINT pt = { 0, 0 }; // 坐标
};

class csocket
{
public:
	static csocket* getsocket();
	bool init();
	void updateaddr(int nip, int nort);
	void closesock();
	bool sendpacket(const cpacket& pack, list<cpacket>& lstpacks)
	{
		cout << m_sock << endl;
		if (m_sock == -1 && m_hthread == INVALID_HANDLE_VALUE)
			m_hthread = (HANDLE)_beginthread(threadentry, 0, this); // TODO: 套接字关闭可能会多开
		//TRACE("cmd %d, thread id %d\r\n", pack.scmd, GetCurrentThreadId());
		mu_lock.lock();
		m_lstsend.push_back(pack);
		mu_lock.unlock();
		WaitForSingleObject(pack.hevent, INFINITE);
		if (m_mpack.find(pack.hevent) == m_mpack.end()) return false; // TODO：错误处理
		for (auto u : m_mpack[pack.hevent]) lstpacks.push_back(u);
		mu_lock.lock();
		m_mpack.erase(m_mpack.find(pack.hevent));
		mu_lock.unlock();
		return true;
	}
	int dealcommand();
	cpacket& getpacket();
	string getfilepath();
	MOUSEV getmousevent();

private:
	HANDLE m_hthread;
	std::mutex mu_lock;
	static void threadentry(void* arg);
	void threadfunc();
	csocket();
	csocket(const csocket&);
	csocket& operator=(const csocket& cs);
	~csocket();
	bool sendate(const char* pdata, uint nsize);
	//bool sendate(cpacket& pack);
	bool sendate(cpacket pack);
	static void releasesock();
	class chelper
	{
	public:
		chelper();
		~chelper();
	};
	list<cpacket> m_lstsend;
	map<HANDLE, list<cpacket>> m_mpack;
	int m_nip;
	int m_nport;
	vector<char> vbuf;
	int nbufidx;
	static csocket* m_csock;
	static chelper m_help;
	SOCKET m_sock;
	cpacket m_packet;
};


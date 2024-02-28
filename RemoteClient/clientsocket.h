#pragma once
#include"common.h"
#include<mutex>
#include "tool.h"

#define WM_SEND_PACK (WM_USER + 1) // ���Ͱ�����
#define WM_SEND_PACK_ACK (WM_USER + 2) 

const int FILESIZE = 256;
const int BUF_SIZE = 2048000;

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
	cpacket(us cmd, const uchar* pdata, int nsize);
	cpacket& operator=(const cpacket& cp);
	int size();
	const char* data();
	~cpacket();
	us shead = 0XFEFF; //��ͷ�̶�λ FE FF
	int nlen = 0; // �����ȣ������ʼ��
	us scmd = 0; // ��������
	string strbuf; // ������
	int nsum = 0; // ��У��
	string strout; // ����������
};
//#pragma pack(pop)

struct MOUSEV
{
	us sact = 0; // ������ƶ���˫��
	us sbtn = -1; // ������Ҽ����н� 
	POINT pt = { 0, 0 }; // ����
};

struct packdata
{
	string strdata;
	uint extend;
	packdata(const char* pdata, int nlen, uint nextend = 0)
	{
		strdata.resize(nlen);
		memcpy((char*)strdata.c_str(), pdata, nlen);
		extend = nextend;
	}
	packdata(const packdata& data)
	{
		strdata = data.strdata;
		extend = data.extend;
	}
	packdata& operator=(const packdata& data)
	{
		if (this == &data) return *this;
		strdata = data.strdata;
		extend = data.extend;
		return *this;
	}
};

class csocket
{
public:
	static csocket* getsocket();
	bool init();
	void updateaddr(int nip, int nort);
	void closesock();
	bool sendpacket(HWND hwnd, cpacket pack, WPARAM wparam = 0);
	int dealcommand();
	cpacket& getpacket();
	string getfilepath();
	MOUSEV getmousevent();

private:
	HANDLE m_eventinvoke; // �߳������¼�
	typedef void(csocket::* MSGFUNC)
		(UINT nmsg, WPARAM wparam, LPARAM lparam);
	map<UINT, MSGFUNC> m_mpfun;
	HANDLE m_hthread;
	uint m_nthreadid;
	std::mutex mu_lock;
	static unsigned __stdcall threadentry(void* arg);
	void threadfunc();
	void sendpackmsg(UINT nmsg, WPARAM wparam/*��������ֵ*/, LPARAM lparam/*���*/);
	csocket();
	csocket(const csocket&);
	csocket& operator=(const csocket& cs);
	~csocket();
	bool sendate(const char* pdata, uint nsize);
	bool sendate(cpacket pack);
	static void releasesock();
	class chelper
	{
	public:
		chelper();
		~chelper();
	};
	int m_nip;
	int m_nport;
	vector<char> vbuf;
	int nbufidx;
	static csocket* m_csock;
	static chelper m_help;
	SOCKET m_sock;
	cpacket m_packet;
};


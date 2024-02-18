#pragma once
#include"common.h"

const int FILESIZE = 256; //256

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
	us shead = 0; //包头固定位 FE FF
	int nlen = 0; // 包长度（从命令开始）
	us scmd = 0; // 控制命令
	string strbuf; // 包数据
	int nsum = 0; // 和校验
	string strout; // 整个包数据
};
//#pragma pack(pop)

struct MOUSEV
{
	us sact = 0; // 点击、移动、双击
	us sbtn = -1; // 左键、右键、中建 
	POINT pt = {0, 0}; // 坐标
};

class csocket
{
public:
	static csocket* getsocket();
	bool init();
	bool acceptclient();
	int dealcommand();
	void closeclient();
	bool sendate(const char* pdata, uint nsize);
	//bool sendate(cpacket& pack);
	bool sendate(cpacket pack);
	cpacket& getpacket();
	string getfilepath();
	MOUSEV getmousevent();

private:
	csocket();
	csocket(const csocket&);
	csocket& operator=(const csocket& cs);
	~csocket();
	static void releasesock();
	class chelper
	{
	public:
		chelper();
		~chelper();
	};
	static csocket* m_csock;
	static chelper m_help;
	SOCKET m_sock;
	SOCKET m_client;
	cpacket m_packet;
};

  
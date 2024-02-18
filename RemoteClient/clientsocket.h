#pragma once
#include"common.h"

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
	us shead = 0; //��ͷ�̶�λ FE FF
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

class csocket
{
public:
	static csocket* getsocket();
	bool init(int nip, int nport);
	void closesock();
	int dealcommand();
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
	vector<char> vbuf;
	static csocket* m_csock;
	static chelper m_help;
	SOCKET m_sock;
	cpacket m_packet;
};


#pragma once
#include"common.h"

class cpacket
{
public:
	cpacket();
	cpacket(const uchar* pdata, int& nsize);
	cpacket(const cpacket& cp);
	cpacket& operator=(const cpacket& cp);
	~cpacket();
	us shead = 0; //��ͷ�̶�λ FE FF
	int nlen = 0; // �����ȣ������ʼ��
	us scmd = 0; // ��������
	string strbuf; // ������
	int lsum = 0; // ��У��
};

class csocket
{
public:
	static csocket* getsocket();
	bool init();
	bool acceptclient();
	int dealcommand();
	bool sendate(const char* pdata, uint nsize);

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

  
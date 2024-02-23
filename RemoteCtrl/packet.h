#pragma once
#include"common.h"

const int FILESIZE = 256; //256

#pragma pack(push)
#pragma pack(1)
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
#pragma pack(pop)

struct FILEINFO
{
	bool isvalid = 0;
	bool isdirectory = 0;
	bool hasnext = 1;
	char filename[FILESIZE] = { 0 };
};

struct MOUSEV
{
	us sact = 0; // ������ƶ���˫��
	us sbtn = -1; // ������Ҽ����н� 
	POINT pt = { 0, 0 }; // ����
};
#pragma once
#include"common.h"
#include"packet.h"

typedef void(*SOCKET_CALLBACK)(void*, int, list<cpacket>&, cpacket&);

class csocket
{
public:
	static csocket* getsocket();
	int run(SOCKET_CALLBACK callback, void* arg, short port = 9527);

protected:
	bool init(short port);
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
	SOCKET_CALLBACK m_callback;
	void* m_arg;
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

  
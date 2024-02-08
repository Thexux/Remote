#pragma once
#include"common.h"

class csocket
{
public:
	static csocket* getsocket();
	bool init();
	bool acceptclient();
	int dealcommand();
	bool sendate(const char* data, uint size);

private:
	csocket();
	csocket(const csocket&);
	csocket& operator=(const csocket& cs);
	~csocket();
	bool initsock();
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
};

  
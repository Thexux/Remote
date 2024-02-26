#include "pch.h"
#include "clientsocket.h"
#include "common.h"

const int BUF_SIZE = 2048000;

csocket* csocket::m_csock = 0;
//csocket* pclient = csocket::getsocket();
csocket::chelper csocket::m_help;

csocket* csocket::getsocket()
{
	if (m_csock == 0) m_csock = new csocket();
	return m_csock;
}

void csocket::threadentry(void* arg)
{
	csocket* psock = (csocket*)arg;
	psock->threadfunc();

}

void csocket::threadfunc()
{
	string strbuf;
	strbuf.reserve(BUF_SIZE);
	char* buf = (char*)strbuf.c_str();
	int idx = 0;
	while (1)
	{
		if (m_lstsend.size() == 0 || m_lstsend.front().shead != 0xfeff) continue;
		
		//Sleep(1);

		cpacket& head = m_lstsend.front();
		init();
		if (sendate(head) == 0) continue; // TODO��������
		
		//while (1)
		{
			int len = recv(m_sock, buf + idx, BUF_SIZE - idx, 0);
			//if (len <= 0) return -1;
			idx += len;

			len = idx;
			cpacket pack((uchar*)buf, len);
			if (len)
			{
				memmove(buf, buf + len, BUF_SIZE - len);
				idx -= len;
				m_mpack[head.hevent].push_back(pack);
				SetEvent(head.hevent);
			}

			//len = recv(m_sock, buf + idx, BUF_SIZE - idx, 0);
			////if (len <= 0) return -1;
			//idx += len;
		}
		//closesock();
		m_lstsend.pop_front();


	}
}

csocket::csocket()
{
	WSADATA data;
	if (WSAStartup(MAKEWORD(1, 1), &data))
	{
		MessageBox(NULL, _T("�޷���ʼ���׽��ֻ�����������������"), _T("��ʼ������"), MB_OK | MB_ICONERROR);
		exit(0);
	}
	vbuf.resize(BUF_SIZE);
	nbufidx = 0;
	m_nip = INADDR_ANY;
	m_nport = 0;
	m_sock = -1;
	m_lstsend.clear();
}

csocket::csocket(const csocket& cs)
{
}

csocket::~csocket()
{
	closesocket(m_sock);
	WSACleanup();
}

bool csocket::init()
{
	if (m_sock != -1) closesock();
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock == -1) return 0;
	cout << m_sock << endl;
	sockaddr_in sev_addr;
	memset(&sev_addr, 0, sizeof sev_addr);
	sev_addr.sin_family = AF_INET;
	sev_addr.sin_addr.s_addr = htonl(m_nip);
	sev_addr.sin_port = htons(m_nport);
	if (sev_addr.sin_addr.s_addr == INADDR_NONE)
	{
		AfxMessageBox("ָ��IP��ַ��������");
		return 0;
	}

	int res = connect(m_sock, (sockaddr*)&sev_addr, sizeof sev_addr);
	if (res == -1)
	{
		AfxMessageBox("����ʧ��");
		TRACE("����ʧ�ܣ�%d\r\n", WSAGetLastError());
		return 0;
	}
	return 1;
}

void csocket::updateaddr(int nip, int nport)
{
	m_nip = nip, m_nport = nport;
}

void csocket::closesock()
{
	closesocket(m_sock);
	m_sock = -1;
}

int csocket::dealcommand()
{
	if (m_sock == -1) return -1;

	char* buf = vbuf.data();
	//if (nbufidx == 0) memset(buf, 0, sizeof buf), nbufidx = 0;
	while (1)
	{
		int len = nbufidx;
		m_packet = cpacket((uchar*)buf, len);
		if (len)
		{
			memmove(buf, buf + len, BUF_SIZE - len);
			nbufidx -= len;
			return m_packet.scmd;
		}

		len = recv(m_sock, buf + nbufidx, BUF_SIZE - nbufidx, 0);
		if (len <= 0) return -1;
		nbufidx += len;
	}
}

bool csocket::sendate(const char* pdata, uint nsize)
{
	if (m_sock == -1) return 0;
	return send(m_sock, pdata, nsize, 0) > 0;

}

//bool csocket::sendate(cpacket& pack)
//{
//	if (m_client == -1) return 0;
//	return send(m_client, pack.data(), pack.size(), 0) > 0;
//}

bool csocket::sendate(cpacket pack)
{
	if (m_sock == -1) return 0;
	return send(m_sock, pack.data(), pack.size(), 0) > 0;
}

cpacket& csocket::getpacket()
{
	return m_packet;
}

string csocket::getfilepath()
{
	return m_packet.strbuf;
}

MOUSEV csocket::getmousevent()
{
	return *(MOUSEV*)m_packet.strbuf.c_str();
}

void csocket::releasesock()
{
	if (m_csock)
	{
		csocket* t = m_csock;
		delete t;
		m_csock = 0;
	}
}

csocket::chelper::chelper()
{
	csocket::getsocket();
}

csocket::chelper::~chelper()
{
	csocket::releasesock();
}

cpacket::cpacket()
{
}

cpacket::cpacket(const uchar* pdata, int& nsize)
{
	/*cout << "=====" << endl;
	for (int i = 0; i < nsize; i++)
	{
		if (i && i % 40 == 0) cout << endl;
		printf("%02X ", pdata[i]);
	}
	cout << endl;*/

	int idx = 0, n = nsize;
	nsize = 0;

	if (idx + 2 >= n) return;
	while (idx < n) if (*(us*)(pdata + idx++) == 0xFEFF) break;

	if (idx + 4 >= n) return;
	nlen = *(int*)&pdata[++idx], idx += 4;
	int nstsize = nlen - 6;
	//cout << "nlen" << ' ' << nlen << endl;
	if (idx + nlen > n) return;

	scmd = *(us*)&pdata[idx], idx += 2;
	//cout << "scmd" << ' ' << scmd << endl;

	int sum = 0;
	strbuf = "";

	for (int i = 0; i < nstsize; i++) sum += pdata[idx + i], strbuf += pdata[idx + i];
	idx += nstsize;
	//cout << "strbuf" << ' ' << strbuf << endl;
	/*FILEINFO* pinfo = (FILEINFO*)strbuf.c_str();
	cout << "strbuf:" << pinfo->filename << ' ' << pinfo->hasnext << ' ' << pinfo->isdirectory <<
		' ' << pinfo->isvalid << endl;*/

	nsum = *(int*)&pdata[idx], idx += 4;
	//cout << "nsum" << ' ' << nsum << endl;

	if (nsum == sum) nsize = idx;
}


cpacket::cpacket(const cpacket& cp)
{
	shead = cp.shead;
	nlen = cp.nlen;
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	nsum = cp.nsum;
	hevent = cp.hevent;
}

cpacket::cpacket(us cmd, const uchar* pdata, int nsize, HANDLE hevent)
{
	shead = 0xFEFF, nlen = nsize + 6, scmd = cmd, nsum = 0, strbuf = "";
	for (int i = 0; i < nsize; i++) nsum += pdata[i], strbuf += pdata[i];

	this->hevent = hevent;
	//cout << "======" << nlen << ' ' << scmd << ' ' << pdata << ' ' << strbuf << ' ' << nsum << endl;
}

cpacket& cpacket::operator=(const cpacket& cp)
{
	shead = cp.shead;
	nlen = cp.nlen;
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	nsum = cp.nsum;
	hevent = cp.hevent;
	return *this;
}

int cpacket::size()
{
	return nlen + 6;
}

const char* cpacket::data()
{
	strout.resize(nlen + 6);
	uchar* pdata = (uchar*)strout.c_str();
	*(us*)pdata = shead, pdata += 2;
	*(int*)pdata = nlen, pdata += 4;
	*(us*)pdata = scmd, pdata += 2;
	memcpy(pdata, strbuf.c_str(), strbuf.size()), pdata += strbuf.size();
	*(int*)pdata = nsum;
	return strout.c_str();
}

cpacket::~cpacket()
{
}

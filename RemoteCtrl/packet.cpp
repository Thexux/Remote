#include "pch.h"
#include "packet.h"
#include"common.h"

cpacket::cpacket()
{
}

cpacket::cpacket(const uchar* pdata, int& nsize)
{
	int idx = 0, n = nsize;
	nsize = 0;

	if (idx + 2 >= n) return;
	while (idx < n) if (*(us*)(pdata + idx++) == 0xFEFF) break;

	if (idx + 4 >= n) return;
	nlen = *(int*)&pdata[++idx], idx += 4;
	int nstsize = nlen - 6;

	if (idx + nlen > n) return;

	scmd = *(us*)&pdata[idx], idx += 2;

	int sum = 0;
	strbuf = "";

	for (int i = 0; i < nstsize; i++) sum += pdata[idx + i], strbuf += pdata[idx + i];
	idx += nstsize;

	nsum = *(int*)&pdata[idx], idx += 4;

	if (nsum == sum) nsize = idx;
}

cpacket::cpacket(const cpacket& cp)
{
	shead = cp.shead;
	nlen = cp.nlen;
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	nsum = cp.nsum;
}

cpacket::cpacket(us cmd, const uchar* pdata, int nsize)
{
	shead = 0xFEFF, nlen = nsize + 6, scmd = cmd, nsum = 0, strbuf = "";
	for (int i = 0; i < nsize; i++) nsum += pdata[i], strbuf += pdata[i];

	//cout << "======" << nlen << ' ' << scmd << ' ' << strbuf << ' ' << nsum << endl;
}

cpacket& cpacket::operator=(const cpacket& cp)
{
	shead = cp.shead;
	nlen = cp.nlen;
	scmd = cp.scmd;
	strbuf = cp.strbuf;
	nsum = cp.nsum;
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

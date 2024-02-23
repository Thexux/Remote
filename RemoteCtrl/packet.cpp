#include "pch.h"
#include "packet.h"
#include"common.h"

cpacket::cpacket()
{
}

cpacket::cpacket(const uchar* pdata, int& nsize)
{
	uint idx = 1;
	while (idx < nsize) if (pdata[idx] << 8 | pdata[idx - 1] == 0xFEFF) break;

	if (++idx + 4 + 4 + 2 > nsize)
	{
		nsize = 0;
		return;
	}

	nlen = *(int*)&pdata[idx], idx += 4;

	if (nlen + idx - 1 > nsize)
	{
		nsize = 0;
		return;
	}

	scmd = *(us*)&pdata[idx], idx += 2;

	int sum = 0;
	strbuf = "";
	while (idx + 4 < nsize) sum += pdata[idx], strbuf += pdata[idx++];

	nsum = *(int*)&pdata[idx], idx += 4;

	if (sum == nsum) nsize = idx;
	else nsize = 0;
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

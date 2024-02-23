#include "pch.h"
#include "command.h"

ccommand::ccommand(): threadid(0)
{
	struct {
		int ncmd;
		cmdfunc func;
	} data[] = {
		{1, &ccommand::makedriverinfo},
		{2, &ccommand::makedirectoryinfo},
		{3, &ccommand::runfile},
		{4, &ccommand::downloadfile},
		{5, &ccommand::deletelocalfile},
		{6, &ccommand::sendscreen},
		{7, &ccommand::mousevent},
		{8, &ccommand::lockmachine},
		{9, &ccommand::unlockmachine},
		{505, &ccommand::testconnect},
		{-1, 0},
	};

	for (int i = 0; i < data[i].ncmd != -1; i++)
		m_mpfunction[data[i].ncmd] = data[i].func;
}

int ccommand::execommand(int ncmd, list<cpacket>& lstpacket, cpacket& inpacket)
{
	if (m_mpfunction.find(ncmd) == m_mpfunction.end()) return -1;
	return (this->*m_mpfunction[ncmd])(lstpacket, inpacket);
}

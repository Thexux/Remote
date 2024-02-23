#pragma once
#include"common.h"

class ctool
{
public:
	static void dump(uchar* pdata, int nsize)
	{
		for (int i = 0; i < nsize; i++)
		{
			if (i && i % 16 == 0) puts("");
			printf("%02X ", pdata[i]);
		}
		puts("");
	}
};


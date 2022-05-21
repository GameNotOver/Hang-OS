#include "../_header_/apifunc.h"

void HariMain(void)
{
	static char s[9] = { 0xb2, 0xdb, 0xca, 0xc6, 0xce, 0xcd, 0xc4, 0x0a, 0x00 };
	// static char s[40] = "一二三四五六七八九十";
	api_putstr(s);
	api_end();
}

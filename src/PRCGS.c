#include "PRCGS.h"
#include "PRCGShdr.h"
#include "lowgraph.h"
#include "drawHLine.h"
#include <stdio.h>
#include <string.h>

#define DEBUG_WRITE_HDR_INFO

static PRCGSHeader hdr = { 0 };
static int width, height; // 読み取っておく
static char runLength[8]; // 0-6 ヘッダ+1 7:1で固定

int setPRCGSHeader(const unsigned char* headers[])
{
	memcpy(&hdr, headers, 0x80);

	// ヘッダのチェック
	if ((hdr.hdr[0] != 'P') || (hdr.hdr[1] != '_') || (hdr.hdr[2] != '3'))
	{
		printf("Error. Header is not P_3");

		return -1;
	}
	width = (hdr.width[0] << 8) | (hdr.width[1]);
	height = (hdr.height[0] << 8) | (hdr.height[1]);

	for (int i = 0; i < 7; ++i)
	{
		runLength[i] = hdr.length[i] + 1;
	}
	runLength[7] = 1; // 固定

#if (defined(DEBUG_WRITE_HDR_INFO))
	printf("CreateSoftVer %c%c\n",hdr.ver[0],hdr.ver[1]);
	printf("CreateData Machine by ");
	for (int i = 0; i < 16; ++i)
	{
		printf("%c", hdr.datMachine[i]);
	}
	printf("\n");

	printf("CreateSoft by ");
	for (int i = 0; i < 8; ++i)
	{
		printf("%c", hdr.appAuthor[i]);
	}
	printf("\n");

	printf("CreateData by ");
	for (int i = 0; i < 8; ++i)
	{
		printf("%c", hdr.datAuthor[i]);
	}
	printf("\n");

	printf("Data Created at ");
	for (int i = 0; i < 8; ++i)
	{
		printf("%c", hdr.createYMD[i]);
	}
	printf(" ");
	for (int i = 0; i < 8; ++i)
	{
		printf("%c", hdr.createHMS[i]);
	}
	printf("\n");
	printf("Data Size %dx%d\n", width, height);
	printf(hdr.arc ? "Compressed\n" : "UnCompressed\n");
	
	printf("RunLength Data ");
	for (int i = 0; i < 8; ++i)
	{
		printf("%d ", hdr.length[i]);
	}
	printf("\n");
	printf(hdr.mono ? "MonoChrome\n" : "Color\n");

#endif

	if ((width > 320) || (height < 200))
	{
		printf("over 320x200 data can't view!");
		return 1;
	}
	if (hdr.arc == 0)
	{
		printf("Can't view uncompressed data!");
		return 1;
	}

	return 0;
}

int addPRCGSData(const unsigned char dat)
{
	return 1; // dummy
}
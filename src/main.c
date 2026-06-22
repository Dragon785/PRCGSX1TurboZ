#include <conio.h>
#include <stdio.h>
#include "PRCGS.h"

#include "lowgraph.h"

#define BUFSIZE 2048
// データ読み込み用バッファ
static char databuf[BUFSIZE];

void sendSubCPU(unsigned char data)
{
	while (inp(0x1a01) & 0x40);
	outp(0x1900, data);
	while (inp(0x1a01) & 0x40);
}


unsigned char readSubCPU(void)
{
	while (inp(0x1a01) & 0x20);
	return (inp(0x1900));
}

void getTime(char* hour, char* minute, char* second)
{
	sendSubCPU(0xef);

	unsigned char hourBCD = readSubCPU();
	unsigned char minBCD = readSubCPU();
	unsigned char secBCD = readSubCPU();

	*hour = (hourBCD >> 4) * 10 + (hourBCD & 7);
	*minute = (minBCD >> 4) * 10 + (minBCD & 7);
	*second = (secBCD >> 4) * 10 + (secBCD & 7);
}

int main(int argc,char* argv[])
{
	clrscr();
	if (argc != 2)
	{
		printf("Usage: look PRCFile\n");
		return -1;
	}
	FILE* f = fopen(argv[1], "rb");
	if (!f)
	{
		printf("File %s not found.\n", argv[1]);

		return -1;
	}

	char s_h=0, s_m=0, s_s=0;
	getTime(&s_h, &s_m, &s_s);

	char hdrbuf[128];
	if (fread(hdrbuf, 1, 128, f) == 128)
	{
		if (setPRCGSHeader(hdrbuf) == 0)
		{
			dispHeaderData();
			int finished = 0;
			do
			{
				// データ読み取り
				int c = fread(databuf, 1, BUFSIZE, f);
				if (c > 0)
				{
					for (int i = 0; i < c; ++i)
					{
						if (finished=addPRCGSData(databuf[i]))
						{
							continue;
						}
					}
				}
				else
				{
					finished = 1;
				}
			} while (!finished);
		}
	}
	else
	{
		printf("Illegal File Size!");
	}

	fclose(f);

	char e_h=0, e_m=0, e_s=0;
	getTime(&e_h, &e_m, &e_s);

	clrscr();

	int diff = e_s - s_s;
	diff = diff + (e_m - s_m) * 60;
	diff = diff + (e_h - s_h) * 3600;
	

	printf("total %d sec (%d:%d:%d)-(%d:%d:%d) \n", diff,s_h,s_m,s_s,e_h,e_m,e_s);

	int dmy = getch();

	clearGRAM();

	dmy = getch();

	return 0;
}
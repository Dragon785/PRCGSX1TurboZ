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

void getTime(int* hour, int* minute, int* second)
{
	sendSubCPU(0xef);

	unsigned char hourBCD = readSubCPU();
	unsigned char minBCD = readSubCPU();
	unsigned char secBCD = readSubCPU();

	*hour = (hourBCD >> 4) * 10 + (hourBCD & 7);
	*minute = (minBCD >> 4) * 10 + (minBCD & 7);
	*second = (secBCD >> 4) * 10 + (secBCD & 7);
}

long getSec(void)
{
	unsigned int hour, minute, second;
	getTime(&hour, &minute, &second);

	long ret = hour;
	ret = ret * 60 + minute;
	ret = ret * 60 + second;

	return ret;
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

	long startTime = getSec();

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

	long endTime = getSec();

	clrscr();

	long diff = (endTime - startTime);
	

	printf("total %ld \n", diff);

	int dmy = getch();

	clearGRAM();

	dmy = getch();

	return 0;
}
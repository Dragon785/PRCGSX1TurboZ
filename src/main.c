#include <conio.h>
#include <stdio.h>
#include "PRCGS.h"

#include "lowgraph.h"

#define BUFSIZE 512
// データ読み込み用バッファ
static char databuf[BUFSIZE];

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

	char hdrbuf[128];
	if (fread(hdrbuf, 1, 128, f) == 128)
	{
		if (setPRCGSHeader(hdrbuf) == 0)
		{
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
			} while (!finished);
		}
	}
	else
	{
		printf("Illegal File Size!");
	}

	fclose(f);

	printf("ANY KEY");
	int dmy = getch();

	clearGRAM();

	return 0;
}
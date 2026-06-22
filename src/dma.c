// https://kyouichisato.blogspot.com/2014/09/bds-c-x1-turbo-dma.htmlを元にリライト

#include <stdlib.h>

#define DMA_ADDR 0x1f80

void resetDMA(void)
{
	for (unsigned char a = 0; a <6; a--)
	{
		outp(DMA_ADDR, 0xc0);
	}
}


void enableDMA(void)
{
	outp(DMA_ADDR, 0x87);
}

void setDMAData(unsigned char cmds[], unsigned char size)
{
	unsigned char* p = cmds;
	for (unsigned char count = 0; count < size; ++count)
	{
		outp(DMA_ADDR, *p++);
	}
}

// status byte
// 7:X
// 6:X
// 5:end of block 0
// 4:match found 0
// 3:interrupt pending 0
// 2:X
// 1:ready active 0
// 0:transfer has occurred 1

static const unsigned char readStatonlyCmd[2] =
{
	0b10111011, // 0xbb read mask cmd
	0b00000001  // status byte only
                // pb addr h/l pa addr h/l byte counter h/l stat
};

static unsigned char xferM2GRAMCmd[13] =
{
	0x7d, // 0111 1101    WR0
		  //       +----- 0:B->A 1:A->B
	0x00, // src lower (portA)
	0x00, // src upper
	0x00, // size lower
	0x00, // size upper
	0x14, // 0001 0100    WR1 PORT A 
		  //   || +------ 0:メモリー 1:I/O 
		  //   ++-------- 00:-- 01:++ 10/11:固定
	0xcd, // 1100 1101    WR4 
		  //  ++--------- 00:バイト 01:コンティニュアス 10:バースト */
	0x00, // dst lower (portB)
	0x00, // dst upper 
	0x18, // 0001 1000    WR2 PORT B 
		  //   || +------ 0:メモリー 1:I/O 
		  //   ++-------- 00:-- 01:++ 10/11:固定 
	0x9a, // 1001 1010    WR5 READYはH有効 
		  //    | +------ 0:READY L 1:READY H 
		  //    +-------- 0:CE 1:CE/WAIT 
	0xcf, // WR6 load
	0xb3  // WR6 force ready
};

void xferM2GRAM(unsigned short src, unsigned short dst, unsigned short size)
{
	size--;
	xferM2GRAMCmd[1] = src & 0xff;
	xferM2GRAMCmd[2] = (src >> 8);
	xferM2GRAMCmd[3] = (size & 0xff);
	xferM2GRAMCmd[4] = (size >> 8);
	xferM2GRAMCmd[7] = (dst & 0xff);
	xferM2GRAMCmd[8] = (dst >> 8);

	setDMAData(xferM2GRAMCmd, 13);
}

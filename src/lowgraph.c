// lowlevel graphic control

#include "lowgraph.h"
#include <stdlib.h>

static const unsigned char CRTC_24K[14] =
{
		0x6b, // R0
		0x50, // R1
		0x59, // R2
		0x88, // R3
		0x1b, // R4
		0x00, // R5
		0x19, // R6
		0x1a, // R7
		0x00, // R8
		0x0f, // R9
		0x00, // R10
		0x00, // R11
		0x00, // R12
		0x00  // R13
};

static const unsigned char CRTC_15K[14] =
{
		55,
		40,
		45,
		52,
		31,
		2,
		25,
		28,
		0,
		7,
		0,
		0,
		0,
		0,
};

void setScreen(int is80, int hires)
{
	unsigned char o = inp(0x1fd0);
	if (hires)
	{
		o |= 0x41;
	}
	else
	{
		o = 0x00;
	}
	outp(0x1fd0, o);

	const unsigned char* crtc = (hires) ? CRTC_24K : CRTC_15K;
	for (int i = 0; i < 14; ++i)
	{
		outp(0x1800, i);
		outp(0x1801, crtc[i]);
	}

	unsigned char w80 = inp(0x1a02);
	if (is80)
	{
		w80 &= 0xb0;
	}
	else
	{
		w80 |= 0x40;
	}

	outp(0x1a02, w80);
}

void clearText(void)
{
	for (int i = 0; i < 80 * 25; ++i)
	{
		outp(0x3000 + i, 0x20);
		outp(0x3800 + i, 0);
	}
}

void initDigitalPalette(void)
{
	outp(0x1000, 0xaa); // blue digital palette
	outp(0x1100, 0xcc); // red digital palette
	outp(0x1200, 0xf0); // green digital palette
}

void setDispGRAM(unsigned char bank)
{
	unsigned char t = inp(0x1fd0);
	t = t & 0xf7 | (bank ? 0x8 : 0x0);
	outp(0x1fd0, t);
}

void setWriteGRAM(unsigned char bank)
{
	unsigned char t = inp(0x1fd0);
	t = t & 0xef | (bank ? 0x10 : 0x0);
	outp(0x1fd0, t);
}

void clearGRAM(void)
{
	for (int bank = 0; bank < 2; ++bank)
	{
		setWriteGRAM(bank);
		outp(0x1a03, (5 << 1) | 1); // port c bit 5 on
		outp(0x1a03, (5 << 1) | 0); // port c bit 5 off enable multiplane write
		for (int i = 0; i < 0x4000; ++i)
		{
			outp(i, 0);
		}
		volatile unsigned char unlock = inp(0x4000); // disable multiplane write
	}
}

void set4096Mode(void)
{
	// set 320*200 mode
	setScreen(0, 0);
	clearGRAM();
	outp(0x1fb0, 0x80);
}

// COPY GRAM Plane(Bank 0/1)
void copyPlane(unsigned short srcAddr, unsigned short dstAddr)
{
	for (unsigned char bank = 0; bank < 2; ++bank)
	{
		setWriteGRAM(bank);
		for (int i = 0; i < 0x4000; ++i)
		{
			outp(dstAddr+i, inp(srcAddr+i));
		}
	}
}
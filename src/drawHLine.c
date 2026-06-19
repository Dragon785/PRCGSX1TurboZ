#include "drawHLine.h"
#include "lowgraph.h"

// 内部ワーク
// 次に描画する座標(320,200で最後)
static int nextX = 0, nextY = 0;
// 書き込み基準ベース
static unsigned int planeBase = 0;

// 階調毎のアドレスオフセットとバンクのオフセットテーブル
typedef struct tag_GradMem
{
	unsigned int addroffset;
	unsigned int bank;
} GradMem;

// 各階調のオフセットアドレス/バンク
static const GradMem GRADMEMTABLE[4] =
{
	{0x400,0},
	{0x000,0},
	{0x400,1},
	{0x000,1}
};

// ８ドット未満の塗りつぶしを高速に行うためのテーブル
typedef struct tag_PaintTbl
{
	unsigned char writebit; // 塗るときのビット(塗らないときは0で固定)
	unsigned char mask; // マスク(~writebitで演算する方が速い？)
} PaintTbl;
// 1-7ドットまでの塗りつぶし用テーブル
// 1ドット(８種)
static const PaintTbl PaintTbl_1[8] =
{
	{0b10000000,0b01111111},
	{0b01000000,0b10111111},
	{0b00100000,0b11011111},
	{0b00010000,0b11101111},
	{0b00001000,0b11110111},
	{0b00000100,0b11111011},
	{0b00000010,0b11111101},
	{0b00000001,0b11111110}
};
// 2ドット（７種)
static const PaintTbl PaintTbl_2[7] =
{
	{0b11000000,0b00111111},
	{0b01100000,0b10011111},
	{0b00110000,0b11001111},
	{0b00011000,0b11100111},
	{0b00001100,0b11110011},
	{0b00000110,0b11111001},
	{0b00000011,0b11111100}
};
// 3ドット(6種)
static const PaintTbl PaintTbl_3[6] =
{
	{0b11100000,0b00011111},
	{0b01110000,0b10001111},
	{0b00111000,0b11000111},
	{0b00011100,0b11100011},
	{0b00001110,0b11110001},
	{0b00000111,0b11111000}
};
// 4ドット(5種)
static const PaintTbl PaintTbl_4[5] =
{
	{0b11110000,0b00001111},
	{0b01111000,0b10000111},
	{0b00111100,0b11000011},
	{0b00011110,0b11100001},
	{0b00001111,0b11110000}
};
// 5ドット(4種)
static const PaintTbl PaintTbl_5[4] =
{
	{0b11111000,0b00000111},
	{0b01111100,0b10000011},
	{0b00111110,0b11000001},
	{0b00011111,0b11100000}
};
// 6ドット(3種)
static const PaintTbl PaintTbl_6[3] =
{
	{0b11111100,0b00000011},
	{0b01111110,0b10000001},
	{0b00111111,0b11000000}
};
// 7ドット(2種)
static const PaintTbl PaintTbl_7[2] =
{
	{0b11111110,0b00000001},
	{0b01111111,0b10000000}
};

// (sx,y)-(ex,y)までgrad階調で横線を引く
static void drawHorizontalSub(int sx, int ex, int y, unsigned char grad)
{
}

void initDrawHLine(unsigned int baseAddr)
{
	planeBase = baseAddr;
	nextX = 0; nextY = 0;
}
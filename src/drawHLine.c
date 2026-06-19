#include "drawHLine.h"
#include "lowgraph.h"
#include <stdlib.h>
#include <stdio.h> // for debug

// 内部ワーク
// 描画したい画面サイズ
static unsigned int width, height;
// 次に描画する座標(320,200で最後)
static unsigned int nextX = 0, nextY = 0;
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
	{0x400,1},
	{0x000,1},
	{0x400,0},
	{0x000,0},
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

static const PaintTbl* PaintTbls[7] =
{
	PaintTbl_1,
	PaintTbl_2,
	PaintTbl_3,
	PaintTbl_4,
	PaintTbl_5,
	PaintTbl_6,
	PaintTbl_7
};

// (sx,y)-(ex,y)までgrad階調で横線を引く
// sx<=exが条件
// 将来的には高速化の際にこれを使わないようになりそう
static void drawHorizontalSub(int sx, int len, int y, unsigned char grad)
{
	// 書き込み相対アドレス計算
	unsigned int writeBaseAddr = (sx >> 3) + ((y & 7) << 11) + ((y >> 3) * 40);
	
	for (int plane = 0; plane < 4; ++plane) // ４プレーン毎に
	{
		int writeLen = len; // 横に引くドットの数

		// 階調と比べて塗るかクリアか判定
		int mustPaint = grad & (1 << plane);
		// 書き込みバンク切り替え,基準アドレス計算
		unsigned int writeAddr = writeBaseAddr + GRADMEMTABLE[plane].addroffset+planeBase;
		setWriteGRAM(GRADMEMTABLE[plane].bank);

		unsigned char leftPiece = sx & 7;
		if (leftPiece)
		{
			// 左端が8の倍数でないならその分をまとめて書く
			// 書き込む長さ計算(8-余りと全長の短い方)
			int toWriteLeft = 8 - leftPiece;
			if (toWriteLeft > writeLen)
			{
				toWriteLeft = writeLen;
			}
			PaintTbl* useTable = PaintTbls[toWriteLeft-1];
			// VRAM読んでマスク
			unsigned char writeData = inp(writeAddr) & useTable[leftPiece].mask;
			// 書き込む階調なら書き込みデータでOR
			if (mustPaint)
			{
				writeData |= useTable[leftPiece].writebit;
			}
			// VRAMに書き戻す
			outp(writeAddr++, writeData);
			writeLen -= toWriteLeft;
		}

		if (writeLen > 0)
		{
			int writeBytes = writeLen >> 3;
			for (int i = 0; i < writeBytes; ++i)
			{
				// まだ残りがあるなら8の倍数分１バイトずつまとめて書く
				// 書き込む階調なら0xff,書き込まないなら0x00
				outp(writeAddr++,(mustPaint) ?  (0xff) : (0x00));
			}
			writeLen -= writeBytes << 3;

			if (writeLen > 0)
			{
				// 余りがあるならその分描画(バイト内位置は0から固定)
				PaintTbl* useTable = PaintTbls[writeLen-1];
				// VRAM読んでマスク
				unsigned char writeData = inp(writeAddr) & useTable[0].mask;

				// 書き込む階調なら書き込みデータでOR
				if (mustPaint)
				{
					writeData |= useTable[0].writebit;
				}
				// VRAMに書き戻す
				outp(writeAddr++, writeData);
			}
		}
	}
}

void initDrawHLine(unsigned int baseAddr,unsigned int w,unsigned int h)
{
	planeBase = baseAddr;
	width = w; height = h;
	nextX = 0; nextY = 0;
}

int addHLine(unsigned char level, unsigned int length)
{
	if (nextY >= height)
	{
		return length; // このプレーンにはこれ以上書き込めない
	}

	while (length > 0)
	{
		unsigned int toWrite = length;

		// 横一杯まで
		if (toWrite > width - nextX)
		{
			toWrite = width - nextX;
		}

		drawHorizontalSub(nextX, toWrite, nextY, level);
		length -= toWrite;
		nextX += toWrite;
		if (nextX >= width)
		{
			// 次のラインへ
			nextX=0;
			nextY++;
			if (nextY >= height)
			{
				// プレーン書き出し終了
				if (length == 0)
				{
					// ちょうど終わった
					return -1;
				}
				else
				{
					// 次のプレーンでlength分描画
					return length;
				}
			}
		}
	}

	return 0; // 通常
}



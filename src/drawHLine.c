#include "drawHLine.h"
#include "lowgraph.h"
#include <stdlib.h>
#include <stdio.h> // for debug

// 内部ワーク
// 描画したい画面サイズ
static unsigned int width=0, height=0;
// 次に描画する座標(320,200で最後)
static unsigned int nextX = 0, nextY = 0;
// 次に描画するラインバッファ位置
static unsigned int writeBufAddr = 0;
// 次に描画するGRAMアドレス
static unsigned int writeBaseAddr = 0;
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

// RAM上で1ライン分処理する為のバッファ（4プレーン分)
static unsigned char LineBuffer[4][40];

// バッファからGRAMに書き込む
static void transferBufferToGRAM(unsigned int writeBaseAddr)
{
	unsigned int writePlaneBase = writeBaseAddr + planeBase;
	for (int plane = 0; plane < 4; ++plane)
	{
		unsigned int writeAddr = writePlaneBase + GRADMEMTABLE[plane].addroffset;
		unsigned char* readBuf = LineBuffer[plane];
		setWriteGRAM(GRADMEMTABLE[plane].bank);
		for (int i = 0; i < 40; ++i)
		{
			outp(writeAddr++, *readBuf++);
		}
	}
}

// addrからlenだけラインバッファに線を書く。開始位置も与える（位置計算の為)
// 戻り値は書き込み"完了"したバイト数
// 将来的には高速化の際にこれを使わないようになりそう
static int drawHorizontalSub(int sx, int len, unsigned int writeBaseAddr, unsigned char grad)
{
	unsigned char leftPiece = sx & 7;
	unsigned char leftMask=0xff, leftWrite=0x00;

	unsigned int completeWriteBytes = 0;
	if (leftPiece)
	{
		int toWriteLeft = 8 - leftPiece;
		completeWriteBytes = 1;
		if (toWriteLeft > len)
		{
			toWriteLeft = len;
			completeWriteBytes = 0; // 次この位置にまた描画するので
		}
		PaintTbl* useTable = PaintTbls[toWriteLeft - 1];
		leftMask = useTable[leftPiece].mask; leftWrite = useTable[leftPiece].writebit;
		len -= toWriteLeft;
	}
	int writeBytes = (len >> 3);
	completeWriteBytes += writeBytes;
	unsigned char rightPiece = (len & 7);
	unsigned char rightMask = 0xff; unsigned char rightWrite = 0x00;
	if (rightPiece)
	{
		PaintTbl* useTable = PaintTbls[rightPiece - 1];
		rightMask = useTable[0].mask; rightWrite = useTable[0].writebit;
		// 右端は次回描画対象なので処理に含めない
	}

	for (int plane = 0; plane < 4; ++plane) // ４プレーン毎に
	{
		unsigned char* writeLineBuf = LineBuffer[plane]+writeBaseAddr;
		// 階調と比べて塗るかクリアか判定
		int mustPaint = grad & (1 << plane);

		if (leftPiece)
		{
			// VRAM読んでマスク
			unsigned char writeData = (*writeLineBuf) & leftMask;
			// 書き込む階調なら書き込みデータでOR
			if (mustPaint)
			{
				writeData |= leftWrite;
			}
			// VRAMに書き戻す
			*(writeLineBuf++)= writeData;
		}
		unsigned char writeByteData = (mustPaint) ? (0xff) : (0x00);
		for (int i = 0; i < writeBytes; ++i)
		{
			// 8ドット単位で処理出来るところ
			*(writeLineBuf++)=writeByteData;
		}

		if (rightPiece)
		{
			unsigned char writeData = 0; // 右端は前のデータは潰していい
			if (mustPaint)
			{
				writeData |= rightWrite;
			}
			*(writeLineBuf) = writeData;
		}
	}

	return completeWriteBytes;
}

void initDrawHLine(unsigned int baseAddr,unsigned int w,unsigned int h)
{
	planeBase = baseAddr;
	width = w; height = h;
	nextX = 0; nextY = 0;
	writeBufAddr = 0;
	writeBaseAddr = 0;
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

		int writedBytes=drawHorizontalSub(nextX, toWrite, writeBufAddr, level);
		writeBufAddr += writedBytes;
		length -= toWrite;
		nextX += toWrite;
		if (nextX >= width)
		{
			// 1ラインバッファからGRAMに書き出す
			transferBufferToGRAM(writeBaseAddr);
			// 次のラインへ
			nextX=0;
			writeBufAddr = 0;
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
			// 新しい基準位置再計算(もっと高速化できるはず)
			writeBaseAddr = ((nextY & 7) << 11) + ((nextY >> 3) * 40);
		}
	}

	return 0; // 通常
}



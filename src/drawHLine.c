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

static const GradMem GRADMEMTABLE[4] =
{
	{0x400,0},
	{0x000,0},
	{0x400,1},
	{0x000,1}
};

// (sx,y)-(ex,y)までgrad階調で横線を引く
static void drawHorizontalSub(int sx, int ex, int y, unsigned char grad)
{
}

void initDrawHLine(unsigned in baseAddr)
{
	planeBase = BaseAddr;
	nextX = 0; nextY = 0;
}
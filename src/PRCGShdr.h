#if (!defined(__PRCGSHDR_H__))
#define __PRCGSHDR_H__

/* https://maaberu.web.fc2.com/prcgs.htmを参考にヘッダを記述 */

// #pragma align(1)
typedef struct tag_PRCGS
{
	unsigned char hdr[3]; // 'P_3'
	unsigned char ver[2]; // 作成ソフトバージョン
	unsigned char datMachine[16]; // データ作成マシン名
	unsigned char appAuthor[8]; // アプリ作成者コールサイン
	unsigned char datAuthor[8]; // データ作成者コールサイン
	unsigned char createYMD[8]; // データ作成年月日（テキスト)
	unsigned char createHMS[8]; // データ作成時分秒（テキスト）
	unsigned char width[2]; // 横ピクセル数（unsigned shortだがエンディアンがあるのでここでは2バイト管理)
	unsigned char height[2]; // 縦ピクセル数
	unsigned char arc; // 0:非圧縮（とりあえず見ない) 1:圧縮
	unsigned char length[7]; // 長さテーブル。実際の長さ-1が入る
	unsigned char mono; // 1:モノクロ
	unsigned char reserved[62];
} PRCGSHeader /* __attribute__((__packed__)) */;

#endif

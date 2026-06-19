#if (!defined(__PRCGS_H__))
#define __PRCGS_H__

/// @param headers ヘッダの入っている配列
/// @return 0:ヘッダ読み込み完了 1:ヘッダおかしい
extern int setPRCGSHeader(const unsigned char* headers[]);

/// @param dat 次のバイト
/// @return 0:データ終わってない 1:データ完了
extern int addPRCGSData(const unsigned char dat);

#endif

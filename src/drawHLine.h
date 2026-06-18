// 横線を引くだけのルーチン。将来的に高速化を見越してソースを分けておく
// (0,0)スタートで指定した長さで指定した階調で描画、320ドットで自動折り返し
#if (!defined(__DRAWHLINE_H__))
#define __DRAWHLINE_H__

// ベースアドレスをセットして描画位置を(0,0)にする
extern void initDrawHLine(unsigned int baseAddr);

// ベースアドレスから横方向に指定階調,指定長さ描画
// 横方向にはみ出る場合は一段下に折り返し
// プレーン境界をはみ出した場合はそこで描画を止めて残数を返す。残数が0の場合は-1を返す
// それ以外の時は戻り値0
extern int addHLine(unsigned char level,unsigned char length)

#endif

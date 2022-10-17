/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __C_LIST_H__
#define __C_LIST_H__

/**
 * @file c_list.h
 * @brief C言語用双方向リストライブラリ
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#if !defined(C_LIST_ENB_DBGPR)
#define CLIST_DBGPR(fmt, args...)
#else 
#define CLIST_DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__,## args); }
#endif
#define CLIST_ERRPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__,## args); }

#if defined(__CLIST_TEST_CHECK_ENB)
static FILE *fp = NULL;
#define CLIST_TEST_CHECK(nn) \
	{ \
		static bool checked = false; \
		if (fp == NULL) fp = fopen (".test_result_clist.txt", "w+"); \
		if (checked == false) { \
			if (fp != NULL) { \
				fprintf (fp, "%d:  %s() %s\n", __LINE__, __FUNCTION__,nn); \
				checked = true; \
			} \
		} \
	}
#define CLIST_TEST_CHECK_END() \
	{ \
		if (fp != NULL) { \
			fclose(fp); \
			fp = NULL; \
		} \
		system ("sort -o test_result_clist.txt .test_result_clist.txt"); \
		system ("rm -f .test_result_clist.txt"); \
		printf ("##### test result file: test_result_clist.txt created\n"); \
}
#else
#define CLIST_TEST_CHECK(nn)
#define CLIST_TEST_CHECK_END()
#endif

// ============================================================================================ マクロ定義等

/// CLIST系関数の戻り値用定義
typedef enum {
	CLIST_RET_OK,			//!< 正常
	CLIST_RET_NG,			//!< その他エラー
	CLIST_RET_NG_ARGERR,	//!< 引数エラー
	CLIST_RET_NG_MEMERR,	//!< メモリーエラー
} CLIST_RET;



// ============================================================================================ CLIST_DATAUTY (CLISTデータユーティリティ構造体)
/**
 * @brief  CLISTユーザー定義関数指定構造体
 * @details　ユーザーは、ユーザーのソースの中でまずユーザーデータの型を定義、続いてそれを処理する関数群を実装し、本構造体に関数のポインタを代入する。
 * @details　create/destroy は実装必須だがその他は任意であり、必要ないときはNULLを指定する。
 */
typedef struct _CLIST_DATAUTY {
	/**
	 * @brief  ユーザーデータをmalloc、初期化する関数を指定（指定必須）
	 * @param  param ユーザーデータ生成時のパラメーターデータ
	 * @return 成功時は生成したユーザーデータのポインタを返す、失敗時はNULLを返す
	 */
	void *(*create)(void *param);

	/**
	 * @brief  ユーザーデータをfreeする関数を指定（指定必須）
	 * @param  datap ユーザーデータのポインタ
	 */
	void (*destroy)(void *datap);

	/**
	 * @brief  ユーザーデータ1とユーザーデータ2を比較する関数を指定（必要ないときはNULLを指定）
	 * @param  datap1,datap2 ユーザーデータ１と２のポインタ
	 * @return 比較結果を返す、ユーザーデータ1が2より小さい場合は -1、 等しい場合は 0、大きい場合は 1
	 */
	int (*compare)(void *datap1,void *datap2);

	/**
	 * @brief  ユーザーデータ1とユーザーデータ2を加算(ユーザーデータは結果で上書き)する関数を指定（必要ないときはNULLを指定）
	 * @param  datap1,datap2 ユーザーデータ１と２のポインタ
	 * @return ユーザーデータ1（演算結果が入っている）を返す
	 */
	void *(*add)(void *datap1,void *datap2);

	/**
	 * @brief  ユーザーデータ1とユーザーデータ2を減算(ユーザーデータは結果で上書き)する関数を指定（必要ないときはNULLを指定）
	 * @param  datap1,datap2 ユーザーデータ１と２のポインタ
	 * @return ユーザーデータ1（演算結果が入っている）を返す
	 */
	void *(*sub)(void *datap1,void *datap2);

	/**
	 * @brief  ユーザーデータ1とユーザーデータ2を乗算(ユーザーデータは結果で上書き)する関数を指定（必要ないときはNULLを指定）
	 * @param  datap1,datap2 ユーザーデータ１と２のポインタ
	 * @return ユーザーデータ1（演算結果が入っている）を返す
	 */
	void *(*mul)(void *datap1,void *datap2);

	/**
	 * @brief  ユーザーデータ1とユーザーデータ2を除算(ユーザーデータは結果で上書き)する関数を指定（必要ないときはNULLを指定）
	 * @param  datap1,datap2 ユーザーデータ１と２のポインタ
	 * @return ユーザーデータ1（演算結果が入っている）を返す
	 */
	void *(*div)(void *datap1,void *datap2);

	/**
	 * @brief  ユーザーデータが keydata の条件にー致するかどうかを判定する関数を指定（必要ないときはNULLを指定）
	 * @param  datap ユーザーデータのポインタ
	 * @param  keydata 一致条件のデータ
	 * @return 比較結果、条件一致すれば true が返る
	 */
	bool/*check_result*/ (*keymatch_chk)(void *datap, void *keydata);

	/**
	 * @brief  デバック用、ユーザーデータを見やすい形で表示(dump)する関数を指定（必要ないときはNULLを指定）
	 * @param  datap ユーザーデータのポインタ
	 * @param  param ユーザーデータ生成時のパラメーターデータ
	 */
	void (*dump)(void *userdata);

} CLIST_DATAUTY;


// ============================================================================================ CLIST_CONTAINER (CLISTコンテナ構造体)
/**
 * @brief  CLISTコンテナ構造体
 * @details　CLISTが保持するコンテナの構造体、双方向リストの前後リンクと、ユーザーのデータへのポインタのみで構成される
 * @details　生成、開放はユーティリティ関数を使用して行う。
 * @details　CLIST_CONTAINER型に typedef されているので、通常はそちらを使用する。
 */
typedef struct _CLIST_CONTAINER {

	// public
	struct _CLIST_CONTAINER *nextp;											//!< 双方向リストの前方リンク
	struct _CLIST_CONTAINER *prevp;											//!< 双方向リストの後方リンク
	void *datap;															//!< ユーザーデータのポインタ

} CLIST_CONTAINER;

// utility function
/**
 * @brief  CLISTコンテナを生成(malloc)する (CLISTコンテナ　ユーティリティ関数)
 * @param  dutyp ユーザーが定義した、各種データ処理関数のポインタを指定する、create/destroy 以外はNULL可
 * @param  create_param dutyp.create関数呼び出し時に引数として渡される
 * @return 生成したCLISTコンテナのポインタ
 * @details　CLISTコンテナをmalloc、dutyp.createでユーザーデータ領域を確保してコンテナにリンクする
 * @details　CLISTコンテナから間接的に実行するため、通常ユーザーは直接使用しない
 */
extern struct _CLIST_CONTAINER *clist_container_create (CLIST_DATAUTY *dutyp, void *create_param);
/**
 * @brief  CLISTコンテナを開放(free)する (CLISTコンテナ　ユーティリティ関数)
 * @param  containerp 開放するCLISTコンテナをポインタで指定
 * @details　dutyp.destroyでユーザーデータ領域を開放後、CLISTコンテナをfree
 * @details　CLISTコンテナから間接的に実行するため、通常ユーザーは直接使用しない
 */
extern void clist_container_destroy (struct _CLIST_CONTAINER *containerp, CLIST_DATAUTY *dutyp);



// ============================================================================================ CLIST 構造体
/**
 * @brief  CLIST構造体
 * @details　CLIST型に typedef されているので、通常はそちらを使用する。
 */
typedef struct _CLIST {
	// private
	CLIST_CONTAINER *container_begin;	//!< 保持するコンテナの先頭を指すポインタ
	CLIST_CONTAINER *container_end;		//!< 保持するコンテナの終端を指すポインタ
	int container_count;				//!< 保持するコンテナの総数
	CLIST_DATAUTY *dutyp;				//!< ユーザーが定義した、各種データ処理関数のポインタを保持する

	// public methods pointer
	/**
	 * @brief  新規コンテナ生成関数、オプション指定でリスト終端に追加（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  create_param コンテナ作成時の引数
	 * @param  do_add_last trueを渡すとCLISTのリスト終端に生成したコンテナを追加する
	 * @return 生成したCLISTコンテナのポインタ
	 */
	CLIST_CONTAINER *(*container_create) (struct _CLIST *clistp, void *create_param, bool do_add_last);
	/**
	 * @brief  コンテナ開放関数（対象は指定のコンテナ１つのみ）、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_destroy) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	/**
	 * @brief  全コンテナ開放関数（対象はCLISTが保持するすべて）、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_destroy_all) (struct _CLIST *clistp);
	/**
	 * @brief  指定のコンテナをリスト先頭につなぐ関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_add_top) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	/**
	 * @brief  指定のコンテナをリスト終端につなぐ関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_add_last) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	/**
	 * @brief  指定のコンテナをリストから外す関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_remove) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	/**
	 * @brief  指定のコンテナをリスト前方に１つ移動する関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_up) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	/**
	 * @brief  指定のコンテナをリスト後方に１つ移動する関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_down) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	/**
	 * @brief  リストをソートする関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_sort) (struct _CLIST *clistp);
	/**
	 * @brief  デバック用、指定のコンテナを見やすい形で表示(dump)する関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @param  containerp 対象コンテナをポインタで指定
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*container_dump) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	/**
	 * @brief  デバック用、保持しているすべてのコンテナを見やすい形で表示(dump)する関数、（CLISTメソッド）
	 * @param  clistp CLISTのポインタ
	 * @return 実行結果をCLIST_RET型で返す
	 */
	CLIST_RET (*dump_all) (struct _CLIST *clistp);

} CLIST;

// ------------------------------------------------------------------------------------------
// public methods extern
extern CLIST_CONTAINER *_clist_container_create (CLIST *clistp, void *create_param, bool do_add_last);		//!< 新規コンテナ生成関数、オプション指定でリスト終端に追加（CLISTメソッド）
extern CLIST_RET _clist_container_destroy (CLIST *clistp, CLIST_CONTAINER *containerp);						//!< コンテナ開放関数（対象は指定のコンテナ１つのみ）、（CLISTメソッド）
extern CLIST_RET _clist_container_destroy_all (CLIST *clistp);												//!< 全コンテナ開放関数（対象はCLISTが保持するすべて）、（CLISTメソッド）

extern CLIST_RET _clist_container_add_top (CLIST *clistp, CLIST_CONTAINER *containerp);						//!< 指定のコンテナをリスト先頭につなぐ関数、（CLISTメソッド）
extern CLIST_RET _clist_container_add_last (CLIST *clistp, CLIST_CONTAINER *containerp);					//!< 指定のコンテナをリスト終端につなぐ関数、（CLISTメソッド）
extern CLIST_RET _clist_container_remove (CLIST *clistp, CLIST_CONTAINER *containerp);						//!< 指定のコンテナをリストから外す関数、（CLISTメソッド）
extern CLIST_RET _clist_container_up (CLIST *clistp, CLIST_CONTAINER *containerp);							//!< 指定のコンテナをリスト前方に１つ移動する関数、（CLISTメソッド）
extern CLIST_RET _clist_container_down (CLIST *clistp, CLIST_CONTAINER *containerp);						//!< 指定のコンテナをリスト後方に１つ移動する関数、（CLISTメソッド）
extern CLIST_RET _clist_container_sort (CLIST *clistp);														//!< リストをソートする関数、（CLISTメソッド）
extern CLIST_RET _clist_container_dump (CLIST *clistp, CLIST_CONTAINER *containerp);						//!< デバック用、指定のコンテナを見やすい形で表示(dump)する関数、（CLISTメソッド）
extern CLIST_RET _clist_dump_all (CLIST *clistp);															//!< デバック用、保持しているすべてのコンテナを見やすい形で表示(dump)する関数、（CLISTメソッド）

// ------------------------------------------------------------------------------------------
// utility function
/**
 * @brief  CLISTを生成する (CLIST　ユーティリティ関数)
 * @param  dutyp ユーザーが定義した、各種データ処理関数のポインタを指定する、create/destroy 以外はNULL可
 * @return 生成したCLISTのポインタ
 * @details CLISTを生成(malloc)、初期化する。
 */
extern CLIST *clist_create (CLIST_DATAUTY *dutyp);
/**
 * @brief  CLISTを開放する (CLIST　ユーティリティ関数)
 * @param 開放するCLISTをポインタで指定する。
 * @return 実行の成否
 * @details 保持しているコンテナをすべて解放後、CLISTを開放(free)する。
 */
extern CLIST_RET clist_destroy (CLIST *clistp);

// ============================================================================================ CLIST_REFERER (CLISTリファラ構造体)
/**
 * @brief  CLISTリファラー構造体
 * @details CLISTリファラーは、データ及びコンテナに用意にアクセスするための手段を提供する
 * @details 基本的に、CLISTが保持するコンテナを参照する際にはこのCLISTリファラーを使用する。直接リストをユーザーが参照・操作することは推奨されない
 * @details CLIST_REFERER型に typedef されているので、通常はそちらを使用する。
 */
typedef struct _CLIST_REFERER {

	CLIST *clistp;					//!< 対象のCLISTを保持
	CLIST_CONTAINER *current;		//!< リファラが現在参照しているコンテナのポインタ
	
	void *(*data)(struct _CLIST_REFERER *crefp, int idx);					//!< 参照先をidx番目のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_begin)(struct _CLIST_REFERER *crefp);						//!< 参照先を先頭のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_end)(struct _CLIST_REFERER *crefp);						//!< 参照先を終端のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_prev)(struct _CLIST_REFERER *crefp);						//!< 参照先を前のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_next)(struct _CLIST_REFERER *crefp);						//!< 参照先を次のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	int (*count)(struct _CLIST_REFERER *crefp);								//!< コンテナの総数を返す関数、（CLISTリファラメソッド）

	void *(*data_current)(struct _CLIST_REFERER *crefp);					//!< 参照先は移動せず、現在位置のユーザーデータのポインタを返す関数、（CLISTリファラメソッド）

	void *(*data_find)(struct _CLIST_REFERER *crefp, int idx, void *key);	//!< 指定したキー条件で、参照先をidx番目のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_find_begin)(struct _CLIST_REFERER *crefp, void *key);		//!< 指定したキー条件で、参照先を先頭のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_find_end)(struct _CLIST_REFERER *crefp, void *key);		//!< 指定したキー条件で、参照先を終端のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_find_prev)(struct _CLIST_REFERER *crefp, void *key);		//!< 指定したキー条件で、参照先を前のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	void *(*data_find_next)(struct _CLIST_REFERER *crefp, void *key);		//!< 指定したキー条件で、参照先を次のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
	int (*find_count)(struct _CLIST_REFERER *crefp, void *key);				//!< 指定したキー条件で、コンテナの総数を返す関数、（CLISTリファラメソッド）

//contains	指定したキーの要素が含まれているかを判定する	C++20
//equal_range	指定したキーの範囲を取得	C++11	

} CLIST_REFERER;

// ------------------------------------------------------------------------------------------
// public methods extern
extern void *_clist_referer_data(struct _CLIST_REFERER *crefp, int idx);					//!< 参照先をidx番目のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_begin(struct _CLIST_REFERER *crefp);						//!< 参照先を先頭のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_end(struct _CLIST_REFERER *crefp);							//!< 参照先を終端のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_prev(struct _CLIST_REFERER *crefp);						//!< 参照先を１つ前のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_next(struct _CLIST_REFERER *crefp);						//!< 参照先を１つ次のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern int _clist_referer_count(struct _CLIST_REFERER *crefp);								//!< コンテナの総数を返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）

extern void *_clist_referer_data_current(struct _CLIST_REFERER *crefp);						//!< 現在の参照先のユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）

extern void *_clist_referer_data_find(struct _CLIST_REFERER *crefp, int idx, void *key);	//!< 指定したキー条件で、参照先をidx番目のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_find_begin(struct _CLIST_REFERER *crefp, void *key);		//!< 指定したキー条件で、参照先を先頭のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_find_end(struct _CLIST_REFERER *crefp, void *key);			//!< 指定したキー条件で、参照先を終端のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_find_prev(struct _CLIST_REFERER *crefp, void *key);		//!< 指定したキー条件で、参照先を１つ前のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern void *_clist_referer_data_find_next(struct _CLIST_REFERER *crefp, void *key);		//!< 指定したキー条件で、参照先を１つ次のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）
extern int _clist_referer_find_count(struct _CLIST_REFERER *crefp, void *key);				//!< 指定したキー条件で、コンテナの総数を返す関数、（CLISTリファラメソッド、必ずCLIST_REFERER経由で呼び出す）

// ------------------------------------------------------------------------------------------
// utility function
/**
 * @brief  CLISTリファラーを生成する (CLISTリファラー　ユーティリティ関数)
 * @param  clistp 対象のCLISTのポインタを指定
 * @return 生成したCLISTリファラのポインタ
 * @details CLISTリファラを生成(malloc)、初期化する。
 */
extern CLIST_REFERER *clist_referer_create (CLIST *clistp);
/**
 * @brief  CLISTリファラーを開放する (CLISTリファラー　ユーティリティ関数)
 * @param  crefp 対象のCLISTリファラをポインタで指定
 * @return 実行の成否
 * @details CLISTリファラを開放(free)する。
 */
extern CLIST_RET clist_referer_destroy (struct _CLIST_REFERER *crefp);
	
#ifdef __CLIST_C__

// ============================================================================================ CLIST_CONTAINER (CLISTコンテナ構造体)　　関数群

// ------------------------------------------------------------------------------------------
// utility function

// ------------------------------------------------------------------------------------------
// CLISTコンテナを生成(malloc)する (CLISTコンテナ　ユーティリティ関数)
struct _CLIST_CONTAINER *clist_container_create (CLIST_DATAUTY *dutyp, void *create_param)
{
	if (dutyp == NULL || dutyp->create == NULL) {
		CLIST_ERRPR("clist_container_create: arg error");
		return NULL;
	}
	CLIST_CONTAINER *containerp = (CLIST_CONTAINER*)malloc (sizeof(CLIST_CONTAINER));			// create container
	if (containerp == NULL) {
		CLIST_ERRPR("clist_container_create: container malloc error");
		return NULL;
	}
	memset (containerp, 0, sizeof(CLIST_CONTAINER));		// container zero clear
	containerp->datap = dutyp->create (create_param);		// data create and initialize
	if (containerp->datap == NULL) {
		CLIST_ERRPR("clist_container_create: data malloc error");
		free (containerp);
		return NULL;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return (NULL);
}

// ------------------------------------------------------------------------------------------
// CLISTコンテナを開放(free)する (CLISTコンテナ　ユーティリティ関数)
void clist_container_destroy (struct _CLIST_CONTAINER *containerp, CLIST_DATAUTY *dutyp)
{
	if (containerp == NULL || dutyp == NULL || dutyp->destroy == NULL) {
		CLIST_ERRPR("clist_container_destroy: arg error");
		return;
	}
	dutyp->destroy(containerp->datap);
	free (containerp);

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
}

// ============================================================================================ CLIST 構造体　　関数群

// ------------------------------------------------------------------------------------------
// public methods extern

// ------------------------------------------------------------------------------------------
// 新規コンテナ生成関数、オプション指定でリスト終端に追加（CLISTメソッド）
CLIST_CONTAINER *_clist_container_create (CLIST *clistp, void *create_param, bool do_add_last)
{
	if (clistp == NULL) {
		CLIST_ERRPR("_clist_container_create: clistp is NULL");
		return NULL;
	}
	CLIST_CONTAINER *containerp = clist_container_create (clistp->dutyp, create_param);
	if (containerp != NULL && do_add_last) {
		clistp->container_add_last (clistp, containerp);
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return containerp;
}

// ------------------------------------------------------------------------------------------
// コンテナ開放関数（対象は指定のコンテナ１つのみ）、（CLISTメソッド）
CLIST_RET _clist_container_destroy (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("_clist_container_destroy: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	_clist_container_remove (clistp, containerp);
	clist_container_destroy (containerp, clistp->dutyp);

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// 全コンテナ開放関数（対象はCLISTが保持するすべて）、（CLISTメソッド）
CLIST_RET _clist_container_destroy_all (CLIST *clistp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("_clist_container_destroy_all: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	CLIST_CONTAINER *ccnextp, *ccp = clistp->container_begin;
	while (ccp) {
		ccnextp = ccp->nextp;
		_clist_container_remove (clistp, ccp);
		clist_container_destroy (ccp, clistp->dutyp);
		ccp = ccnextp;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// 指定のコンテナをリスト先頭につなぐ関数、（CLISTメソッド）
CLIST_RET _clist_container_add_top (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	if (clistp == NULL || containerp == NULL) {
		CLIST_ERRPR("_clist_container_add_top: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	if (clistp->container_begin == NULL) {
		// no container -> one container
		clistp->container_begin   = containerp;
		clistp->container_end     = containerp;
		containerp->nextp = NULL;
		containerp->prevp = NULL;
	} else if (clistp->container_begin == clistp->container_end) {
		// one container -> two containers
		clistp->container_begin        = containerp;
		clistp->container_begin->prevp = NULL;
		clistp->container_begin->nextp = clistp->container_end;
		clistp->container_end->prevp   = clistp->container_begin;
	} else {
		// 2> containers
		clistp->container_begin->prevp = containerp;
		containerp->nextp              = clistp->container_begin;
		containerp->prevp              = NULL;
		clistp->container_begin        = containerp;
	}
	clistp->container_count ++;
	CLIST_DBGPR("_clist_container_add_top: success");

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// 指定のコンテナをリスト終端につなぐ関数、（CLISTメソッド）
CLIST_RET _clist_container_add_last (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	if (clistp == NULL || containerp == NULL) {
		CLIST_ERRPR("_clist_container_add_last: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	if (clistp->container_begin == NULL) {
		// no container -> one container
		clistp->container_begin   = containerp;
		clistp->container_end     = containerp;
		containerp->nextp = NULL;
		containerp->prevp = NULL;
	} else if (clistp->container_begin == clistp->container_end) {
		// one container -> two containers
		clistp->container_end          = containerp;
		clistp->container_end->prevp   = clistp->container_begin;
		clistp->container_end->nextp   = NULL;
		clistp->container_begin->nextp = clistp->container_end;
	} else {
		// 2> containers
		clistp->container_end->nextp   = containerp;
		containerp->prevp              = clistp->container_end;
		containerp->nextp              = NULL;
		clistp->container_end          = containerp;
	} 
	clistp->container_count ++;
	CLIST_DBGPR("_clist_container_add_last: success");

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// 指定のコンテナをリストから外す関数、（CLISTメソッド）
CLIST_RET _clist_container_remove (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("_clist_container_destroy_all: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	CLIST_CONTAINER *ccp = clistp->container_begin;
	while (ccp) {
		if (ccp == containerp) {
			// containerp is exist in list
			CLIST_CONTAINER *nextp = containerp->nextp;
			CLIST_CONTAINER *prevp = containerp->prevp;
			if (nextp) nextp->prevp = prevp;
			if (prevp) prevp->nextp = nextp;
			if (clistp->container_begin == containerp) clistp->container_begin = nextp;
			if (clistp->container_end   == containerp) clistp->container_end   = prevp;
			containerp->nextp = NULL;
			containerp->prevp = NULL;
			clistp->container_count --;
			CLIST_DBGPR("_clist_container_remove: success");
			break;
		}
		ccp = ccp->nextp;
	}

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// 指定のコンテナをリスト前方に１つ移動する関数、（CLISTメソッド）
CLIST_RET _clist_container_up (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// 指定のコンテナをリスト後方に１つ移動する関数、（CLISTメソッド）
CLIST_RET _clist_container_down (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// リストをソートする関数、（CLISTメソッド）
CLIST_RET _clist_container_sort (CLIST *clistp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// デバック用、指定のコンテナを見やすい形で表示(dump)する関数、（CLISTメソッド）
CLIST_RET _clist_container_dump (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	if (clistp != NULL && clistp->dutyp != NULL && clistp->dutyp->dump != NULL) {
		clistp->dutyp->dump(containerp->datap);
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// デバック用、保持しているすべてのコンテナを見やすい形で表示(dump)する関数、（CLISTメソッド）
CLIST_RET _clist_dump_all (CLIST *clistp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("_clist_dump: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	CLIST_CONTAINER *ccp = clistp->container_begin;
	while (ccp) {
		clistp->container_dump(clistp, ccp);
		ccp = ccp->nextp;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ------------------------------------------------------------------------------------------
// utility function

// ------------------------------------------------------------------------------------------
// CLISTを生成する (CLIST　ユーティリティ関数)
CLIST *clist_create (CLIST_DATAUTY *dutyp)
{
	if (dutyp == NULL || dutyp->create == NULL || dutyp->destroy == NULL) {
		CLIST_ERRPR("clist_create: arg error");
		return NULL;
	}
	CLIST *clistp = (CLIST*)malloc (sizeof(CLIST));					// create CLIST
	if (clistp == NULL) {
		CLIST_ERRPR("clist_create: malloc error");
		return NULL;
	}
	memset (clistp, 0, sizeof(CLIST));							// zero clear

	clistp->dutyp = dutyp;

	clistp->container_create = _clist_container_create;
	clistp->container_destroy = _clist_container_destroy;
	clistp->container_destroy_all = _clist_container_destroy_all;
	clistp->container_add_top = _clist_container_add_top;
	clistp->container_add_last = _clist_container_add_last;
	clistp->container_remove = _clist_container_remove;
	clistp->container_up = _clist_container_up;
	clistp->container_down = _clist_container_down;
	clistp->container_sort = _clist_container_sort;
	clistp->container_dump = _clist_container_dump;
	clistp->dump_all = _clist_dump_all;

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return clistp;
}

// ------------------------------------------------------------------------------------------
//  CLISTを開放する (CLIST　ユーティリティ関数)
CLIST_RET clist_destroy (CLIST *clistp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("clist_destroy: clistp is NULL");
		return CLIST_RET_NG_ARGERR;
	}
	clistp->container_destroy_all (clistp);
	free (clistp);

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}

// ============================================================================================ CLIST_REFERER (CLISTリファラ構造体)　　関数群

// ------------------------------------------------------------------------------------------
// public methods

// ------------------------------------------------------------------------------------------
// 参照先をidx番目のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data(CLIST_REFERER *crefp, int idx)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return _clist_referer_data_find(crefp, idx, NULL);
}

// ------------------------------------------------------------------------------------------
// 参照先を先頭のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_begin(CLIST_REFERER *crefp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return _clist_referer_data_find_begin(crefp, NULL);
}

// ------------------------------------------------------------------------------------------
//!< 参照先を終端のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_end(CLIST_REFERER *crefp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return _clist_referer_data_find_end(crefp, NULL);
}

// ------------------------------------------------------------------------------------------
// 参照先を１つ前のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_prev(CLIST_REFERER *crefp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return _clist_referer_data_find_prev(crefp, NULL);
}

// ------------------------------------------------------------------------------------------
// 参照先を１つ次のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_next(CLIST_REFERER *crefp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return _clist_referer_data_find_next(crefp, NULL);
}

// ------------------------------------------------------------------------------------------
// 現在の参照先のユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_current(CLIST_REFERER *crefp)
{
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}

// ------------------------------------------------------------------------------------------
// コンテナの総数を返す関数、（CLISTリファラメソッド）
int _clist_referer_count(struct _CLIST_REFERER *crefp)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data: crefp is NULL");
		return 0;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return crefp->clistp->container_count;
}

// ------------------------------------------------------------------------------------------
// 指定したキー条件で、参照先をidx番目のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_find(struct _CLIST_REFERER *crefp, int idx, void *key)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data: crefp is NULL");
		return NULL;
	}
	int i = 0;
	crefp->current = crefp->clistp->container_begin;
	while(crefp->current) {
		if (key != NULL && crefp->clistp->dutyp->keymatch_chk != NULL) {
			if (idx == i && crefp->clistp->dutyp->keymatch_chk(crefp->current->datap,key) == false) break;	// found with key
		} else {
			if (idx == i) break;	// found without key
		}
		if (crefp->current->nextp == NULL) {
			// error:list end
			crefp->current = NULL;
			break;
		}
		i++;
		crefp->current = crefp->current->nextp;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}

// ------------------------------------------------------------------------------------------
// 指定したキー条件で、参照先を先頭のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_find_begin(struct _CLIST_REFERER *crefp, void *key)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data: crefp is NULL");
		return NULL;
	}
	crefp->current = crefp->clistp->container_begin;
	while(crefp->current) {
		if (key != NULL && crefp->clistp->dutyp->keymatch_chk != NULL) {
			if (crefp->clistp->dutyp->keymatch_chk(crefp->current->datap,key) == false) break;	// found with key
		} else {
			break;	// found without key
		}
		if (crefp->current->nextp == NULL) {
			// list end -> not found
			crefp->current = NULL;
			break;
		}
		crefp->current = crefp->current->nextp;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}

// ------------------------------------------------------------------------------------------
// 指定したキー条件で、参照先を終端のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_find_end(struct _CLIST_REFERER *crefp, void *key)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data: crefp is NULL");
		return NULL;
	}
	crefp->current = crefp->clistp->container_end;
	while(crefp->current) {
		if (key != NULL && crefp->clistp->dutyp->keymatch_chk != NULL) {
			if (crefp->clistp->dutyp->keymatch_chk(crefp->current->datap,key) == false) break;	// found with key
		} else {
			break;	// found without key
		}
		if (crefp->current->prevp == NULL) {
			// list end -> not found
			crefp->current = NULL;
			break;
		}
		crefp->current = crefp->current->prevp;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}

// ------------------------------------------------------------------------------------------
// 指定したキー条件で、参照先を１つ次のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_find_prev(struct _CLIST_REFERER *crefp, void *key)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data: crefp is NULL");
		return NULL;
	}
	if (crefp->current != NULL) crefp->current = crefp->current->prevp; // 最初に１つ移動して、あとはendの処理を回す
	while(crefp->current) {
		if (key != NULL && crefp->clistp->dutyp->keymatch_chk != NULL) {
			if (crefp->clistp->dutyp->keymatch_chk(crefp->current->datap,key) == false) break;	// found with key
		} else {
			break;	// found without key
		}
		if (crefp->current->prevp == NULL) {
			// list end -> not found
			crefp->current = NULL;
			break;
		}
		crefp->current = crefp->current->prevp;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}

// ------------------------------------------------------------------------------------------
// 指定したキー条件で、参照先を１つ前のコンテナに移動し、ユーザーデータのポインタを返す関数、（CLISTリファラメソッド）
void *_clist_referer_data_find_next(struct _CLIST_REFERER *crefp, void *key)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data: crefp is NULL");
		return NULL;
	}
	if (crefp->current != NULL) crefp->current = crefp->current->nextp; // 最初に１つ移動して、あとbeginの処理を回す
	while(crefp->current) {
		if (key != NULL && crefp->clistp->dutyp->keymatch_chk != NULL) {
			if (crefp->clistp->dutyp->keymatch_chk(crefp->current->datap,key) == false) break;	// found with key
		} else {
			break;	// found without key
		}
		if (crefp->current->nextp == NULL) {
			// list end -> not found
			crefp->current = NULL;
			break;
		}
		crefp->current = crefp->current->nextp;
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}

// ------------------------------------------------------------------------------------------
// 指定したキー条件で、コンテナの総数を返す関数、（CLISTリファラメソッド）
int _clist_referer_find_count(struct _CLIST_REFERER *crefp, void *key)
{
	// TODO: not impremented
	return _clist_referer_count(crefp);
}

// ------------------------------------------------------------------------------------------
// utility function

// ------------------------------------------------------------------------------------------
// CLISTリファラーを生成する (CLISTリファラー　ユーティリティ関数)
CLIST_REFERER *clist_referer_create (CLIST *clistp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("clist_referer_create: clistp is NULL");
		return NULL;
	}
	CLIST_REFERER *crefp = (CLIST_REFERER*)malloc(sizeof(CLIST_REFERER));
	if (crefp == NULL) {
		CLIST_ERRPR("clist_referer_create: malloc error");
		return NULL;
	}
	memset (crefp, 0, sizeof(CLIST_REFERER));
	crefp->clistp = clistp;

	crefp->data = _clist_referer_data;
	crefp->data_begin = _clist_referer_data_begin;
	crefp->data_end = _clist_referer_data_end;
	crefp->data_prev = _clist_referer_data_prev;
	crefp->data_next = _clist_referer_data_next;
	crefp->data_current = _clist_referer_data_current;
	crefp->count = _clist_referer_count;
	crefp->data_find = _clist_referer_data_find;
	crefp->data_find_begin = _clist_referer_data_find_begin;
	crefp->data_find_end = _clist_referer_data_find_end;
	crefp->data_find_prev = _clist_referer_data_find_prev;
	crefp->data_find_next = _clist_referer_data_find_next;
	crefp->find_count = _clist_referer_find_count;

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return crefp;
}

// ------------------------------------------------------------------------------------------
// CLISTリファラーを開放する (CLISTリファラー　ユーティリティ関数)
CLIST_RET clist_referer_destroy (struct _CLIST_REFERER *crefp)
{
	if (crefp == NULL) {
		CLIST_ERRPR("clist_referer_destroy: crefp is NULL");
		return CLIST_RET_NG_ARGERR;
	}
	free (crefp);

	CLIST_TEST_CHECK("1/1");	// MODULE TEST
	return CLIST_RET_OK;
}
#endif // __CLIST_C__





// ============================================================================================ モジュールテスト　＆　実装サンプル

/**
 * @brief  CLIST　モジュールテスト関数 兼 実装サンプル
 * @details　基本的にはCLISTのモジュールテストで使用するための関数で無効化されていますが、実装サンプルコードとして参照できるように実装・コメント記載しています。使用法が不明な場合はこちらを参照してください
 */
extern void clist_test_main(void);

#ifdef __CLIST_TEST_C__

typedef struct _CLIST_TEST_DATA {
	int n;
} CLIST_TEST_DATA;

static void *data2datap (int n) {
	static CLIST_TEST_DATA data2datap_buf;
	data2datap_buf.n = n;
	return (void*)(&data2datap_buf);
}
static void *clist_test_data_create(void *_param)
{
	int param = *((int*)_param);
	int *dp = (int*)malloc(sizeof(int));
	if (dp) {
		*dp = param;
		CLIST_DBGPR ("TEST: data created (%d)", *dp);
	}
	return dp;
}
static void clist_test_data_destroy(void *dp)
{
	if (dp) {
		CLIST_DBGPR ("TEST: data destroyed (%d)", *((int*)dp));
		free (dp);
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
}
static void clist_test_data_dump(void *dp)
{
	CLIST_TEST_DATA *datap = (CLIST_TEST_DATA*)dp;
	if (dp) {
		CLIST_DBGPR ("TEST: DUMP n=%d", datap->n);
	}
	CLIST_TEST_CHECK("1/1");	// MODULE TEST
}
static CLIST_DATAUTY clist_test_data_uty = {
	clist_test_data_create,		// create data
	clist_test_data_destroy,	// destroy data
	NULL,						// compare
	NULL,						// add
	NULL,						// sub
	NULL,						// mul
	NULL,						// div
	NULL,						// keymatch_chk
	clist_test_data_dump		// dump
};

void clist_test_main(void)
{

	//int test_no = 1;
	int i;
	
	printf ("-------- create test （-10から10までのコンテナを生成）\n");

	CLIST *clistp = clist_create (&clist_test_data_uty);


	for (i=0; i<5; i++) {
		clistp->container_create (clistp, data2datap(i), true/*do_add_last*/);
	}
	for (i=5; i<=10; i++) {
		CLIST_CONTAINER *ccp = clistp->container_create (clistp, data2datap(i), false/*do_add_last*/);
		clistp->container_add_last (clistp, ccp);
	}
	for (i=-1; i>=-10; i--) {
		CLIST_CONTAINER *ccp = clistp->container_create (clistp, data2datap(i), false/*do_add_last*/);
		clistp->container_add_top (clistp, ccp);
	}
	printf ("-------- result\n");
	clistp->dump_all(clistp);

	printf ("-------- remove container test （remove container[0] -> remove container[2]）\n");
	CLIST_REFERER *crefp = clist_referer_create (clistp);
	crefp->data_begin(crefp);
	clistp->container_remove (clistp, crefp->current);

	crefp->data_begin(crefp);
	crefp->data_next(crefp);
	crefp->data_next(crefp);
	clistp->container_remove (clistp, crefp->current);

	printf ("-------- result\n");
	clistp->dump_all(clistp);

	printf ("-------- remove clist test\n");
	clist_destroy (clistp);

	printf ("-------- result is none\n");

	// TESTED: clist_create (&duty);
	// TESTED: clist_container_create()
	// TESTED: clist_container_destroy()
	
	printf ("-------- user code stype sample test\n");
	{
		printf ("-------- create LIST\n");
		{
			int i;
			CLIST *clistp = clist_create (&clist_test_data_uty);
			for (i=0; i<5; i++) {
				clistp->container_create (clistp, data2datap(i), true/*do_add_last*/);
			}
		}
		printf ("-------- \n");
		{
			CLIST_REFERER *crefp = clist_referer_create (clistp);
			CLIST_TEST_DATA *datap;

			// FORWARD LOOP
			printf ("-------- loop LIST (FORWARD LOOP)\n");
			datap = (CLIST_TEST_DATA *)crefp->data_begin(crefp);
			while (datap) {
				printf (" n = %d\n", datap->n);
				datap = (CLIST_TEST_DATA *)crefp->data_next(crefp);
			}
			printf ("-------- \n");

			// REVERSE LOOP
			printf ("-------- loop LIST (REVERSE LOOP)\n");
			datap = (CLIST_TEST_DATA *)crefp->data_end(crefp);
			while (crefp->current) {
				printf (" n = %d\n", datap->n);
				datap = (CLIST_TEST_DATA *)crefp->data_prev(crefp);
			}
			printf ("-------- \n");

			clist_referer_destroy (crefp);
		}
	}

	CLIST_TEST_CHECK_END();
}
#endif // __CLIST_TEST_C__


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __C_LIST_H__


/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __C_VECTOR_H__
#define __C_VECTOR_H__

/**
 * @file c_vector.h
 * @brief C言語用簡易VECTORライブラリ
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>

#include "c_typeid.h"

#if !defined(C_VECTOR_ENB_DBGPR)
#define CVECTOR_DBGPR(fmt, args...)
#else 
#define CVECTOR_DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__,## args); }
#endif
#define CVECTOR_ERRPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__,## args); }

// テスト用
#if defined(__CVECTOR_TEST_ASSERT_ENB)

#ifdef __CVECTOR_C__
static FILE *vdbgfp = NULL;
#endif // __CVECTOR_C__

#define CVECTOR_TEST_ASSERT(nn,judge) \
	{ \
		static bool checked = false; \
		if (vdbgfp == NULL) vdbgfp = fopen (".test_result_cvector.txt", "w+"); \
		if (checked == false) { \
			if (vdbgfp != NULL) { \
				fprintf (vdbgfp, "%d:  %s() %s %s\n", __LINE__, __FUNCTION__,nn,judge ? "[OK]" : "[NG]"); \
				checked = true; \
			} \
		} \
	}
#define CVECTOR_TEST_ASSERT_CLOSE() \
	{ \
		if (vdbgfp != NULL) { \
			fclose(vdbgfp); \
			vdbgfp = NULL; \
		} \
		system ("sort -o test_result_cvector.txt .test_result_cvector.txt"); \
		system ("rm -f .test_result_cvector.txt"); \
		printf ("##### test result file: test_result_cvector.txt created\n"); \
}
#else
#define CVECTOR_TEST_ASSERT(nn,judge)
#define CVECTOR_TEST_ASSERT_CLOSE()
#endif

// デバック用強制エラーつきmalloc関数
#if defined(__CVECTOR_TEST_C__)
static bool c_vector_dbg_force_malloc_error = false;
void *c_vector_dbg_malloc(size_t size) {
	if (c_vector_dbg_force_malloc_error) {
		c_vector_dbg_force_malloc_error = false;
		return NULL;
	}
	return malloc(size);
}
void *c_vector_dbg_realloc(void *ptr, size_t size) {
	if (c_vector_dbg_force_malloc_error) {
		c_vector_dbg_force_malloc_error = false;
		return NULL;
	}
	return realloc(ptr,size);
}
#define c_vector_malloc(size) c_vector_dbg_malloc(size)
#define c_vector_realloc(ptr,size) c_vector_dbg_realloc(ptr,size)
#define c_vector_force_allocerr() c_vector_dbg_force_malloc_error = true
#else
#define c_vector_malloc(size)   malloc(size)
#define c_vector_realloc(size) remalloc(ptr,size)
#define c_vector_force_allocerr()
#endif // defined(__CVECTOR_TEST_C__)

// ============================================================================================ 型宣言、マクロ等

/// CVECTOR系関数の戻り値用定義

typedef void* voidp;		//!< タイプ定義　voidp = (void*)

typedef union _c_vector_arrayp {
	char          *c;
	unsigned char *uc;
	int           *i;
	unsigned int  *ui;
	float         *f;
	double        *d;
	voidp         *vp;
	voidp         ptr;		// pointer取得用
} c_vector_arrayp;

typedef struct _c_vector {

	CTYPEID  data_type;
	c_vector_arrayp data;

	int  data_num;
	int  alloced_num;
	int  element_size;
} c_vector;

typedef union _c_vector_data {
	char          c;
	unsigned char uc;
	int           i;
	unsigned int  ui;
	float         f;
	double        d;
	voidp         vp;
	voidp         ptr;		// pointer取得用
} c_vector_data;

#define CVECTOR_ALLOC_MINMUM  10
#define CVECTOR_ALLOC_RESERVE 5

// ============================================================================================ utility関数 宣言

/// 確保済みのc_vectorを初期化する　(要素数のみ指定、データ初期値なし(ゼロで初期化))
extern c_vector *c_vector_alloc_data (c_vector *thisp, CTYPEID type, int num);

/// c_vectorをallocして初期化　(要素数のみ指定、データ初期値なし(ゼロで初期化))  
extern c_vector *c_vector_alloc (CTYPEID type, int num);

/// c_vectorを初期化（確保積みメモリ領域用初期化）　(要素数のみ指定、データ初期値なし(ゼロで初期化))
extern c_vector *c_vector_init (c_vector *thisp, CTYPEID type, int num);

/// c_vectorを解放する、データのみ開放
extern void c_vector_free_data (c_vector *thisp);

/// c_vectorを解放する
extern void c_vector_free (c_vector *thisp);

/// c_vectorのデータ領域を指定要素数にリサイズ、成功時trueを返す
extern c_vector *c_vector_resize (c_vector *thisp, int num);

/// c_vectorのデータ領域を指定要素数にリサイズ、拡張時初期値付、成功時trueを返す
extern c_vector *c_vector_resize_initval (c_vector *thisp, int num, ...);

/// c_vectorの確保メモリが要素数に対して最適になるようにリサイズ（リザーブ領域は確保されない）、成功時trueを返す
extern c_vector *c_vector_shrink (c_vector *thisp);

/// c_vectorを初期化、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern c_vector *c_vector_dataset_arg (c_vector *thisp, int num, ...);

#define c_vector_alloc_arg(type,num,...) c_vector_dataset_arg(c_vector_alloc(type,num),num,__VA_ARGS__)

/// c_vectorを初期化、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern c_vector *c_vector_dataset_array (c_vector *thisp, int num, ...);

#define c_vector_alloc_array(type,num,...) c_vector_dataset_array(c_vector_new(type,num),num,__VA_ARGS__)

/// c_vectorを初期化、処理内容としては、source からの deep copy と等しい。、成功時trueを返す
extern c_vector *c_vector_dataset_copy (c_vector *thisp, c_vector *source);

#define c_vector_alloc_c_vectorp(source) c_vector_dataset_copy(c_vector_new(type,source ? source->data_num : 0),source)
	
/// c_vectorの配列後方にデータを追加する、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern c_vector *c_vector_addlast (c_vector *thisp, ...);
#define c_vector_push_back c_vector_addlast

/// c_vectorの配列後方にデータを追加する、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern c_vector *c_vector_addlast_arg (c_vector *thisp, int num, ...);
#define c_vector_push_back_arg c_vector_addlast_arg

/// c_vectorの配列後方にデータを追加する、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern c_vector *c_vector_addlast_array (c_vector *thisp, int num, ...);
#define c_vector_push_back_array c_vector_addlast_array

/// c_vectorの配列後方にデータを追加する、(要素数指定あり、データ初期値あり(c_vector(int型))、必要に応じてresizeも行われる、成功時trueを返す
extern c_vector *c_vector_addlast_c_vector (c_vector *thisp, c_vector *source);
#define c_vector_push_back_c_vector c_vector_addlast_c_vector

/// c_vectorの
extern c_vector *c_vector_copy (c_vector *thisp, c_vector *source);

/// c_vectorの
//extern c_vector *c_vector_insert (c_vector *thisp, int idx, ...);

/// c_vectorの
//extern c_vector *c_vector_insert_arg (c_vector *thisp, int idx, int num, ...);

/// c_vectorの配列要素をゼロクリアする
extern c_vector *c_vector_zeros (c_vector *thisp);

/// c_vectorの配列をゼロにリサイズ、要素数はゼロとなる
extern c_vector *c_vector_clear (c_vector *thisp);

/// c_vectorの要素数を返す
extern int c_vector_size (c_vector *thisp);

/// c_vectorの内容をダンプする、デバック用
extern void c_vector_dump (c_vector *thisp, char *name);

// ------------------------
/// 確保済みのc_vector<MEMORY SPACE>を初期化する　(要素数のみ指定、データ初期値なし(ゼロで初期化))
extern c_vector *c_vector_ms_alloc_data (c_vector *thisp, int num, int mssize);

/// c_vector<MEMORY SPACE>をallocして初期化　(要素数のみ指定、データ初期値なし(ゼロで初期化))  
extern c_vector *c_vector_ms_alloc (int num, int mssize);

/// c_vector<MEMORY SPACE>を初期化（確保積みメモリ領域用初期化）　(要素数のみ指定、データ初期値なし(ゼロで初期化))
extern c_vector *c_vector_ms_init (c_vector *thisp, int num, int mssize);

/**
 * @brief  c_vector の配列最後尾データのindex値を返す、要素数ゼロの場合は負の値を返す、c_unordered_setの代用処理で使用する
 * @param  thisp c_vector構造体のポインタ
 * @return 配列最後尾データのindex値
 */
inline int c_vector_getidx_end (c_vector *thisp) {
	if (thisp == NULL) {
		CVECTOR_ERRPR("thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: thisp is NULL",(thisp == NULL));
		return -1;
	}
	if (thisp->data_num == 0) {
		CVECTOR_TEST_ASSERT("2/3 normal end, data_num is 0",(true));
		return -1;					// 要素数ゼロなので-1を返す
	}
	CVECTOR_TEST_ASSERT("3/3 normal enda",(true));
	return thisp->data_num-1;		// 最後尾のindexを返す
}

/**
 * @brief  c_vector の配列をサーチし、引数と同じデータがあったらそのindex値を返す、c_unordered_setの代用処理で使用する
 * @param  thisp c_vector構造体のポインタ
 * @param  VA_ARG(0) サーチするデータ値、alloc時に指定した任意の型で１つだけ指定できる
 * @return 見つけたデータのindex値、見つからなかった場合ば負の値を返す
 */
inline int c_vector_getidx_find (c_vector *thisp, /*<T> data*/...)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: thisp is NULL",(thisp == NULL));
		return -1;
	}
	int i;
	char dt_c;
	unsigned char dt_uc;
	int dt_i;
	unsigned int dt_ui;
	float dt_f;
	double dt_d;
	voidp dt_vp;

	va_list ap;
	va_start(ap, thisp);							// valistから入れたい値を取得し、配列に存在するかサーチする
	switch (thisp->data_type) {
	case CTYPEID_CHAR:
		dt_c = (char)va_arg(ap, int/*char*/);
		for (i=0; i<thisp->data_num; i++) if (thisp->data.c[i]  == dt_c) { goto FOUND; }
		break;
	case CTYPEID_UCHAR:
		dt_uc = (unsigned char)va_arg(ap, int/*unsigned char*/);
		for (i=0; i<thisp->data_num; i++) if (thisp->data.uc[i] == dt_uc) { goto FOUND; }
		break;
	case CTYPEID_INT:
		dt_i = (char)va_arg(ap, int);
		for (i=0; i<thisp->data_num; i++) if (thisp->data.i[i]  == dt_i) { goto FOUND; }
		break;
	case CTYPEID_UINT:
		dt_ui = (unsigned int)va_arg(ap, int/*unsigned int*/);
		for (i=0; i<thisp->data_num; i++) if (thisp->data.ui[i] == dt_ui) { goto FOUND; }
		break;
	case CTYPEID_FLOAT:
		dt_f = (float)va_arg(ap, double/*float*/);
		for (i=0; i<thisp->data_num; i++) if (thisp->data.f[i]  == dt_f) { goto FOUND; }
		break;
	case CTYPEID_DOUBLE:
		dt_d = va_arg(ap, double);
		for (i=0; i<thisp->data_num; i++) if (thisp->data.d[i]  == dt_d) { goto FOUND; }
		break;
	case CTYPEID_VOIDP:
		dt_vp = va_arg(ap, voidp);
		for (i=0; i<thisp->data_num; i++) if (thisp->data.vp[i] == dt_vp) { goto FOUND; }
		break;
	case CTYPEID_MSPACE:
		dt_vp = va_arg(ap, voidp);
		for (i=0; i<thisp->data_num; i++) if (memcmp(thisp->data.ptr,dt_vp,thisp->element_size) == 0) { goto FOUND; }
		break;
	default: break;
	}
	va_end(ap);
	CVECTOR_TEST_ASSERT("2/3 normal end, not found",(true));
	return -1;
 FOUND:
	va_end(ap);
	CVECTOR_TEST_ASSERT("3/3 normal end, found",(true));
	return i;
}
	
/**
 * @brief  c_vector の配列をチェックし、同じデータがないときだけ最後尾にデータを追加する、c_unordered_setの代用処理で使用する
 * @param  thisp c_vector構造体のポインタ
 * @param  VA_ARG(0) 追加するデータ値、alloc時に指定した任意の型で１つだけ指定できる
 * @return 見つけたデータまたは追加した位置のindex値を返す、エラー時は負の値を返す
 */
inline int c_vector_add_uniqdata (c_vector *thisp, /*<T> data*/...)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: thisp is NULL",(thisp == NULL));
		return -1;
	}
	int ret = 0;
	va_list ap;
	va_start(ap, thisp);							// valistから入れたい値を取得し、配列に存在するかサーチする
	switch (thisp->data_type) {
	case CTYPEID_CHAR:		ret = c_vector_getidx_find (thisp, va_arg(ap, int/*char*/)); break;
	case CTYPEID_UCHAR:		ret = c_vector_getidx_find (thisp, va_arg(ap, int/*unsigned char*/)); break;
	case CTYPEID_INT:		ret = c_vector_getidx_find (thisp, va_arg(ap, int)); break;
	case CTYPEID_UINT:		ret = c_vector_getidx_find (thisp, va_arg(ap, int/*unsigned int*/)); break;
	case CTYPEID_FLOAT:		ret = c_vector_getidx_find (thisp, va_arg(ap, double/*float*/)); break;
	case CTYPEID_DOUBLE:	ret = c_vector_getidx_find (thisp, va_arg(ap, double)); break;
	case CTYPEID_VOIDP:		ret = c_vector_getidx_find (thisp, va_arg(ap, voidp)); break;
	case CTYPEID_MSPACE:	ret = c_vector_getidx_find (thisp, va_arg(ap, voidp)); break;
	default: va_end(ap); return -1;
	}
	va_end(ap);
	if (ret >= 0) {
		CVECTOR_TEST_ASSERT("2/3 normal end, data already exist",(true));
		return ret;									// すでに配列中にデータがあるため、そのindexを返す
	}
	va_start(ap, thisp);							// valistから初期値代入
	switch (thisp->data_type) {
	case CTYPEID_CHAR:		c_vector_addlast (thisp, va_arg(ap, int/*char*/)); break;
	case CTYPEID_UCHAR:		c_vector_addlast (thisp, va_arg(ap, int/*unsigned char*/)); break;
	case CTYPEID_INT:		c_vector_addlast (thisp, va_arg(ap, int)); break;
	case CTYPEID_UINT:		c_vector_addlast (thisp, va_arg(ap, int/*unsigned int*/)); break;
	case CTYPEID_FLOAT:		c_vector_addlast (thisp, va_arg(ap, double/*float*/)); break;
	case CTYPEID_DOUBLE:	c_vector_addlast (thisp, va_arg(ap, double)); break;
	case CTYPEID_VOIDP:		c_vector_addlast (thisp, va_arg(ap, voidp)); break;
	case CTYPEID_MSPACE:	c_vector_addlast (thisp, va_arg(ap, voidp)); break;
	default: break;
	}
	va_end(ap);
	CVECTOR_TEST_ASSERT("3/3 normal end",(true));
	return thisp->data_num-1;						// 最後尾のindexを返す
}
	
/**
 * @brief  c_vector のindex番目の要素へ書き込む、確保済み範囲外のindexが指定されると自動的にresizeが行われる
 * @param  thisp c_vector構造体のポインタ
 * @param  index indexの指定値
 * @param  VA_ARG(0) 追加するデータ値、alloc時に指定した任意の型で１つだけ指定できる
 * @return 書き込んだ位置のindex値を返す、エラー時は負の値を返す
 */
inline int c_vector_write (c_vector *thisp, int index, /*<T> data*/...)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: thisp is NULL",(thisp == NULL));
		return -1;
	}
	if (thisp->alloced_num <= index) c_vector_resize(thisp,index);
	va_list ap;
	va_start(ap, index);							// valistから初期値代入
	switch (thisp->data_type) {
	case CTYPEID_CHAR:	thisp->data.c[index]  = va_arg(ap, int/*char*/); break;
	case CTYPEID_UCHAR:	thisp->data.uc[index] = va_arg(ap, int/*unsigned char*/); break;
	case CTYPEID_INT:	thisp->data.i[index]  = va_arg(ap, int); break;
	case CTYPEID_UINT:	thisp->data.ui[index] = va_arg(ap, int/*unsigned int*/); break;
	case CTYPEID_FLOAT:	thisp->data.f[index]  = va_arg(ap, double/*float*/); break;
	case CTYPEID_DOUBLE:thisp->data.d[index]  = va_arg(ap, double); break;
	case CTYPEID_VOIDP:	thisp->data.vp[index] = va_arg(ap, voidp); break;
	case CTYPEID_MSPACE:
		memcpy (thisp->data.uc+thisp->element_size*index, va_arg(ap,voidp), thisp->element_size);
		break;
	default: break;
	}
	va_end(ap);
	CVECTOR_TEST_ASSERT("2/2 normal end",(true));
	return index;
}
	
/**
 * @brief  c_vector のindex番目の要素を読み出す、使用例）int型データのindex指定読み出し　c_vector_read(thisp,1)->i
 * @param  thisp c_vector構造体のポインタ
 * @param  index indexの指定値
 * @return 指定位置のアドレスを返す、エラー時はNULLを返す
 */
inline c_vector_data *c_vector_read (c_vector *thisp, int index) {
	if (thisp == NULL) {
		CVECTOR_ERRPR("thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: thisp is NULL",(thisp == NULL));
		return NULL;
	}
	switch (thisp->data_type) {
	case CTYPEID_CHAR:	CVECTOR_TEST_ASSERT("2/3 normal end",(true));	return (c_vector_data*)(&(thisp->data.c[index]));
	case CTYPEID_UCHAR:	CVECTOR_TEST_ASSERT("2/3 normal end",(true));	return (c_vector_data*)(&(thisp->data.uc[index]));
	case CTYPEID_INT:	CVECTOR_TEST_ASSERT("2/3 normal end",(true));	return (c_vector_data*)(&(thisp->data.i[index]));
	case CTYPEID_UINT:	CVECTOR_TEST_ASSERT("2/3 normal end",(true));	return (c_vector_data*)(&(thisp->data.ui[index]));
	case CTYPEID_FLOAT:	CVECTOR_TEST_ASSERT("2/3 normal end",(true));	return (c_vector_data*)(&(thisp->data.f[index]));
	case CTYPEID_DOUBLE:CVECTOR_TEST_ASSERT("2/3 normal end",(true));	return (c_vector_data*)(&(thisp->data.d[index]));
	case CTYPEID_VOIDP:	CVECTOR_TEST_ASSERT("2/3 normal end",(true));	return (c_vector_data*)(&(thisp->data.vp[index]));
	case CTYPEID_MSPACE:CVECTOR_TEST_ASSERT("2/3 normal end",(true));
		return (c_vector_data*)(&(thisp->data.uc[index*thisp->element_size]));
	default: break;
	}
	CVECTOR_TEST_ASSERT("3/3 error check: type unknown",(true));
	return NULL;
}
	

	
#ifdef __cplusplus
/// c_vector を vector に変換
#define c_vector_c2cc(cc_dest,c_src) \
{ \
	if (cc_dest.size() != c_vector_size(c_src)) cc_dest.resize(c_vector_size(c_src)); /* size check */ \
	memcpy (cc_dest.data(), c_src.data.ptr, c_vector_size(c_src)); \
}
#define c_vector_cc2c(cc_src,c_dest) \
{ \
	if (cc_src.size() != c_vector_size(c_dest)) c_vector_resize(c_dest,cc_src.size()); /* size check */ \
	memcpy (c_dest.data.ptr, cc_src.data(), cc_src.size()); \
}
#define c_vector_new_ccvector(type,cc_src) c_vector_dataset_arg(c_vector_new(type,cc_src.size()),cc_src.size(),cc_src.data())
#endif /* __cplusplus */

// ============================================================================================ utility関数 実装
#ifdef __CVECTOR_C__

c_vector *_c_vector_alloc_data_0 (c_vector *thisp, CTYPEID type, int num, int element_size)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("c_vector malloc error");
		CVECTOR_TEST_ASSERT("1/4 error check: thisp malloc errpr",(thisp == NULL));
		return NULL;
	}

	// elementsize設定
	switch (type) {
	case CTYPEID_CHAR:	thisp->element_size = sizeof(char);				break;
	case CTYPEID_UCHAR:	thisp->element_size = sizeof(unsigned char);	break;
	case CTYPEID_INT:	thisp->element_size = sizeof(int);				break;
	case CTYPEID_UINT:	thisp->element_size = sizeof(unsigned int);		break;
	case CTYPEID_FLOAT:	thisp->element_size = sizeof(float);			break;
	case CTYPEID_DOUBLE:thisp->element_size = sizeof(double);			break;
	case CTYPEID_VOIDP:	thisp->element_size = sizeof(void*);			break;
	case CTYPEID_MSPACE:thisp->element_size = element_size;				break;
	default:
		CVECTOR_ERRPR("arg type unknown");
		CVECTOR_TEST_ASSERT("2/4 error check: type unknown",true);
		return NULL;
	}
	// データ領域のallocサイズの計算
	thisp->alloced_num = num + CVECTOR_ALLOC_RESERVE;	//  デフォルトは新要素数＋リザーブ数
	if (thisp->alloced_num < CVECTOR_ALLOC_MINMUM) {
		thisp->alloced_num = CVECTOR_ALLOC_MINMUM;		//  最小サイズを適用
	}
	thisp->data.ptr = c_vector_malloc(thisp->alloced_num * thisp->element_size);
	if (thisp->data.ptr == NULL) {
		CVECTOR_ERRPR("c_vector->data malloc error");
		CVECTOR_TEST_ASSERT("3/4 error check: thisp->data malloc error",(thisp->data.vp == NULL));
		free (thisp);
		return NULL;
	}
	memset (thisp->data.ptr, 0, (thisp->alloced_num * thisp->element_size)); // データ領域ゼロクリア
	thisp->data_num  = num;		// データ数
	thisp->data_type = type;	// データタイプ

	CVECTOR_TEST_ASSERT("4/4 normal end",(thisp != NULL));
	return thisp;
}
/// 確保済みのc_vectorを初期化する　(要素数のみ指定、データ初期値なし(ゼロで初期化))
c_vector *c_vector_alloc_data (c_vector *thisp, CTYPEID type, int num)
{
	CVECTOR_TEST_ASSERT("1/1 called",(true));
	return _c_vector_alloc_data_0 (thisp, type, num, 0);
}

/// c_vectorをallocして初期化　(要素数のみ指定、データ初期値なし(ゼロで初期化))
c_vector *c_vector_alloc (CTYPEID type, int num)
{
	CVECTOR_TEST_ASSERT("1/1 called",(true));
	return c_vector_init ((c_vector*)malloc(sizeof(c_vector)), type, num);
}
/// c_vectorを初期化（確保積みメモリ領域用初期化）　(要素数のみ指定、データ初期値なし(ゼロで初期化))
c_vector *c_vector_init (c_vector *thisp, CTYPEID type, int num)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("malloc error");
		CVECTOR_TEST_ASSERT("1/2 error check: malloc error",(thisp == NULL));
		return NULL;
	}
	memset (thisp, 0, sizeof(c_vector)); // ゼロクリア
	thisp = _c_vector_alloc_data_0 (thisp, type, num, 0);
	CVECTOR_TEST_ASSERT("2/2 normal end",(true));
	return thisp;
}

/// c_vectorを解放する、データのみ開放
void c_vector_free_data (c_vector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",(thisp == NULL));
		return;
	}
	free (thisp->data.ptr);

	CVECTOR_TEST_ASSERT("2/2 normal end",(thisp != NULL));
}

/// c_vectorを解放する
void c_vector_free (c_vector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",(thisp == NULL));
		return;
	}
	free (thisp->data.ptr);
	free (thisp);

	CVECTOR_TEST_ASSERT("2/2 free OK",(thisp != NULL));
}

/// c_vectorのデータ領域を指定要素数にリサイズ、成功時trueを返す、失敗時はfalseが返るがthisp自体がfreeされるので注意！
c_vector *c_vector_resize (c_vector *thisp, int num)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return NULL;
	}
	int old_alloced_num = thisp->alloced_num;
	if (thisp->alloced_num != (num + CVECTOR_ALLOC_RESERVE)) {
		// サイズ変更必要
		thisp->data_num    = num;
		thisp->alloced_num = num + CVECTOR_ALLOC_RESERVE;
		CVECTOR_DBGPR("do realloc %d->%d",(old_alloced_num * thisp->element_size),(thisp->alloced_num * thisp->element_size));
		thisp->data.ptr = c_vector_realloc(thisp->data.ptr, thisp->alloced_num * thisp->element_size);
		if (thisp->data.ptr == NULL) {
			// realloc失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			free (thisp);
			return NULL;
		}
		if (old_alloced_num < thisp->alloced_num) {
			// サイズが広がったので、その分をセロクリア
			memset ((void*)((unsigned long)(thisp->data.ptr) + old_alloced_num*thisp->element_size),
					0, ((thisp->alloced_num-old_alloced_num) * thisp->element_size));
		}
	} else {
		thisp->data_num = num;	// メモリはそのままで要素数だけ書き換え
	}
	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}
/// c_vectorの確保メモリが要素数に対して最適になるようにリサイズ（リザーブ領域は確保されない）、成功時trueを返す
c_vector *c_vector_shrink (c_vector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return NULL;
	}
	int old_alloced_num = thisp->alloced_num;
	if (thisp->alloced_num != thisp->data_num) {
		// サイズ変更必要
		thisp->alloced_num = thisp->data_num;
		thisp->data.ptr = c_vector_realloc(thisp->data.ptr, thisp->alloced_num * thisp->element_size);
		if (thisp->data.ptr == NULL) {
			// realloc失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			free (thisp);
			return NULL;
		}
		if (old_alloced_num < thisp->alloced_num) {
			// サイズが広がったので、その分をセロクリア
			memset ((void*)((unsigned long)(thisp->data.ptr) + old_alloced_num*thisp->element_size),
					0, ((thisp->alloced_num-old_alloced_num) * thisp->element_size));
		}
	}
	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}


/// c_vectorを初期化、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる
c_vector *c_vector_dataset_arg (c_vector *thisp, int num, ...)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return NULL;
	}
	if (thisp->alloced_num < num) {
		// 領域がたらないのでリサイズ必要
		if (c_vector_resize(thisp,num) == NULL) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return NULL;
		}
	} else {
		thisp->data_num = num;	// メモリはそのままで要素数だけ書き換え
	}

	// 可変長引数からの初期値代入
	int i;
	va_list ap;
	va_start(ap, num);							// valistから初期値代入
	for (i=0; i<num; i++) {
		switch (thisp->data_type) {
		case CTYPEID_CHAR:	thisp->data.c[i]  = va_arg(ap, int/*char*/); break;
		case CTYPEID_UCHAR:	thisp->data.uc[i] = va_arg(ap, int/*unsigned char*/); break;
		case CTYPEID_INT:	thisp->data.i[i]  = va_arg(ap, int); break;
		case CTYPEID_UINT:	thisp->data.ui[i] = va_arg(ap, int/*unsigned int*/); break;
		case CTYPEID_FLOAT:	thisp->data.f[i]  = va_arg(ap, double/*float*/); break;
		case CTYPEID_DOUBLE:thisp->data.d[i]  = va_arg(ap, double); break;
		case CTYPEID_VOIDP:	thisp->data.vp[i] = va_arg(ap, voidp); break;
		case CTYPEID_MSPACE:
			memcpy (thisp->data.uc+thisp->element_size*i, va_arg(ap,voidp), thisp->element_size);
			break;
		default: break;
			// unknown type 時のエラー処理は実装しても通らないので省略
			// va_argの第２引数、int/double/pointerで指定するのが正しいらしい
		}
	}
	va_end(ap);

	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorを初期化、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる
c_vector *c_vector_dataset_array (c_vector *thisp, int num, ...)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return NULL;
	}
	if (thisp->alloced_num < num) {
		// 領域がたらないのでリサイズ必要
		if (c_vector_resize(thisp,num) == NULL) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return NULL;
		}
	} else {
		thisp->data_num = num;	// メモリはそのままで要素数だけ書き換え
	}

	// 可変長引数の先頭から配列の先頭アドレスを取り出してmemcpyする
	va_list ap;
	va_start(ap, num);							// valistから初期値代入
	memcpy (thisp->data.ptr, va_arg(ap,voidp/*char**/), (num*thisp->element_size));	// 引数配列からコピー
	va_end(ap);

	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorを初期化、処理内容としては、source からの deep copy
c_vector *c_vector_dataset_copy (c_vector *thisp, c_vector *source)
{
	if (thisp == NULL || source == NULL) {
		CVECTOR_ERRPR("arg thisp or source is NULL");
		CVECTOR_TEST_ASSERT("1/4 error check: arg error",(thisp == NULL || source == NULL));
		return NULL;
	}
	if (thisp->data_type != source->data_type
		|| thisp->element_size != source->element_size) {
		CVECTOR_ERRPR("data type not match");
		CVECTOR_TEST_ASSERT("2/4 error check: data type not match",(thisp->data_type != source->data_type));
		return NULL;
	}
	if (thisp->alloced_num < source->data_num) {
		// 領域がたらないのでリサイズ必要
		if (c_vector_resize(thisp,source->data_num) == NULL) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("3/4 error check: realloc error",true);
			return NULL;
		}
	} else {
		thisp->data_num = source->data_num;	// メモリはそのままで要素数だけ書き換え
	}

	// データコピー
	memcpy (thisp->data.ptr, source->data.ptr, (source->data_num*source->element_size));	// 引数配列からコピー
	thisp->data_num = source->data_num;

	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorの配列後方にデータを追加する、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
c_vector *c_vector_addlast (c_vector *thisp, ...)
{
	va_list ap;
	va_start(ap, thisp);							// valistから初期値代入
	switch (thisp->data_type) {
	case CTYPEID_CHAR:	thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, int/*char*/)); break;
	case CTYPEID_UCHAR:	thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, int/*unsigned char*/)); break;
	case CTYPEID_INT:	thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, int)); break;
	case CTYPEID_UINT:	thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, int/*unsigned int*/)); break;
	case CTYPEID_FLOAT:	thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, double/*float*/)); break;
	case CTYPEID_DOUBLE:thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, double)); break;
	case CTYPEID_VOIDP:	thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, voidp)); break;
	case CTYPEID_MSPACE:thisp = c_vector_addlast_arg (thisp, 1, va_arg(ap, voidp)); break;
	default: break;
		// unknown type 時のエラー処理は実装しても通らないので省略
		// va_argの第２引数、int/double/pointerで指定するのが正しいらしい
	}
	va_end(ap);

	CVECTOR_TEST_ASSERT("1/1 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorの配列後方にデータを追加する、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる
c_vector *c_vector_addlast_arg (c_vector *thisp, int num, ...)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return NULL;
	}
	int org_data_num = thisp->data_num;
	if (thisp->alloced_num < (org_data_num+num)) {
		// 領域がたらないのでリサイズ必要
		if (c_vector_resize(thisp,(org_data_num+num)) == NULL) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return NULL;
		}
	} else {
		thisp->data_num += num;	// メモリはそのままで要素数だけ書き換え
	}

	// 可変長引数からの初期値代入
	int i;
	va_list ap;
	va_start(ap, num);							// valistから初期値代入
	for (i=0; i<num; i++) {
		switch (thisp->data_type) {
		case CTYPEID_CHAR:	thisp->data.c[org_data_num+i]  = va_arg(ap, int/*char*/); break;
		case CTYPEID_UCHAR:	thisp->data.uc[org_data_num+i] = va_arg(ap, int/*unsigned char*/); break;
		case CTYPEID_INT:	thisp->data.i[org_data_num+i]  = va_arg(ap, int); break;
		case CTYPEID_UINT:	thisp->data.ui[org_data_num+i] = va_arg(ap, int/*unsigned int*/); break;
		case CTYPEID_FLOAT:	thisp->data.f[org_data_num+i]  = va_arg(ap, double/*float*/); break;
		case CTYPEID_DOUBLE:thisp->data.d[org_data_num+i]  = va_arg(ap, double); break;
		case CTYPEID_VOIDP:	thisp->data.vp[org_data_num+i] = va_arg(ap, voidp); break;
		case CTYPEID_MSPACE:
			memcpy (thisp->data.uc+thisp->element_size*(org_data_num+i), va_arg(ap,voidp), thisp->element_size);
			break;
		default: break;
			// unknown type 時のエラー処理は実装しても通らないので省略
			// va_argの第２引数、int/double/pointerで指定するのが正しいらしい
		}
	}
	va_end(ap);

	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorの配列後方にデータを追加する、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる
c_vector *c_vector_addlast_array (c_vector *thisp, int num, ...)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return NULL;
	}
	int org_data_num = thisp->data_num;
	if (thisp->alloced_num < (org_data_num+num)) {
		// 領域がたらないのでリサイズ必要
		if (c_vector_resize(thisp,(org_data_num+num)) == NULL) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return NULL;
		}
	} else {
		thisp->data_num += num;	// メモリはそのままで要素数だけ書き換え
	}

	// 可変長引数の先頭から配列の先頭アドレスを取り出してmemcpyする
	va_list ap;
	va_start(ap, num);							// valistから初期値代入
	memcpy ((void*)((unsigned long)(thisp->data.ptr) + org_data_num*thisp->element_size),
			va_arg(ap,voidp/*char**/), (num*thisp->element_size));	// 引数配列からコピー
	va_end(ap);

	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorの配列後方にデータを追加する、(要素数指定あり、データ初期値あり(c_vector(int型))、必要に応じてresizeも行われる
c_vector *c_vector_addlast_c_vector (c_vector *thisp, c_vector *source)
{
	CVECTOR_TEST_ASSERT("1/1 addlast cvecor OK",true);
	return c_vector_addlast_array (thisp, source->data_num, source->data.ptr);
}

/// c_vectorの配列要素をゼロクリアする
c_vector *c_vector_zeros (c_vector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",thisp == NULL);
		return NULL;
	}
	memset (thisp->data.ptr, 0, (thisp->data_num * thisp->element_size)); // データ領域ゼロクリア

	CVECTOR_TEST_ASSERT("2/2 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorの配列をゼロにリサイズ、要素数はゼロとなる
c_vector *c_vector_clear (c_vector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",thisp == NULL);
		return NULL;
	}
	if (c_vector_resize(thisp,0) == NULL) {
		// リサイズ失敗
		CVECTOR_ERRPR("realloc failed");
		CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
		return NULL;
	}
	CVECTOR_TEST_ASSERT("3/3 normal end",(thisp != NULL));
	return thisp;
}

/// c_vectorの要素数を返す
int c_vector_size (c_vector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",thisp == NULL);
		return 0;
	}
	CVECTOR_TEST_ASSERT("2/2 return data_num",true);
	return thisp->data_num;
}

/// c_vectorの内容をダンプする、デバック用
void c_vector_dump (c_vector *thisp, char *name)
{
	if (thisp == NULL) {
		printf ("thisp = NULL\n");
	} else {
		int i,j;
		printf ("c_vector %s {\n", name);
		printf ("    data_num = %d;", thisp->data_num);
		printf (" alloced_num = %d;", thisp->alloced_num);
		printf (" element_size = %d;  ", thisp->element_size);
		switch (thisp->data_type) {
		case CTYPEID_CHAR:
			printf (" data_type = char, ");
			printf (" #### data.c[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.c[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CTYPEID_UCHAR:
			printf (" data_type = unsigned char, ");
			printf (" #### data.uc[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.uc[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CTYPEID_INT:
			printf (" data_type = int, ");
			printf (" #### data.i[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.i[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CTYPEID_UINT:
			printf (" data_type = unsigned int, ");
			printf (" #### data.ui[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.ui[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CTYPEID_FLOAT:
			printf (" data_type = float, ");
			printf (" #### data.f[%d] = {", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%f ", thisp->data.f[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CTYPEID_DOUBLE:
			printf (" data_type = double, ");
			printf (" #### data.d[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%f ", thisp->data.d[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CTYPEID_VOIDP:
			printf (" data_type = voidp, ");
			printf (" #### data.vp[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%08lx ", (unsigned long)(thisp->data.vp[i]));
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CTYPEID_MSPACE:
			printf (" data_type = memory space, ");
			printf (" #### data.mspace[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) {
				for (j=0; j<thisp->element_size; j++) {
					printf ("%d ", thisp->data.uc[i*thisp->element_size+j]);
				}
				printf ("\n");
			}
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		default: break;
		}
	}
}

/// 確保済みのc_vector<MEMORY SPACE>を初期化する　(要素数のみ指定、データ初期値なし(ゼロで初期化))
c_vector *c_vector_ms_alloc_data (c_vector *thisp, int num, int mssize)
{
	CVECTOR_TEST_ASSERT("1/1 called",(true));
	return _c_vector_alloc_data_0 (thisp, CTYPEID_MSPACE, num, mssize);
}

/// c_vector<MEMORY SPACE>をallocして初期化　(要素数のみ指定、データ初期値なし(ゼロで初期化))  
c_vector *c_vector_ms_alloc (int num, int mssize)
{
	CVECTOR_TEST_ASSERT("1/1 called",(true));
	return c_vector_ms_init ((c_vector*)malloc(sizeof(c_vector)), num, mssize);
}

/// c_vector<MEMORY SPACE>を初期化（確保積みメモリ領域用初期化）　(要素数のみ指定、データ初期値なし(ゼロで初期化))
c_vector *c_vector_ms_init (c_vector *thisp, int num, int mssize)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("malloc error");
		CVECTOR_TEST_ASSERT("1/2 error check: malloc error",(thisp == NULL));
		return NULL;
	}
	memset (thisp, 0, sizeof(c_vector)); // ゼロクリア
	thisp = _c_vector_alloc_data_0 (thisp, CTYPEID_MSPACE, num, mssize);
	CVECTOR_TEST_ASSERT("2/2 normal end",(true));
	return thisp;
}

#endif // __CVECTOR_C__
// ============================================================================================ モジュールテスト　＆　実装サンプル

/**
 * @brief  CVECTOR　モジュールテスト関数 兼 実装サンプル
 * @details　基本的にはCVECTORのモジュールテストで使用するための関数で無効化されていますが、実装サンプルコードとして参照できるように実装・コメント記載しています。使用法が不明な場合はこちらを参照してください
 */
extern void c_vector_test_main(void);

#ifdef __CVECTOR_TEST_C__
void c_vector_test_main(void)
{
	{
		printf ("---------------------------\n");
		c_vector *cvectp = c_vector_alloc (CTYPEID_CHAR, 0);
		c_vector_dump (cvectp, (char*)"c_vector.alloc(0)");
		c_vector_free (cvectp);
	}
	{
		printf ("---------------------------\n");
		c_vector cvectp;
		c_vector_alloc_data (&cvectp, CTYPEID_CHAR, 5);
		c_vector_dump (&cvectp, (char*)"c_vector.alloc(5)");
		c_vector_free_data (&cvectp);
	}
	{
		printf ("---------------------------\n");
		c_vector *cvectp = c_vector_alloc (CTYPEID_CHAR, 9);
		c_vector_dump (cvectp, (char*)"c_vector.alloc(9)");
		c_vector_free (cvectp);
	}
	{
		printf ("---------------------------\n");
		c_vector *cvectp = c_vector_alloc (CTYPEID_CHAR, 100);
		for (int i=0; i<cvectp->data_num; i++) cvectp->data.c[i] = i;
		c_vector_dump (cvectp, (char*)"c_vector.alloc(10)");
		c_vector_free (cvectp);
	}
	printf ("---------------------------\n");
	{
		c_vector *cvectp = c_vector_alloc_arg (CTYPEID_CHAR, 6, 1,2,3,4,5,6);
		c_vector_dump (cvectp, (char*)"c_vector.dataset_arg(6, 1,2,3,4,5,6)");
		c_vector_addlast_arg (cvectp, 6, -1,-2,-3,-4,-5,-127);
		c_vector_dump (cvectp, (char*)"c_vector.addlast_arg(6, -1,-2,-3,-4,-5,-127)");
		c_vector_resize (cvectp, 5);
		c_vector_dump (cvectp, (char*)"c_vector.resize(5)");
		c_vector_zeros (cvectp);
		c_vector_dump (cvectp, (char*)"c_vector.zeros()");
		c_vector_clear (cvectp);
		c_vector_dump (cvectp, (char*)"c_vector.clear()");
		c_vector_free (cvectp);
	}
	printf ("---------------------------\n");
	{
		c_vector *cvectp1 = c_vector_alloc (CTYPEID_CHAR, 0);
		c_vector *cvectp2 = c_vector_alloc (CTYPEID_CHAR, 0);
		c_vector_dataset_arg (cvectp1, 6, 1,2,3,4,5,6);
		c_vector_dump (cvectp1, (char*)"c_vector.alloc(0)");
		c_vector_dataset_copy (cvectp2, cvectp1);
		c_vector_dump (cvectp2, (char*)"c_vector_2.dataset(c_vector)");
		c_vector_free (cvectp1);
		c_vector_free (cvectp2);
	}
	printf ("---------------------------\n");
	{
		c_vector *cvectp = c_vector_alloc (CTYPEID_INT, 0);
		c_vector_dataset_arg (cvectp, 6, 1,2,3,4,5,6);
		c_vector_dump (cvectp, (char*)"c_vector.dataset_arg(6, 1,2,3,4,5,6) <int>");
		c_vector_zeros (cvectp);
		c_vector_dump (cvectp, (char*)"c_vector.zeros() <float>");
		c_vector_free (cvectp);
	}
	printf ("---------------------------\n");
	{
		c_vector *cvectp = c_vector_alloc (CTYPEID_FLOAT, 0);
		c_vector_dataset_arg (cvectp, 6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6);
		c_vector_dump (cvectp, (char*)"c_vector.dataset_arg(6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6) <float>");
		c_vector_free (cvectp);
	}
	printf ("---------------------------\n");
	{
		c_vector *cvectp = c_vector_alloc (CTYPEID_DOUBLE, 0);
		c_vector_dataset_arg (cvectp, 6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6);
		c_vector_dump (cvectp, (char*)"c_vector.dataset_arg(6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6) <double>");
		c_vector_free (cvectp);
	}
	printf ("---------------------------\n");
	{
		c_vector *cvectp = c_vector_alloc (CTYPEID_VOIDP, 0);
		c_vector_dataset_arg (cvectp, 3, (voidp)"str1", (voidp)"str2", (voidp)"str3");
		c_vector_dump (cvectp, (char*)"c_vector.dataset_arg((voidp)\"str1\", (voidp)\"str2\", (voidp)\"str3\") <voidp>");
		{
			int i,size=c_vector_size(cvectp);
			for (i=0; i<size; i++) {
				printf ("## data.vp[%d] = %s\n", i, (char*)(cvectp->data.vp[i]));
			}
		}
		c_vector_zeros (cvectp);
		c_vector_dump (cvectp, (char*)"c_vector.zeros() <float>");
		c_vector_free (cvectp);
	}
	printf ("---------------------------\n");
	{
		c_vector *cvectp = c_vector_alloc (CTYPEID_CHAR, 0);
		c_vector_dataset_arg (cvectp, 6, 1,2,3,4,5,6);
		c_vector_dump (cvectp, (char*)"c_vector.dataset_arg(6, 1,2,3,4,5,6)");
		{
			int i,size=c_vector_size(cvectp);
			for (i=0; i<size; i++) {
				printf ("## data.c[%d] = %d\n", i, cvectp->data.c[i]);
			}
		}
		c_vector_free (cvectp);
	}
	printf ("---------------------------\n");

	
	CVECTOR_TEST_ASSERT_CLOSE();
}
#endif // __CVECTOR_TEST_C__


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __C_VECTOR_H__


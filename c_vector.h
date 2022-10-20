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
				fprintf (vdbgfp, "%d:  %s() %s %s\n", __LINE__, __FUNCTION__,nn,judge ? "OK" : "NG"); \
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
#define CVECTOR_TEST_ASSERT(nn)
#define CVECTOR_TEST_ASSERT_CLOSE()
#endif

// デバック用強制エラーつきmalloc関数
#if defined(__CVECTOR_TEST_C__)
static bool cvector_dbg_force_malloc_error = false;
void *cvector_dbg_malloc(size_t size) {
	if (cvector_dbg_force_malloc_error) {
		cvector_dbg_force_malloc_error = false;
		return NULL;
	}
	return malloc(size);
}
void *cvector_dbg_realloc(void *ptr, size_t size) {
	if (cvector_dbg_force_malloc_error) {
		cvector_dbg_force_malloc_error = false;
		return NULL;
	}
	return realloc(ptr,size);
}
#define cvector_malloc(size) cvector_dbg_malloc(size)
#define cvector_realloc(ptr,size) cvector_dbg_realloc(ptr,size)
#define cvector_force_allocerr() cvector_dbg_force_malloc_error = true
#else
#define cvector_malloc(size)   malloc(size)
#define cvector_realloc(size) remalloc(ptr,size)
#define cvector_force_allocerr()
#endif // defined(__CVECTOR_TEST_C__)

// ============================================================================================ 型宣言、マクロ等

/// CVECTOR系関数の戻り値用定義

typedef void* voidp;		//!< タイプ定義　voidp = (void*)

typedef enum {
	CVECTOR_TYPE_CHAR,		//!< タイプ指定　char型
	CVECTOR_TYPE_UCHAR,		//!< タイプ指定　unsigned char型
	CVECTOR_TYPE_INT,		//!< タイプ指定　int型
	CVECTOR_TYPE_UINT,		//!< タイプ指定　unsigned int型
	CVECTOR_TYPE_FLOAT,		//!< タイプ指定　float型
	CVECTOR_TYPE_DOUBLE,	//!< タイプ指定　double型
	CVECTOR_TYPE_VOIDP,		//!< タイプ指定　voidp型
	CVT_C  = CVECTOR_TYPE_CHAR,		//!< タイプ指定(省略系)　char型
	CVT_UC = CVECTOR_TYPE_UCHAR,	//!< タイプ指定(省略系)　unsigned char型
	CVT_I  = CVECTOR_TYPE_INT,		//!< タイプ指定(省略系)　int型
	CVT_UI = CVECTOR_TYPE_UINT,		//!< タイプ指定(省略系)　unsigned int型
	CVT_F  = CVECTOR_TYPE_FLOAT,	//!< タイプ指定(省略系)　float型
	CVT_D  = CVECTOR_TYPE_DOUBLE,	//!< タイプ指定(省略系)　double型
	CVT_VP = CVECTOR_TYPE_VOIDP,	//!< タイプ指定(省略系)　voidp型
} CVECTOR_TYPE;

typedef struct _cvector {
	union {
		char          *c;
		unsigned char *uc;
		int           *i;
		unsigned int  *ui;
		float         *f;
		double        *d;
		voidp         *vp;
		void          *ptr;		// pointer取得用
	} data;
	CVECTOR_TYPE  data_type;

	int  data_num;
	int  alloced_num;
	int  element_size;
} cvector;

#define CVECTOR_ALLOC_MINMUM  10
#define CVECTOR_ALLOC_RESERVE 5

// ============================================================================================ utility関数 宣言

/// cvectorをallocして初期化　(要素数のみ指定、データ初期値なし(ゼロで初期化))
extern cvector *cvector_alloc (CVECTOR_TYPE type, int num);

/// cvectorを解放する
extern void cvector_free (cvector *thisp);

/// cvectorのデータ領域を指定要素数にリサイズ、成功時trueを返す
extern bool cvector_resize (cvector *thisp, int num);

/// cvectorの確保メモリが要素数に対して最適になるようにリサイズ（リザーブ領域は確保されない）、成功時trueを返す
extern bool cvector_shrink (cvector *thisp);

/// cvectorを初期化、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern bool cvector_dataset_arg (cvector *thisp, int num, ...);

/// cvectorを初期化、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern bool cvector_dataset_array (cvector *thisp, int num, ...);

/// cvectorを初期化、処理内容としては、source からの deep copy と等しい。、成功時trueを返す
extern bool cvector_dataset_cvector (cvector *thisp, cvector *source);

/// cvectorの配列後方にデータを追加する、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern bool cvector_addlast_arg (cvector *thisp, int num, ...);

/// cvectorの配列後方にデータを追加する、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる、成功時trueを返す
extern bool cvector_addlast_array (cvector *thisp, int num, ...);

/// cvectorの配列後方にデータを追加する、(要素数指定あり、データ初期値あり(cvector(int型))、必要に応じてresizeも行われる、成功時trueを返す
extern bool cvector_addlast_cvector (cvector *thisp, cvector *source);

/// cvectorの配列要素をゼロクリアする
extern void cvector_zeros (cvector *thisp);

/// cvectorの配列をゼロにリサイズ、要素数はゼロとなる
extern void cvector_clear (cvector *thisp);

/// cvectorの要素数を返す
extern int cvector_size (cvector *thisp);

/// cvectorの内容をダンプする、デバック用
extern void cvector_dump (cvector *thisp, char *name);

// ============================================================================================ utility関数 実装
#ifdef __CVECTOR_C__

/// cvectorをallocして初期化　(要素数のみ指定、データ初期値なし(ゼロで初期化))
cvector *cvector_alloc (CVECTOR_TYPE type, int num)
{
	// cvector 構造体のalloc
	cvector *thisp = (cvector*)cvector_malloc(sizeof(cvector));
	if (thisp == NULL) {
		CVECTOR_ERRPR("cvector malloc error");
		CVECTOR_TEST_ASSERT("1/3 error check: thisp malloc errpr",(thisp == NULL));
		return NULL;
	}
	memset (thisp, 0, sizeof(cvector)); // ゼロクリア

	// elementsize設定
	switch (type) {
	case CVECTOR_TYPE_CHAR:		thisp->element_size = sizeof(char);				break;
	case CVECTOR_TYPE_UCHAR:	thisp->element_size = sizeof(unsigned char);	break;
	case CVECTOR_TYPE_INT:		thisp->element_size = sizeof(int);				break;
	case CVECTOR_TYPE_UINT:		thisp->element_size = sizeof(unsigned int);		break;
	case CVECTOR_TYPE_FLOAT:	thisp->element_size = sizeof(float);			break;
	case CVECTOR_TYPE_DOUBLE:	thisp->element_size = sizeof(double);			break;
	case CVECTOR_TYPE_VOIDP:	thisp->element_size = sizeof(void*);			break;
	default:
		CVECTOR_ERRPR("arg type unknown");
		CVECTOR_TEST_ASSERT("1/3 error check: type unknown",true);
		return NULL;
	}
	// データ領域のallocサイズの計算
	thisp->alloced_num = num + CVECTOR_ALLOC_RESERVE;	//  デフォルトは新要素数＋リザーブ数
	if (thisp->alloced_num < CVECTOR_ALLOC_MINMUM) {
		thisp->alloced_num = CVECTOR_ALLOC_MINMUM;		//  最小サイズを適用
	}
	thisp->data.ptr = cvector_malloc(thisp->alloced_num * thisp->element_size);
	if (thisp->data.ptr == NULL) {
		CVECTOR_ERRPR("cvector->data malloc error");
		CVECTOR_TEST_ASSERT("2/3 error check: thisp->data malloc error",(thisp->data.vp == NULL));
		free (thisp);
		return NULL;
	}
	memset (thisp->data.ptr, 0, (thisp->alloced_num * thisp->element_size)); // データ領域ゼロクリア
	thisp->data_num  = num;		// データ数
	thisp->data_type = type;	// データタイプ

	CVECTOR_TEST_ASSERT("3/3 alloc OK",(thisp != NULL));
	return thisp;
}

/// cvectorを解放する
void cvector_free (cvector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",(thisp == NULL));
		return;
	}
	free (thisp);
	CVECTOR_TEST_ASSERT("2/2 free OK",(thisp != NULL));
}

/// cvectorのデータ領域を指定要素数にリサイズ、成功時trueを返す、失敗時はfalseが返るがthisp自体がfreeされるので注意！
bool cvector_resize (cvector *thisp, int num)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return false;
	}
	int old_alloced_num = thisp->alloced_num;
	if (thisp->alloced_num != (num + CVECTOR_ALLOC_RESERVE)) {
		// サイズ変更必要
		thisp->data_num    = num;
		thisp->alloced_num = num + CVECTOR_ALLOC_RESERVE;
		CVECTOR_DBGPR("do realloc %d->%d",(old_alloced_num * thisp->element_size),(thisp->alloced_num * thisp->element_size));
		thisp->data.ptr = cvector_realloc(thisp->data.ptr, thisp->alloced_num * thisp->element_size);
		if (thisp->data.ptr == NULL) {
			// realloc失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			free (thisp);
			return false;
		}
		if (old_alloced_num < thisp->alloced_num) {
			// サイズが広がったので、その分をセロクリア
			memset ((void*)((unsigned long)(thisp->data.ptr) + old_alloced_num*thisp->element_size),
					0, ((thisp->alloced_num-old_alloced_num) * thisp->element_size));
		}
	} else {
		thisp->data_num = num;	// メモリはそのままで要素数だけ書き換え
	}
	CVECTOR_TEST_ASSERT("3/3 dataset OK",true);
	return true;
}
/// cvectorの確保メモリが要素数に対して最適になるようにリサイズ（リザーブ領域は確保されない）、成功時trueを返す
bool cvector_shrink (cvector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return false;
	}
	int old_alloced_num = thisp->alloced_num;
	if (thisp->alloced_num != thisp->data_num) {
		// サイズ変更必要
		thisp->alloced_num = thisp->data_num;
		thisp->data.ptr = cvector_realloc(thisp->data.ptr, thisp->alloced_num * thisp->element_size);
		if (thisp->data.ptr == NULL) {
			// realloc失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			free (thisp);
			return false;
		}
		if (old_alloced_num < thisp->alloced_num) {
			// サイズが広がったので、その分をセロクリア
			memset ((void*)((unsigned long)(thisp->data.ptr) + old_alloced_num*thisp->element_size),
					0, ((thisp->alloced_num-old_alloced_num) * thisp->element_size));
		}
	}

	CVECTOR_TEST_ASSERT("3/3 dataset OK",true);
	return true;
}


/// cvectorを初期化、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる
bool cvector_dataset_arg (cvector *thisp, int num, ...)
{
	bool ret = false;
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return false;
	}
	if (thisp->alloced_num < num) {
		// 領域がたらないのでリサイズ必要
		if (cvector_resize(thisp,num) == false) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return false;
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
		case CVECTOR_TYPE_CHAR:		thisp->data.c[i]  = va_arg(ap, int/*char*/); break;
		case CVECTOR_TYPE_UCHAR:	thisp->data.uc[i] = va_arg(ap, int/*unsigned char*/); break;
		case CVECTOR_TYPE_INT:		thisp->data.i[i]  = va_arg(ap, int); break;
		case CVECTOR_TYPE_UINT:		thisp->data.ui[i] = va_arg(ap, int/*unsigned int*/); break;
		case CVECTOR_TYPE_FLOAT:	thisp->data.f[i]  = va_arg(ap, double/*float*/); break;
		case CVECTOR_TYPE_DOUBLE:	thisp->data.d[i]  = va_arg(ap, double); break;
		case CVECTOR_TYPE_VOIDP:	thisp->data.vp[i] = va_arg(ap, voidp); break;
			// unknown type 時のエラー処理は実装しても通らないので省略
			// va_argの第２引数、int/double/pointerで指定するのが正しいらしい
		}
	}
	va_end(ap);
	ret = true;

	CVECTOR_TEST_ASSERT("3/3 dataset OK",ret == true);
	return ret;
}

/// cvectorを初期化、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる
bool cvector_dataset_array (cvector *thisp, int num, ...)
{
	bool ret = false;
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return false;
	}
	if (thisp->alloced_num < num) {
		// 領域がたらないのでリサイズ必要
		if (cvector_resize(thisp,num) == false) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return false;
		}
	} else {
		thisp->data_num = num;	// メモリはそのままで要素数だけ書き換え
	}

	// 可変長引数の先頭から配列の先頭アドレスを取り出してmemcpyする
	va_list ap;
	va_start(ap, num);							// valistから初期値代入
	memcpy (thisp->data.ptr, va_arg(ap,voidp/*char**/), (num*thisp->element_size));	// 引数配列からコピー
	va_end(ap);

	ret = true;

	CVECTOR_TEST_ASSERT("3/3 dataset OK",ret == true);
	return true;
}

/// cvectorを初期化、処理内容としては、source からの deep copy
bool cvector_dataset_cvector (cvector *thisp, cvector *source)
{
	bool ret = false;
	if (thisp == NULL || source == NULL) {
		CVECTOR_ERRPR("arg thisp or source is NULL");
		CVECTOR_TEST_ASSERT("1/4 error check: arg error",(thisp == NULL || source == NULL));
		return false;
	}
	if (thisp->data_type != source->data_type
		|| thisp->element_size != source->element_size) {
		CVECTOR_ERRPR("data type not match");
		CVECTOR_TEST_ASSERT("2/4 error check: data type not match",(thisp->data_type != source->data_type));
		return false;
	}
	if (thisp->alloced_num < source->data_num) {
		// 領域がたらないのでリサイズ必要
		if (cvector_resize(thisp,source->data_num) == false) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("3/4 error check: realloc error",true);
			return false;
		}
	} else {
		thisp->data_num = source->data_num;	// メモリはそのままで要素数だけ書き換え
	}

	// データコピー
	memcpy (thisp->data.ptr, source->data.ptr, (source->data_num*source->element_size));	// 引数配列からコピー
	thisp->data_num = source->data_num;
	ret = true;	// コピー成功

	CVECTOR_TEST_ASSERT("4/4 dataset OK",ret == true);
	return true;
}

/// cvectorの配列後方にデータを追加する、代入値は要素数と列挙データで指定、領域が足らない場合に限りresizeも行われる
bool cvector_addlast_arg (cvector *thisp, int num, ...)
{
	bool ret = false;
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return false;
	}
	int org_data_num = thisp->data_num;
	if (thisp->alloced_num < (org_data_num+num)) {
		// 領域がたらないのでリサイズ必要
		if (cvector_resize(thisp,(org_data_num+num)) == false) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return false;
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
		case CVECTOR_TYPE_CHAR:		thisp->data.c[org_data_num+i]  = va_arg(ap, int/*char*/); break;
		case CVECTOR_TYPE_UCHAR:	thisp->data.uc[org_data_num+i] = va_arg(ap, int/*unsigned char*/); break;
		case CVECTOR_TYPE_INT:		thisp->data.i[org_data_num+i]  = va_arg(ap, int); break;
		case CVECTOR_TYPE_UINT:		thisp->data.ui[org_data_num+i] = va_arg(ap, int/*unsigned int*/); break;
		case CVECTOR_TYPE_FLOAT:	thisp->data.f[org_data_num+i]  = va_arg(ap, double/*float*/); break;
		case CVECTOR_TYPE_DOUBLE:	thisp->data.d[org_data_num+i]  = va_arg(ap, double); break;
		case CVECTOR_TYPE_VOIDP:	thisp->data.vp[org_data_num+i] = va_arg(ap, voidp); break;
			// unknown type 時のエラー処理は実装しても通らないので省略
			// va_argの第２引数、int/double/pointerで指定するのが正しいらしい
		}
	}
	va_end(ap);
	ret = true;

	CVECTOR_TEST_ASSERT("3/3 dataset OK",ret == true);
	return ret;
}

/// cvectorの配列後方にデータを追加する、代入値は要素数と配列の先導アドレスで指定、領域が足らない場合に限りresizeも行われる
bool cvector_addlast_array (cvector *thisp, int num, ...)
{
	bool ret = false;
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/3 error check: arg error",(thisp == NULL));
		return false;
	}
	int org_data_num = thisp->data_num;
	if (thisp->alloced_num < (org_data_num+num)) {
		// 領域がたらないのでリサイズ必要
		if (cvector_resize(thisp,(org_data_num+num)) == false) {
			// リサイズ失敗
			CVECTOR_ERRPR("realloc failed");
			CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
			return false;
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

	ret = true;

	CVECTOR_TEST_ASSERT("3/3 addlast array OK",ret == true);
	return true;
}

/// cvectorの配列後方にデータを追加する、(要素数指定あり、データ初期値あり(cvector(int型))、必要に応じてresizeも行われる
bool cvector_addlast_cvector (cvector *thisp, cvector *source)
{
	CVECTOR_TEST_ASSERT("1/1 addlast cvecor OK",true);
	return cvector_addlast_array (thisp, source->data_num, source->data.ptr);
}

/// cvectorの配列要素をゼロクリアする
void cvector_zeros (cvector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",thisp == NULL);
		return;
	}
	memset (thisp->data.ptr, 0, (thisp->data_num * thisp->element_size)); // データ領域ゼロクリア
	CVECTOR_TEST_ASSERT("2/2 zeroclr OK",true);
}

/// cvectorの配列をゼロにリサイズ、要素数はゼロとなる
void cvector_clear (cvector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",thisp == NULL);
		return;
	}
	if (cvector_resize(thisp,0) == false) {
		// リサイズ失敗
		CVECTOR_ERRPR("realloc failed");
		CVECTOR_TEST_ASSERT("2/3 error check: realloc error",true);
		return;
	}
	CVECTOR_TEST_ASSERT("3/3 clear OK",true);
}

/// cvectorの要素数を返す
int cvector_size (cvector *thisp)
{
	if (thisp == NULL) {
		CVECTOR_ERRPR("arg thisp is NULL");
		CVECTOR_TEST_ASSERT("1/2 error check: arg error",thisp == NULL);
		return 0;
	}
	CVECTOR_TEST_ASSERT("2/2 return data_num",true);
	return thisp->data_num;
}

/// cvectorの内容をダンプする、デバック用
void cvector_dump (cvector *thisp, char *name)
{
	if (thisp == NULL) {
		printf ("thisp = NULL\n");
	} else {
		int i;
		printf ("cvector %s {\n", name);
		printf ("    data_num  = %d,", thisp->data_num);
		printf (" alloced_num = %d,", thisp->alloced_num);
		printf (" element_size = %d,", thisp->element_size);
		switch (thisp->data_type) {
		case CVECTOR_TYPE_CHAR:
			printf (" data_type = char, ");
			printf (" #### data.c[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.c[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CVECTOR_TYPE_UCHAR:
			printf (" data_type = unsigned char, ");
			printf (" #### data.uc[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.uc[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CVECTOR_TYPE_INT:
			printf (" data_type = int, ");
			printf (" #### data.i[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.i[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CVECTOR_TYPE_UINT:
			printf (" data_type = unsigned int, ");
			printf (" #### data.ui[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%d ", thisp->data.ui[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CVECTOR_TYPE_FLOAT:
			printf (" data_type = float, ");
			printf (" #### data.f[%d] = {", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%f ", thisp->data.f[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CVECTOR_TYPE_DOUBLE:
			printf (" data_type = double, ");
			printf (" #### data.d[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%f ", thisp->data.d[i]);
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		case CVECTOR_TYPE_VOIDP:
			printf (" data_type = voidp, ");
			printf (" #### data.vp[%d] = { ", thisp->data_num);
			for (i=0; i<thisp->data_num; i++) printf ("%08lx ", (unsigned long)(thisp->data.vp[i]));
			printf (" } reserved num %d \n", thisp->alloced_num-thisp->data_num);
			break;
		}
	}
}

#endif // __CVECTOR_C__
// ============================================================================================ モジュールテスト　＆　実装サンプル

/**
 * @brief  CVECTOR　モジュールテスト関数 兼 実装サンプル
 * @details　基本的にはCVECTORのモジュールテストで使用するための関数で無効化されていますが、実装サンプルコードとして参照できるように実装・コメント記載しています。使用法が不明な場合はこちらを参照してください
 */
extern void cvector_test_main(void);

#ifdef __CVECTOR_TEST_C__
void cvector_test_main(void)
{
	cvector *cvectp;
	cvector *cvectp_2;

	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_CHAR, 0);
	cvector_dump (cvectp, (char*)"cvector.alloc(0)");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_CHAR, 5);
	cvector_dump (cvectp, (char*)"cvector.alloc(5)");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_CHAR, 9);
	cvector_dump (cvectp, (char*)"cvector.alloc(9)");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_CHAR, 10);
	cvector_dump (cvectp, (char*)"cvector.alloc(10)");
	cvector_free (cvectp);

	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_CHAR, 0);
	cvector_dump (cvectp, (char*)"cvector.alloc(0)");
	cvector_dataset_arg (cvectp, 6, 1,2,3,4,5,6);
	cvector_dump (cvectp, (char*)"cvector.dataset_arg(6, 1,2,3,4,5,6)");
	cvector_addlast_arg (cvectp, 6, -1,-2,-3,-4,-5,-127);
	cvector_dump (cvectp, (char*)"cvector.addlast_arg(6, -1,-2,-3,-4,-5,-127)");
	cvector_resize (cvectp, 5);
	cvector_dump (cvectp, (char*)"cvector.resize(5)");
	cvector_zeros (cvectp);
	cvector_dump (cvectp, (char*)"cvector.zeros()");
	cvector_clear (cvectp);
	cvector_dump (cvectp, (char*)"cvector.clear()");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_CHAR, 0);
 	cvectp_2 = cvector_alloc (CVECTOR_TYPE_CHAR, 0);
	cvector_dataset_arg (cvectp, 6, 1,2,3,4,5,6);
	cvector_dump (cvectp, (char*)"cvector.alloc(0)");
	cvector_dataset_cvector (cvectp_2, cvectp);
	cvector_dump (cvectp_2, (char*)"cvector_2.dataset(cvector)");
	cvector_free (cvectp);
	cvector_free (cvectp_2);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_INT, 0);
	cvector_dataset_arg (cvectp, 6, 1,2,3,4,5,6);
	cvector_dump (cvectp, (char*)"cvector.dataset_arg(6, 1,2,3,4,5,6) <int>");
	cvector_zeros (cvectp);
	cvector_dump (cvectp, (char*)"cvector.zeros() <float>");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_FLOAT, 0);
	cvector_dataset_arg (cvectp, 6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6);
	cvector_dump (cvectp, (char*)"cvector.dataset_arg(6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6) <float>");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_DOUBLE, 0);
	cvector_dataset_arg (cvectp, 6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6);
	cvector_dump (cvectp, (char*)"cvector.dataset_arg(6, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6) <double>");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_VOIDP, 0);
	cvector_dataset_arg (cvectp, 3, (voidp)"str1", (voidp)"str2", (voidp)"str3");
	cvector_dump (cvectp, (char*)"cvector.dataset_arg((voidp)\"str1\", (voidp)\"str2\", (voidp)\"str3\") <voidp>");
	{
		int i,size=cvector_size(cvectp);
		for (i=0; i<size; i++) {
			printf ("## data.vp[%d] = %s\n", i, (char*)(cvectp->data.vp[i]));
		}
	}
	cvector_zeros (cvectp);
	cvector_dump (cvectp, (char*)"cvector.zeros() <float>");
	cvector_free (cvectp);
	printf ("---------------------------\n");
 	cvectp = cvector_alloc (CVECTOR_TYPE_CHAR, 0);
	cvector_dataset_arg (cvectp, 6, 1,2,3,4,5,6);
	cvector_dump (cvectp, (char*)"cvector.dataset_arg(6, 1,2,3,4,5,6)");
	{
		int i,size=cvector_size(cvectp);
		for (i=0; i<size; i++) {
			printf ("## data.c[%d] = %d\n", i, cvectp->data.c[i]);
		}
	}
	cvector_free (cvectp);
	printf ("---------------------------\n");

	
	CVECTOR_TEST_ASSERT_CLOSE();
}
#endif // __CVECTOR_TEST_C__


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __C_VECTOR_H__


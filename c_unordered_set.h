
/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __C_UNORDERED_SET_H__
#define __C_UNORDERED_SET_H__

/**
 * @file c_unordered_set.h
 * @brief C言語用簡易unordered_setライブラリ
 */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>
#include <string.h>


#if !defined(C_UNORDERED_SET_ENB_DBGPR)
#define CUNORDERED_SET_DBGPR(fmt, args...)
#else 
#define CUNORDERED_SET_DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__,## args); }
#endif
#define CUNORDERED_SET_ERRPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt "\n", __FILE__,__FUNCTION__,__LINE__,## args); }

// テスト用
#if defined(__CUNORDERED_SET_TEST_ASSERT_ENB)

#ifdef __CUNORDERED_SET_C__
static FILE *usdbgfp = NULL;
#endif // __CUNORDERED_SET_C__

#define CUNORDERED_SET_TEST_ASSERT(nn,judge) \
	{ \
		static bool checked = false; \
		if (usdbgfp == NULL) usdbgfp = fopen (".test_result_cunordered_set.txt", "w+"); \
		if (checked == false) { \
			if (usdbgfp != NULL) { \
				fprintf (usdbgfp, "%d:  %s() %s %s\n", __LINE__, __FUNCTION__,nn,judge ? "OK" : "NG"); \
				checked = true; \
			} \
		} \
	}
#define CUNORDERED_SET_TEST_ASSERT_CLOSE() \
	{ \
		if (usdbgfp != NULL) { \
			fclose(usdbgfp); \
			usdbgfp = NULL; \
		} \
		system ("sort -o test_result_cunordered_set.txt .test_result_cunordered_set.txt"); \
		system ("rm -f .test_result_cunordered_set.txt"); \
		printf ("##### test result file: test_result_cunordered_set.txt created\n"); \
}
#else
#define CUNORDERED_SET_TEST_ASSERT(nn)
#define CUNORDERED_SET_TEST_ASSERT_CLOSE()
#endif

// デバック用強制エラーつきmalloc関数
#if defined(__CUNORDERED_SET_TEST_C__)
static bool c_unordered_set_dbg_force_malloc_error = false;
void *c_unordered_set_dbg_malloc(size_t size) {
	if (c_unordered_set_dbg_force_malloc_error) {
		c_unordered_set_dbg_force_malloc_error = false;
		return NULL;
	}
	return malloc(size);
}
void *c_unordered_set_dbg_realloc(void *ptr, size_t size) {
	if (c_unordered_set_dbg_force_malloc_error) {
		c_unordered_set_dbg_force_malloc_error = false;
		return NULL;
	}
	return realloc(ptr,size);
}
#define c_unordered_set_malloc(size) c_unordered_set_dbg_malloc(size)
#define c_unordered_set_realloc(ptr,size) c_unordered_set_dbg_realloc(ptr,size)
#define c_unordered_set_force_allocerr() c_unordered_set_dbg_force_malloc_error = true
#else
#define c_unordered_set_malloc(size)   malloc(size)
#define c_unordered_set_realloc(size) remalloc(ptr,size)
#define c_unordered_set_force_allocerr()
#endif // defined(__CUNORDERED_SET_TEST_C__)

// ============================================================================================ 型宣言、マクロ等

/// CUNORDERED_SET系関数の戻り値用定義

typedef void* voidp;		//!< タイプ定義　voidp = (void*)

typedef enum {
	CUNORDERED_SET_TYPE_CHAR,		//!< タイプ指定　char型
	CUNORDERED_SET_TYPE_UCHAR,		//!< タイプ指定　unsigned char型
	CUNORDERED_SET_TYPE_INT,		//!< タイプ指定　int型
	CUNORDERED_SET_TYPE_UINT,		//!< タイプ指定　unsigned int型
	CUNORDERED_SET_TYPE_FLOAT,		//!< タイプ指定　float型
	CUNORDERED_SET_TYPE_DOUBLE,	//!< タイプ指定　double型
	CUNORDERED_SET_TYPE_VOIDP,		//!< タイプ指定　voidp型
} CUNORDERED_SET_TYPE;

typedef struct _c_unordered_set {
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
	CUNORDERED_SET_TYPE  data_type;

	int  data_num;
	int  alloced_num;
	int  element_size;
} c_unordered_set;


#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __C_UNORDERED_SET_H__

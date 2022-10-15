/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __C_LIST_H__
#define __C_LIST_H__

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
#define CLIST_TEST_CHECK() \
	{ \
		static bool checked = false; \
		if (fp == NULL) fp = fopen (".test_result_clist.txt", "w+"); \
		if (checked == false) { \
			if (fp != NULL) { \
				fprintf (fp, "%d:  %s()\n", __LINE__, __FUNCTION__); \
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
#define CLIST_TEST_CHECK()
#define CLIST_TEST_CHECK_END()
#endif
	

typedef enum {
	CLIST_RET_OK,
	CLIST_RET_NG,
	CLIST_RET_NG_ARGERR,
	CLIST_RET_NG_MEMERR,
} CLIST_RET;

// ------------------------------------------------
typedef struct _CLIST_DATAUTY {

	void *(*create)(void*/*param*/);
	void (*destroy)(void*/*data*/);
	int (*sortcompare)(void*/*data1*/,void*/*data2*/,int/*sortmode*/);
	int (*compare)(void*,void*);
	void *(*add)(void*,void*);
	void *(*sub)(void*,void*);
	void *(*mul)(void*,void*);
	void *(*div)(void*,void*);
	void (*dump)(void*);

} CLIST_DATAUTY;

// ------------------------------------------------
typedef struct _CLIST_CONTAINER {

	// private
	struct _CLIST_CONTAINER *nextp;
	struct _CLIST_CONTAINER *prevp;
	void *datap;

} CLIST_CONTAINER;

// utility function
extern struct _CLIST_CONTAINER *clist_container_create (CLIST_DATAUTY *dutyp, void *create_param);
extern void clist_container_destroy (struct _CLIST_CONTAINER *containerp, CLIST_DATAUTY *dutyp);

// ------------------------------------------------
typedef struct _CLIST {
	// private
	CLIST_CONTAINER *container_begin;
	CLIST_CONTAINER *container_end;
	
	CLIST_DATAUTY *dutyp;

	// public methods pointer
	CLIST_CONTAINER *(*container_create) (struct _CLIST *clistp, void *create_param, bool do_add_last);
	CLIST_RET (*container_destroy) (struct _CLIST *clistp, CLIST_CONTAINER *cntp);
	CLIST_RET (*container_destroy_all) (struct _CLIST *clistp);
	
	CLIST_RET (*container_add_top) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	CLIST_RET (*container_add_last) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	CLIST_RET (*container_remove) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	CLIST_RET (*container_up) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	CLIST_RET (*container_down) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	CLIST_RET (*container_sort) (struct _CLIST *clistp, int sort_param);
	CLIST_RET (*container_dump) (struct _CLIST *clistp, CLIST_CONTAINER *containerp);
	CLIST_RET (*dump_all) (struct _CLIST *clistp);

} CLIST;

// public methods extern
extern CLIST_CONTAINER *_clist_container_create (CLIST *clistp, void *create_param, bool do_add_last);
extern CLIST_RET _clist_container_destroy (CLIST *clistp, CLIST_CONTAINER *cntp);
extern CLIST_RET _clist_container_destroy_all (CLIST *clistp);

extern CLIST_RET _clist_container_add_top (CLIST *clistp, CLIST_CONTAINER *containerp);
extern CLIST_RET _clist_container_add_last (CLIST *clistp, CLIST_CONTAINER *containerp);
extern CLIST_RET _clist_container_remove (CLIST *clistp, CLIST_CONTAINER *containerp);
extern CLIST_RET _clist_container_up (CLIST *clistp, CLIST_CONTAINER *containerp);
extern CLIST_RET _clist_container_down (CLIST *clistp, CLIST_CONTAINER *containerp);
extern CLIST_RET _clist_container_sort (CLIST *clistp, int sort_param);
extern CLIST_RET _clist_container_dump (CLIST *clistp, CLIST_CONTAINER *containerp);
extern CLIST_RET _clist_dump_all (CLIST *clistp);

// utility function
extern CLIST *clist_create (CLIST_DATAUTY *dutyp);
extern CLIST_RET clist_destroy (CLIST *clistp);

// ------------------------------------------------
typedef struct _CLIST_REFERER {

	CLIST *clistp;
	CLIST_CONTAINER *current;
	
	void *(*data)(struct _CLIST_REFERER *crefp, int idx);
	void *(*data_begin)(struct _CLIST_REFERER *crefp);
	void *(*data_end)(struct _CLIST_REFERER *crefp);
	void *(*data_prev)(struct _CLIST_REFERER *crefp);
	void *(*data_next)(struct _CLIST_REFERER *crefp);
	void *(*data_current)(struct _CLIST_REFERER *crefp);

} CLIST_REFERER;

// public methods extern
extern void *_clist_referer_data(struct _CLIST_REFERER *crefp, int idx);
extern void *_clist_referer_data_begin(struct _CLIST_REFERER *crefp);
extern void *_clist_referer_data_end(struct _CLIST_REFERER *crefp);
extern void *_clist_referer_data_prev(struct _CLIST_REFERER *crefp);
extern void *_clist_referer_data_next(struct _CLIST_REFERER *crefp);
extern void *_clist_referer_data_current(struct _CLIST_REFERER *crefp);

// utility function
extern CLIST_REFERER *clist_referer_create (CLIST *clistp);
extern CLIST_RET clist_referer_destroy (struct _CLIST_REFERER *crefp);
	
#ifdef __CLIST_C__

// ================================================================== CLIST_CONTAINER

// utility function
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
	CLIST_TEST_CHECK();	// MODULE TEST
	return (containerp);
}
void clist_container_destroy (struct _CLIST_CONTAINER *containerp, CLIST_DATAUTY *dutyp)
{
	if (containerp == NULL || dutyp == NULL || dutyp->destroy == NULL) {
		CLIST_ERRPR("clist_container_destroy: arg error");
		return;
	}
	dutyp->destroy(containerp->datap);
	free (containerp);

	CLIST_TEST_CHECK();	// MODULE TEST
}

// ================================================================== CLIST

// public methods extern
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
	CLIST_TEST_CHECK();	// MODULE TEST
	return containerp;
}
CLIST_RET _clist_container_destroy (CLIST *clistp, CLIST_CONTAINER *cntp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("_clist_container_destroy: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	clist_container_destroy (cntp, clistp->dutyp);

	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
CLIST_RET _clist_container_destroy_all (CLIST *clistp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("_clist_container_destroy_all: arg error");
		return CLIST_RET_NG_ARGERR;
	}
	CLIST_REFERER *cr = clist_referer_create (clistp);
	{
		void *dp = cr->data_begin(cr);
		while (dp) {
			CLIST_CONTAINER *cc = cr->current;
			dp = cr->data_next(cr);
			clist_container_destroy (cc, clistp->dutyp);
		}
		clist_referer_destroy (cr);
	}
	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
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
	CLIST_DBGPR("_clist_container_add_top: success");

	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
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
	CLIST_DBGPR("_clist_container_add_last: success");

	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
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
			CLIST_DBGPR("_clist_container_remove: success");
			break;
		}
		ccp = ccp->nextp;
	}

	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
CLIST_RET _clist_container_up (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
CLIST_RET _clist_container_down (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
CLIST_RET _clist_container_sort (CLIST *clistp, int sort_param)
{
	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
CLIST_RET _clist_container_dump (CLIST *clistp, CLIST_CONTAINER *containerp)
{
	if (clistp != NULL && clistp->dutyp != NULL && clistp->dutyp->dump != NULL) {
		clistp->dutyp->dump(containerp->datap);
	}
	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}
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
	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}

// utility function
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

	CLIST_TEST_CHECK();	// MODULE TEST
	return clistp;
}
CLIST_RET clist_destroy (CLIST *clistp)
{
	if (clistp == NULL) {
		CLIST_ERRPR("clist_destroy: clistp is NULL");
		return CLIST_RET_NG_ARGERR;
	}
	clistp->container_destroy_all (clistp);
	free (clistp);

	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}

// ================================================================== CLIST_REFERER

// public methods
void *_clist_referer_data(CLIST_REFERER *crefp, int idx)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data: crefp is NULL");
		return NULL;
	}
	int i = 0;
	crefp->data_begin(crefp);
	while(crefp->current) {
		if (idx == i) break;	// found
		if (crefp->current->nextp == NULL) {
			// error:list end
			crefp->current = NULL;
			break;
		}
		i++;
		crefp->current = crefp->current->nextp;
	}
	CLIST_TEST_CHECK();	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}
void *_clist_referer_data_begin(CLIST_REFERER *crefp)
{
	if (crefp == NULL) {
		CLIST_ERRPR("_clist_referer_data_begin: crefp is NULL");
		return NULL;
	}
	if (crefp->clistp == NULL) {
		CLIST_ERRPR("_clist_referer_data_begin: clistp is NULL");
		crefp->current = NULL;
	} else {
		crefp->current = crefp->clistp->container_begin;	// begin
	}
	CLIST_DBGPR("_clist_referer_data_begin: current move to begin");

	CLIST_TEST_CHECK();	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}
void *_clist_referer_data_end(CLIST_REFERER *crefp)
{
	_clist_referer_data_begin(crefp);
	if (crefp != NULL && crefp->current != NULL) {
		while (crefp->current->nextp != NULL) {
			crefp->current = crefp->current->nextp;
		}
	}
	CLIST_DBGPR("_clist_referer_data_begin: current move to end");

	CLIST_TEST_CHECK();	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}
void *_clist_referer_data_prev(CLIST_REFERER *crefp)
{
	if (crefp != NULL && crefp->current != NULL) {
		crefp->current = crefp->current->prevp;					// currentは終端でNULLになる
		CLIST_DBGPR("_clist_referer_data_prev: current move to prev");
	} else {
		CLIST_DBGPR("_clist_referer_data_prev: detect loop end");
	}

	CLIST_TEST_CHECK();	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}
void *_clist_referer_data_next(CLIST_REFERER *crefp)
{
	if (crefp != NULL && crefp->current != NULL) {
		crefp->current = crefp->current->nextp;					// currentは終端でNULLになる
		CLIST_DBGPR("_clist_referer_data_prev: current move to next");
	} else {
		CLIST_DBGPR("_clist_referer_data_prev: detect loop end");
	}

	CLIST_TEST_CHECK();	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}
void *_clist_referer_data_current(CLIST_REFERER *crefp)
{
	CLIST_TEST_CHECK();	// MODULE TEST
	return (crefp != NULL && crefp->current != NULL) ? crefp->current->datap : NULL;
}

// utility function
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

	CLIST_TEST_CHECK();	// MODULE TEST
	return crefp;
}
CLIST_RET clist_referer_destroy (struct _CLIST_REFERER *crefp)
{
	if (crefp == NULL) {
		CLIST_ERRPR("clist_referer_destroy: crefp is NULL");
		return CLIST_RET_NG_ARGERR;
	}
	free (crefp);

	CLIST_TEST_CHECK();	// MODULE TEST
	return CLIST_RET_OK;
}

// ================================================================== 

#endif // __CLIST_C__

// ================================================================== MODULE TEST CODE

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
	CLIST_TEST_CHECK();	// MODULE TEST
}
static void clist_test_data_dump(void *dp)
{
	CLIST_TEST_DATA *datap = (CLIST_TEST_DATA*)dp;
	if (dp) {
		CLIST_DBGPR ("TEST: DUMP n=%d", datap->n);
	}
	CLIST_TEST_CHECK();	// MODULE TEST
}
void clist_test_main(void)
{

	//int test_no = 1;
	int i;
	CLIST_DATAUTY duty = {
		clist_test_data_create,		// create data
		clist_test_data_destroy,	// destroy data
		NULL,						// sortcompare
		NULL,						// compare
		NULL,						// add
		NULL,						// sub
		NULL,						// mul
		NULL,						// div
		clist_test_data_dump		// dump
	};
		
	printf ("-------- create test （-10から10までのコンテナを生成）\n");

	CLIST *clistp = clist_create (&duty);


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
			CLIST *clistp = clist_create (&duty);
			for (i=0; i<5; i++) {
				clistp->container_create (clistp, data2datap(i), true/*do_add_last*/);
			}
		}
		printf ("-------- \n");
		{
			CLIST_REFERER *crefp = clist_referer_create (clistp);

			printf ("-------- loop LIST (dump regular order)\n");
			crefp->data_begin(crefp);
			while (crefp->current) {
				CLIST_CONTAINER *ccp = crefp->current;
				void *datap = ccp->datap;
				CLIST_TEST_DATA *testdatap = (CLIST_TEST_DATA *)datap;
				printf (" n = %d\n", testdatap->n);
				crefp->data_next(crefp);
			}
			printf ("-------- \n");

			printf ("-------- loop LIST (dump reverse order)\n");
			crefp->data_end(crefp);
			while (crefp->current) {
				CLIST_CONTAINER *ccp = crefp->current;
				void *datap = ccp->datap;
				CLIST_TEST_DATA *testdatap = (CLIST_TEST_DATA *)datap;
				printf (" n = %d\n", testdatap->n);
				crefp->data_prev(crefp);
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


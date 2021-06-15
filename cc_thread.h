/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file wb_fifo.h
 * @brief thread class for C++  by hirai
 */

#ifndef __CC_THREAD_H__
#define __CC_THREAD_H__

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "cc_fifo.h"

#undef CC_THREAD_DBGPR
#undef CC_THREAD_ERRPR
#undef CC_THREAD_WARNPR
#define CC_THREAD_ERRPR(fmt, args...) \
	{ printf("[%s:%s():%d] ##### ERROR!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#define CC_THREAD_WARNPR(fmt, args...)											\
	{ printf("[%s:%s():%d] ##### WARNING!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }

#if !defined(CC_THREAD_ENB_DBGPR)
#define CC_THREAD_DBGPR(fmt, args...)
#else 
#define CC_THREAD_DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt, __FILE__,__FUNCTION__,__LINE__,## args); }
#endif

///
/// ■■■■■　thread 制御クラス
///
class cc_thread {
private:
public:
	// ---------------------------------------------------------
	pthread_t thread_id;							/// ■■■■■　スレッドID
	bool thread_loop;								/// ■■■■■　スレッドループ有効フラグ、falseにするとスレッドループを抜ける

	cc_fifo *fifo_to_thread;						/// ■■■■■　メインからスレッド向けのFIFO
	cc_fifo	*fifo_from_thread;						/// ■■■■■　スレッドからメイン向けのFIFO

	// ---------------------------------------------------------
	cc_thread (bool thread_up_flg, void * (*start_routine)(void *)=thread_func_template);/// ■■■■■　コンストラクタ
	virtual ~cc_thread ();							/// ■■■■■　デストラクタ

	static void *thread_func_template (void *arg);	/// ■■■■■　再定義用スレッド関数（継承先で再定義すること）
	virtual bool thread_up (void * (*start_routine)(void *)); /// ■■■■■　スレッド起動
	virtual bool thread_down (void);				/// ■■■■■　スレッド停止

	int send_to_thread (void *dptr, int dsize);		/// ■■■■■　メインからスレッドにデータ送信
	int recv_to_thread (void *bptr, int bsize);		/// ■■■■■　メインからのデータをスレッドで受信

	int send_from_thread (void *dptr, int dsize);	/// ■■■■■　スレッドからメインにデータ送信
	int recv_from_thread (void *bptr, int bsize);	/// ■■■■■　スレッドからのデータをメインで受信
};

#endif// __CC_THREAD_H__


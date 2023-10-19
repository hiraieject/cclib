/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file cc_mutex.h
 * @brief mutex class for C++
 */

#ifndef __CC_MUTEX_H__
#define __CC_MUTEX_H__

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

#undef CC_MUTEX_DBGPR
#undef CC_MUTEX_ERRPR
#undef CC_MUTEX_WARNPR
#define CC_MUTEX_ERRPR(fmt, args...) \
	{ printf(" [CC_MUTEX:%s():%d] ##### ERROR!: " fmt,__FUNCTION__,__LINE__, ## args); }
#define CC_MUTEX_WARNPR(fmt, args...)											\
	{ printf(" [CC_MUTEX:%s():%d] ##### WARNING!: " fmt,__FUNCTION__,__LINE__, ## args); }

#if !defined(CC_MUTEX_ENB_DBGPR)
#define CC_MUTEX_DBGPR(fmt, args...)
#else 
#define CC_MUTEX_DBGPR(fmt, args...)	\
	{ printf(" [CC_MUTEX:%s():%d] " fmt,__FUNCTION__,__LINE__,## args); }
#endif

///
/// ■■■■■　mutex 制御クラス
///
class cc_mutex {
private:
	pthread_mutex_t	mtx;
	char *mutex_name;
	bool blocker;
public:
	// ------------------------------------------------------------------------------------------- コンストラクタ、デストラクタ

	/// ■■■■■　コンストラクタ
	cc_mutex (char *arg_mutex_name=(char*)NULL, bool enb_recursive_lock=true)
	{
		CC_MUTEX_DBGPR ("cc_mutex: instance created\n");

		if (mutex_name == NULL) mutex_name = (char*)"unknown mutex";
		mutex_name = strdup (arg_mutex_name);

		if (enb_recursive_lock) {
			mtx = PTHREAD_RECURSIVE_MUTEX_INITIALIZER_NP;	// 再帰的にロックすることを許容する
		} else {
			mtx = PTHREAD_MUTEX_INITIALIZER;				// 再帰的にロックすることを許容しない
		}
		blocker = false;
	};

	/// ■■■■■　デストラクタ
	virtual ~cc_mutex ()
	{
		blocker = true;
		if (now_locking()) {
			unlock ((char*)__FUNCTION__);
		}
		free (mutex_name);
		
		CC_MUTEX_DBGPR ("cc_mutex: instance deleted\n");
	}

	/// ■■■■■　ロック
	bool lock (char *message=(char*)"")
	{
		if (blocker) {
			return false;
		}
#if (DEBUG_MUTEX)
		extern FILE *dbgfp;
		fprintf (dbgfp, "%s:%d: #### MUTEX LOCK start (%s:%s)\n", __FUNCTION__,__LINE__, mutex_name, message);
		fflush (dbgfp);
		pthread_mutex_lock (&mtx);
		//fprintf (dbgfp, "%s:%d: #### MUTEX LOCK done  (%s:%s)\n", __FUNCTION__,__LINE__, mutex_name, message);
		//fflush (dbgfp);
#else // (DEBUG_MUTEX)
		pthread_mutex_lock (&mtx);
#endif
		return true;
	}

	/// ■■■■■　アンロック
	bool unlock (char *message=(char*)"")
	{
#if (DEBUG_MUTEX)
		extern FILE *dbgfp;
		fprintf (dbgfp, "%s:%d: #### MUTEX UNLOCK start (%s:%s)\n", __FUNCTION__,__LINE__, mutex_name, message);
		fflush (dbgfp);
		pthread_mutex_unlock (&mtx);
		//fprintf (dbgfp, "%s:%d: #### MUTEX UNLOCK done  (%s:%s)\n", __FUNCTION__,__LINE__, mutex_name, message);
		//fflush (dbgfp);
#else // (DEBUG_MUTEX)
		pthread_mutex_unlock (&mtx);
#endif
		return true;
	}

	/// ■■■■■　ロック助教問い合わせ
	bool now_locking (void)
	{
		return true;
	}
};

#endif // __CC_MUTEX_H__

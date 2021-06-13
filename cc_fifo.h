/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file wb_fifo.h
 * @brief fifo class for C++  by hirai
 */

#ifndef __CC_FIFO_H__
#define __CC_FIFO_H__

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

//#define CC_FIFO_DBGPR

#if !defined(CC_FIFO_DBGPR) || !defined(__linux)
#undef CC_FIFO_DBGPR
#undef CC_FIFO_ERRPR
#undef CC_FIFO_WARNPR
#define CC_FIFO_DBGPR(fmt, args...)
#define CC_FIFO_ERRPR(fmt, args...)
#define CC_FIFO_WARNPR(fmt, args...)

#else 
#undef CC_FIFO_DBGPR
#undef CC_FIFO_ERRPR
#undef CC_FIFO_WARNPR
#define CC_FIFO_DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt, __FILE__,__FUNCTION__,__LINE__,## args); }
#define CC_FIFO_ERRPR(fmt, args...) \
	{ printf("[%s:%s():%d] ##### ERROR!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#define CC_FIFO_WARNPR(fmt, args...)											\
	{ printf("[%s:%s():%d] ##### WARNING!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#endif

///
/// ■■■■■　fifo 制御クラス
///
class cc_fifo {
private:
	int	fd;						// ■■■■■　ファイルデスクリプタ
	int msgQId;					// ■■■■■　メッセージキューID
	bool enabled;				// ■■■■■　FIFO使用可能フラグ

	// -------------------------------------------------------------------------------------------　fifo の作成、消去

	/// ■■■■■　fifo 作成
	bool open_fifo (void)
	{
		char fifoname[32];

		close_fifo ();
		
		// create fifo
		msgQId = msgget( IPC_PRIVATE , 0666 | IPC_CREAT );
		snprintf(fifoname, 4048, "/tmp/fifo.%d", msgQId);
		if (access(fifoname,F_OK) != 0) {
			if ((mkfifo(fifoname, 0666) < 0) && (errno != EEXIST)) {
				perror("cc_fifo: mkfifo() error");
				return false;
			}
		}
		// open fifo
		if ((fd = open(fifoname, O_RDWR | O_NONBLOCK, 0)) == -1) {
			perror("cc_fifo: open() error");
			return false;
		}
		
		enabled = true;
		CC_FIFO_DBGPR ("cc_fifo: fifo opened\n");

		return true;
	};
	
	/// ■■■■■　fifo 消去
	bool close_fifo (void)
	{
		if (fd != -1) {
			close (fd);
			enabled = false;
			CC_FIFO_DBGPR ("cc_fifo: fifo closed\n");
		}
		return true;
	}

public:

	// ------------------------------------------------------------------------------------------- コンストラクタ、デストラクタ

	/// ■■■■■　コンストラクタ
	cc_fifo (void)
	{
		CC_FIFO_DBGPR ("cc_fifo: instance created\n");

		// initialize
		fd = -1;
		msgQId = -1;
		enabled = false;

		// open fifo
		open_fifo ();
	};

	/// ■■■■■　デストラクタ
	virtual ~cc_fifo ()
	{
		char fifoname[32];
		
		// close fifo
		close_fifo ();

		// remove fifo file
		if (msgQId != -1) {
			snprintf (fifoname, 4048, "/tmp/fifo.%d", msgQId);
			if (access(fifoname,F_OK) == 0) {
				unlink (fifoname); // ファイルを消去
			}
		}
		CC_FIFO_DBGPR ("cc_fifo: instance deleted\n");
	};

	// -------------------------------------------------------------------------------------------　private 情報へのアクセス

	/// ■■■■■　ファイルデスクリプタ取得
	int get_fd (void)
	{
		return fd;				// select用にファイルデスクリプタを返す
	}

	/// ■■■■■　状況取得
	bool get_status (void)
	{
		return enabled;			// FIFO使用可能ならtrueを返す
	}


	// -------------------------------------------------------------------------------------------　送信、受信

	/// ■■■■■　FIFO送信
	int send (void *bptr, int bsize)
	{
		int ret;
		if (bsize <= 0) return -1;
		if (fd == -1) {
			printf ("cc_fifo.send(): error fifo not opened\n");
			return -1;
		}
		ret = write (fd,bptr,bsize); // 送信
		if (ret == -1) {
			perror("cc_fifo.send(): write() returned error");
		} else {
			CC_FIFO_DBGPR ("cc_fifo.send(): %d sended\n", ret);
		}
		return ret;
	}

	/// ■■■■■　FIFO受信
	int recv (void *bptr, int bsize)
	{
		int ret;
		if (bsize <= 0) return -1;
		if (fd == -1) {
			printf ("cc_fifo.recv(): error fifo not opened\n");
			return -1;
		}
		ret = read (fd,bptr,bsize); // 受信
		if (ret == -1) {
			perror("cc_fifo.recv(): read() returned error");
		} else {
			CC_FIFO_DBGPR ("cc_fifo.recv(): %d recved\n", ret);
		}
		return ret;
	}
};

#endif // __CC_FIFO_H__


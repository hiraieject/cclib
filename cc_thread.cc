#include "cc_thread.h"

// =====================================================================================
// ===================================================================================== THREAD
// =====================================================================================

void *
cc_thread::thread_func_template (void *arg)
{
	//　以下テンプレート、継承先のクラスでは　thread_func() で定義する

	cc_thread *myobj = (cc_thread *)arg;
	struct timeval timeout;
	fd_set	rfds;

	while (myobj->thread_loop) {
        // タイムアウト値の設定
        timeout.tv_sec	= 0;
		timeout.tv_usec	= 200*1000;

		FD_ZERO(&rfds);
		FD_SET(myobj->fifo_to_thread->get_fd(), &rfds);
		select(FD_SETSIZE, &rfds, 0, 0, &timeout);		// wait

		if( FD_ISSET(myobj->fifo_to_thread->get_fd(), &rfds) ) {
			int recvsize;
			unsigned char buf[100];
			recvsize = myobj->fifo_to_thread->recv (buf, sizeof(buf));
			printf ("recv fifo event from main (%dbyte)\n", recvsize);
		}
	}
	pthread_exit (NULL);
	return NULL;
}

// -------------------------------------------------------------------------------------------

cc_thread::cc_thread (bool thread_up_flg, void * (*start_routine)(void *))
{
	CC_THREAD_DBGPR ("cc_thread: instance created\n");

	// initialize
	thread_id			= 0;;
	thread_loop			= false;

	// create fifo
	fifo_to_thread    = new cc_fifo ();
	fifo_from_thread  = new cc_fifo ();
	
	// thread
	if (thread_up_flg) {
		thread_up (start_routine);
	}
}

cc_thread::~cc_thread ()
{
	thread_down ();

	delete fifo_to_thread;
	delete fifo_from_thread;

	CC_THREAD_DBGPR ("cc_thread: instance deleted\n");
}

// -------------------------------------------------------------------------------------------

bool
cc_thread::thread_up (void * (*start_routine)(void *))
{
	if (thread_id == 0) {
		// wakeup thread
		thread_loop = true;
		if (pthread_create (&thread_id, NULL, start_routine, (void*)this) != 0) {
			thread_loop = false;
			perror("cc_thread: pthread_create() error");
			return false;
		}
		// joinするので、detachしない
		//pthread_detach (thread_id);
	}
	return true;
}

bool
cc_thread::thread_down(void)
{
	if (thread_id != 0) {
		thread_loop = false;

		CC_THREAD_DBGPR ("start pthread join\n");
		pthread_join (thread_id, NULL);
		CC_THREAD_DBGPR ("success pthread join\n");

		thread_id = 0;
	}
	return true;
}

// -------------------------------------------------------------------------------------------

int
cc_thread::send_to_thread (void *dptr, int dsize)
{
	return fifo_to_thread->send (dptr, dsize);
}
int
cc_thread::recv_to_thread (void *bptr, int bsize)
{
	return fifo_to_thread->recv (bptr, bsize);
}

int
cc_thread::send_from_thread (void *dptr, int dsize)
{
	return fifo_from_thread->send (dptr, dsize);
}
int
cc_thread::recv_from_thread (void *bptr, int bsize)
{
	return fifo_from_thread->recv (bptr, bsize);
}

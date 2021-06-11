#include "cc_udpcomm.h"

#define UDPCOMM_DBGPR

#if !defined(UDPCOMM_DBGPR) || !defined(__linux)
#undef DBGPR
#undef ERRPR
#undef WARNPR
#define DBGPR(fmt, args...)
#define ERRPR(fmt, args...)
#define WARNPR(fmt, args...)

#else 
#undef DBGPR
#undef ERRPR
#undef WARNPR
#define DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt, __FILE__,__FUNCTION__,__LINE__,## args); }
#define ERRPR(fmt, args...) \
	{ printf("[%s:%s():%d] ##### ERROR!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#define WARNPR(fmt, args...)											\
	{ printf("[%s:%s():%d] ##### WARNING!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#endif


// =====================================================================================
// ===================================================================================== SEND
// =====================================================================================

cc_udpsend::cc_udpsend (void)
{
	DBGPR ("called()\n");
	connected = false;
	sock = -1;
}
	
cc_udpsend::~cc_udpsend ()
{
	DBGPR ("called()\n");
	disconnect();
	DBGPR ("udpsend deleted\n");
}

bool
cc_udpsend::connect (unsigned int port, in_addr_t ipaddr)
{
	// address
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// create socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		DBGPR ("error socket()\n");
		disconnect ();
		return false;
	}
	// connect success
	connected = true;
	return true;
}
bool
cc_udpsend::disconnect (void) {
	if (sock != -1) {
		close(sock);
		sock = -1;
	}
	connected = false;
	return true;
}
bool
cc_udpsend::getstatus (void) {
	return connected;
}

bool
cc_udpsend::send (unsigned char *dptr, int dsize)
{
	//printf ("#### send data %dbyte\n", dsize);
	sendto(sock, dptr, dsize, 0, (struct sockaddr *)&addr, sizeof(addr));
	return true;
}

// =====================================================================================
// ===================================================================================== RECV
// =====================================================================================

pthread_t cc_udprecv::thread_id;				//!< スレッドID
int cc_udprecv::msgQId;

void *cc_udprecv::thread_func(void *arg)
{
	struct timeval timeout;
	cc_udprecv *urecvp = (cc_udprecv*)arg;
	fd_set	rfds;
	int rcvsize;
	
	while (urecvp->threadloop) {
        // タイムアウト値の設定
        timeout.tv_sec	= 0;
		timeout.tv_usec	= 250*1000;

		FD_ZERO(&rfds);
		FD_SET(urecvp->fifo, &rfds);
		if (urecvp->sock != -1) {
			FD_SET(urecvp->sock, &rfds);
		}
		select(FD_SETSIZE, &rfds, 0, 0, &timeout);		// wait

		if( FD_ISSET(urecvp->fifo, &rfds) ) {
			DBGPR ("recv fifo\n");
		}
		if (urecvp->sock != -1) {
			if( FD_ISSET(urecvp->sock, &rfds) ) {
				//DBGPR ("recv sock\n");
				rcvsize = recvfrom (urecvp->sock, urecvp->buffer, urecvp->buffersize, 0, NULL, NULL);
				//if(numrcv == -1) { status = close(recvSocket); break; }
				//printf("received: %s\n", buffer);

				urecvp->datarecv (rcvsize);
			}
		}
	}
	pthread_exit (NULL);
	
	return NULL;
}

cc_udprecv::cc_udprecv (void)
{
	int msgQId;
	char fifoname[32];

	DBGPR ("called()\n");

	buffer = NULL;
	sock = -1;

	// create fifo
 	msgQId = msgget( IPC_PRIVATE , 0666 | IPC_CREAT );
	snprintf(fifoname, 4048, "/tmp/fifo.%d", msgQId);
	if( (mkfifo(fifoname, 0666) < 0) && (errno != EEXIST) ) {
		perror("mkfifo error");
		return;
	}
	fifo = open(fifoname, O_RDONLY | O_NONBLOCK, 0);

	// wakeup thread
	threadloop = true;
	pthread_create (&thread_id, NULL, cc_udprecv::thread_func, this);

	// joinしない場合はdetachしておく
	//pthread_detach (thread_id); ->joinする
}

cc_udprecv::~cc_udprecv ()
{
	DBGPR ("called()\n");

	// thread close
	if (threadloop == true) {
		threadloop = false;
		// send fifo
		DBGPR ("start pthread join\n");
		pthread_join (thread_id, NULL);
		DBGPR ("success pthread join\n");
	}
	
	// detache resource
	disconnect ();
	if (fifo != -1) close (fifo);

	DBGPR ("udprecv deleted\n");
}

bool
cc_udprecv::connect (unsigned int port, in_addr_t ipaddr, int bsize)
{
	DBGPR ("called()\n");

	// 接続済みなら切断
	disconnect ();

	// create buffer
	if ((buffer = (unsigned char*)malloc (bsize)) == NULL) {
		perror("buffer malloc error");
		return false;
	}
	buffersize = bsize;
	
	// address
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// create socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		ERRPR ("error socket()\n");
		disconnect ();
		return false;
	}
	
	// bind
	if (bind(sock, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
		ERRPR ("error bind()\n");
		disconnect ();
		return false;
	}
	
	connected = true;
	return true;
}

bool
cc_udprecv::disconnect (void)
{
	DBGPR ("called()\n");

	// disconnect
	if (sock != -1) {
		close (sock);
		sock = -1;
	}
	if (buffer != NULL) {
		free (buffer);
		buffer = NULL;
	}
	connected = false;
	return true;
}

bool
cc_udprecv::getstatus (void)
{
	return connected;
}

void
cc_udprecv::datarecv (int rcvsize)
{
	// test code
	if (rcvsize > 0) {
		//printf ("#### recv data [%s] %dbyte\n", buffer, rcvsize);
		printf ("#### recv data %dbyte\n", rcvsize);
	} else {
		printf ("#### connection closed\n");
	}
}



	

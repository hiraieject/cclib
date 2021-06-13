#include "cc_udpcomm.h"

//#define CC_UDPCOMM_DBGPR

#if !defined(CC_UDPCOMM_DBGPR) || !defined(__linux)
#undef CC_UDPCOMM_DBGPR
#undef CC_UDPCOMM_ERRPR
#undef CC_UDPCOMM_WARNPR
#define CC_UDPCOMM_DBGPR(fmt, args...)
#define CC_UDPCOMM_ERRPR(fmt, args...)
#define CC_UDPCOMM_WARNPR(fmt, args...)

#else 
#undef CC_UDPCOMM_DBGPR
#undef CC_UDPCOMM_ERRPR
#undef CC_UDPCOMM_WARNPR
#define CC_UDPCOMM_DBGPR(fmt, args...)	\
	{ printf("[%s:%s():%d] " fmt, __FILE__,__FUNCTION__,__LINE__,## args); }
#define CC_UDPCOMM_ERRPR(fmt, args...) \
	{ printf("[%s:%s():%d] ##### ERROR!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#define CC_UDPCOMM_WARNPR(fmt, args...)											\
	{ printf("[%s:%s():%d] ##### WARNING!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#endif


// =====================================================================================
// ===================================================================================== SEND
// =====================================================================================

cc_udpsend::cc_udpsend (void)
{
	CC_UDPCOMM_DBGPR ("cc_udpsend: instance created\n");

	// initialize
	connected = false;
	sock = -1;
}
	
cc_udpsend::~cc_udpsend ()
{
	disconnect();

	CC_UDPCOMM_DBGPR ("cc_udpsend: instance deleted\n");
}

bool
cc_udpsend::connect (unsigned int port, in_addr_t ipaddr, bool bcast)
{
	disconnect ();

	// create socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		CC_UDPCOMM_DBGPR ("error socket()\n");
		return false;
	}
	// set broadcast flag
	if (bcast) {
		int yes = 1;
		setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char *)&yes, sizeof(yes));
 	}
	// get IPv4 IP address
	{
		struct in_addr in;
		get_myip (in);
		printf ("cc_udpsend: myip %s-%s\n" , "eth0" , inet_ntoa(in));
	}
	
	// address
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ipaddr;

	// connect success
	connected = true;
	CC_UDPCOMM_DBGPR ("cc_udpsend: connected\n");
	return true;
}
bool
cc_udpsend::disconnect (void) {
	if (sock != -1) {
		close(sock);
		sock = -1;
	}
	if (connected) {
		CC_UDPCOMM_DBGPR ("cc_udpsend: disconnected\n");
		connected = false;
	}
	return true;
}
bool
cc_udpsend::get_status (void) {
	return connected;
}
void
cc_udpsend::get_myip (struct in_addr &in) {
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , "eth0" , 30);
	ioctl(sock, SIOCGIFADDR, &ifr);
	in = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	//printf ("myip %s-%s\n" , "eth0" , inet_ntoa(in));
}

ssize_t
cc_udpsend::send (unsigned char *dptr, int dsize)
{
	ssize_t ret = sendto(sock, dptr, dsize, 0, (struct sockaddr *)&addr, sizeof(addr));
	if (ret == -1) {
		perror("#### error sendto()");
	} else {
		CC_UDPCOMM_DBGPR ("cc_udpsend: send udp packet retcode=%ldbyte\n", ret);
	}
	return ret;
}

// =====================================================================================
// ===================================================================================== RECV
// =====================================================================================

void *
cc_udprecv::thread_func(void *arg)
{
	cc_udprecv *myobj = (cc_udprecv *)arg;
	struct timeval timeout;
	fd_set	rfds;
	ssize_t ret;

	struct sockaddr_in from;
	socklen_t sockaddr_in_size = (socklen_t)sizeof(struct sockaddr_in);
	
	while (myobj->thread_loop) {
        // タイムアウト値の設定
        timeout.tv_sec	= 0;
		timeout.tv_usec	= 200*1000;

		FD_ZERO(&rfds);
		FD_SET(myobj->fifo_to_thread->get_fd(), &rfds);
		if (myobj->sock != -1) {
			FD_SET(myobj->sock, &rfds);
		}
		select(FD_SETSIZE, &rfds, 0, 0, &timeout);		// wait

		if( FD_ISSET(myobj->fifo_to_thread->get_fd(), &rfds) ) {
			int recvsize;
			unsigned char buf[200];
			recvsize = myobj->fifo_to_thread->recv (buf, sizeof(buf));
			printf ("cc_udpsend: recv fifo event (%d byte)\n", recvsize);
		}
		if (myobj->sock != -1) {
			if( FD_ISSET(myobj->sock, &rfds) ) {
				
				if ((ret = recvfrom (myobj->sock, myobj->buffer, myobj->buffersize, 0, (struct sockaddr *)&from, &sockaddr_in_size)) == -1) {
					perror("#### error recvfrom()");
				} else {
					CC_UDPCOMM_DBGPR ("cc_udpsend: recv udp packet   retcode=%ldbyte\n", ret);
				}
				myobj->datarecv (ret, from);
			}
		}
	}
	pthread_exit (NULL);
	return NULL;
}

cc_udprecv::cc_udprecv (void) : cc_thread (true/*thread_up_flg*/,thread_func)
{
	CC_UDPCOMM_DBGPR ("cc_udrecv: instance created\n");

	// initialize
	buffer = NULL;
	sock = -1;
	connected = false;

	// crate sub thread
	thread_up (thread_func);
}

cc_udprecv::~cc_udprecv ()
{
	// detache resource
	disconnect ();
	thread_down ();

	CC_UDPCOMM_DBGPR ("cc_udprecv: instance deleted\n");
}

bool
cc_udprecv::connect (unsigned int port, in_addr_t ipaddr, int bsize)
{
	disconnect ();

	// create buffer
	if ((buffer = (unsigned char*)malloc (bsize)) == NULL) {
		perror("#### error malloc()");
		return false;
	}
	buffersize = bsize;
	
	// create socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		perror("#### error socket()");
		disconnect ();
		return false;
	}
	
	// get IPv4 IP address
	{
		struct in_addr in;
		get_myip (in);
		printf ("cc_udprecv: myip %s-%s\n" , "eth0" , inet_ntoa(in));
	}
	
	// address
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = ipaddr;

	// bind
	if (bind(sock, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
		perror("#### error bind()");
		disconnect ();
		return false;
	}
	
	connected = true;
	CC_UDPCOMM_DBGPR ("cc_udprecv: connected\n");
	return true;
}

bool
cc_udprecv::disconnect (void)
{
	// disconnect
	if (sock != -1) {
		close (sock);
		sock = -1;
	}
	if (buffer != NULL) {
		free (buffer);
		buffer = NULL;
	}
	if (connected) {
		CC_UDPCOMM_DBGPR ("cc_udprecv: disconnected\n");
		connected = false;
	}
	return true;
}

bool
cc_udprecv::get_status (void)
{
	return connected;
}
void
cc_udprecv::get_myip (struct in_addr &in) {
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = AF_INET;
	strncpy(ifr.ifr_name , "eth0" , 30);
	ioctl(sock, SIOCGIFADDR, &ifr);
	in = ((struct sockaddr_in *)&ifr.ifr_addr)->sin_addr;
	//printf ("myip %s-%s\n" , "eth0" , inet_ntoa(in));
}

void
cc_udprecv::datarecv (ssize_t rcvsize, struct sockaddr_in from)
{
	// test code
	if (rcvsize > 0) {
		//printf ("#### recv data [%s] %dbyte\n", buffer, rcvsize);
		CC_UDPCOMM_DBGPR ("cc_udprecv: recv data %ldbyte from %s:%d\n", rcvsize, inet_ntoa(from.sin_addr), ntohs(from.sin_port));
	} else {
		CC_UDPCOMM_DBGPR ("cc_udprecv: connection closed\n");
	}
}

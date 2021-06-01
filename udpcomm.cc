#include "udpcomm.h"

#define UDPCOMM_DBGPR

#if defined(UDPCOMM_DBGPR) || !defined(__linue)
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

udpsend::udpsend (unsigned int port, char *ipaddr)
{
	// address
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	// create socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		DBGPR ("error socket()\n", dsize);
		return;
	}
}
	
udpsend::~udpsend ()
{
	if (sock == -1) return;

	close(sock);
	sock = -1;
}

void udpsend::send (unsigned char *dptr, int dsize)
{
	sendto(sock, dptr, dsize, 0, (struct sockaddr *)&addr, sizeof(addr));
}

// =====================================================================================
// ===================================================================================== RECV
// =====================================================================================

pthread_t udprecv::thread_id;				//!< スレッドID
int udprecv::msgQId;

void *udprecv::thread_func(void *arg)
{
	return NULL;
}

udprecv::udprecv (unsigned int port, char *ipaddr)
{
	// address
	memset(&addr, 0, sizeof(addr));
	addr.sin_port = htons(port);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// create socket
	if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		DBGPR ("error socket()\n", dsize);
		return;
	}
	
	// bind
	if (bind(sock, (const struct sockaddr *) &addr, sizeof(addr)) == -1) {
		DBGPR ("error bind()\n", dsize);
		return;
	}

	pthread_create (&thread_id, NULL, udprecv::thread_func, NULL);
}

udprecv::~udprecv ()
{
	if (sock == -1) return;

	close(sock);
	sock = -1;
}
 
void udprecv::datarecv (unsigned char *dptr, int dsize)
{
	DBGPR ("[udprecv] recv %d byte\n", dsize);
}
	

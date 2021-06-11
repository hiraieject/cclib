/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

/*
 * udp communication lib for C++  by Yoshikazu Hirai
 */

#ifndef __CC_UDPCOMM_H__
#define __CC_UDPCOMM_H__

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

class cc_udpsend {
private:
	int sock;
	struct sockaddr_in addr;
	bool connected;
public:
	cc_udpsend (void);
	virtual ~cc_udpsend ();		// 継承用なのでvirtual

	bool connect (unsigned int port, in_addr_t ipaddr);
	bool disconnect (void);
	bool getstatus (void);
	
	bool send (unsigned char *dptr, int dsize);
};

class cc_udprecv {
private:
	int sock;
	struct sockaddr_in addr;
	bool connected;
	bool threadloop;
	int	 fifo;

	static pthread_t thread_id;
	static int msgQId;
	static void *thread_func(void *arg);
public:
	int  buffersize;
	unsigned char *buffer;

	cc_udprecv (void);
	virtual ~cc_udprecv ();		// 継承用なのでvirtual

	virtual bool connect (unsigned int port, in_addr_t ipaddr, int bsize=1024);
	virtual bool disconnect (void);
	virtual bool getstatus (void);
	
	virtual void datarecv (int rcvsize);
};

#endif// __CC_UDPCOMM_H__


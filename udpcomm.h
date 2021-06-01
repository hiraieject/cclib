/* -*- Mode: C; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __UDPCOMM_H__
#define __UDPCOMM_H__

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>

class udpsend {
private:
	int sock;
	struct sockaddr_in addr;
	bool connected;
public:
	udpsend (unsigned int port, char *ipaddr);
	~udpsend ();

	void send (unsigned char *dptr, int dsize);
};

class udprecv {
private:
	int sock;
	struct sockaddr_in addr;
	bool connected;

	static pthread_t thread_id;				//!< スレッドID
	static int msgQId;
	static void *thread_func(void *arg);
public:
	udprecv (unsigned int port, char *ipaddr);
	~udprecv ();

	virtual void datarecv (unsigned char *dptr, int dsize);
};

#endif// __UDPCOMM_H__


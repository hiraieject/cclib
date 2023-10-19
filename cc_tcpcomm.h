/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file cc_tcpcomm.h
 * @brief tcp communication class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_TCPCOMM_H__
#define __CC_TCPCOMM_H__

#include <vector>

#include "cc_thread.h"

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
#include <sys/ioctl.h>
#include <net/if.h>

///
/// ■■■■■　TCP 送信クラス
///
class cc_tcpconnect : public cc_thread {

public:
	int clientSocket;
	struct sockaddr_in clientAddress;

    cc_tcpconnect (key_t message_key, std::string nickname);
    //cc_tcpconnect (void);
	virtual ~cc_tcpconnect ();

    virtual void thread_main (void);
    
    bool exec_server_connect (unsigned int port, const char *addrstr);
	bool exec_server_connect (unsigned int port, in_addr_t ipaddr);
	bool exec_disconnect (void);
	ssize_t exec_send (unsigned char *dptr, int dsize);
	ssize_t exec_recv (unsigned char *dptr, int dsize);

    int get_fd (void);
	bool get_status (void);
	void get_ip (struct in_addr &in);
	void get_ifinfo (struct ifreq &ifr);	// inet_ntoa((&(ifr.ifr_addr))->sin_addr)
											// inet_ntoa((&(ifr.ifr_broadaddr))->sin_addr)
											// inet_ntoa((&(ifr.ifr_netmask))->sin_addr)
											// inet_ntoa((&(ifr.ifr_hwaddr))->sin_addr)

    cc_debugprint tcpcomm_dbg;              /// cclib debugprint
};

///
/// ■■■■■　TCP 送信クラス
///
class cc_tcpserver : public cc_thread {
    
private:
	int serverSocket;
	struct sockaddr_in serverAddress;
    int max_connection;
    std::vector<cc_tcpconnect*> connections;

public:
	//cc_tcpserver (void);
    cc_tcpserver (key_t message_key, std::string nickname);
	virtual ~cc_tcpserver ();

    virtual void thread_main (void);
    
	bool start_server (unsigned int port, int cmax=1);
	bool stop_server (void);

    virtual cc_tcpconnect *create_conn (void);
    
	bool get_server_status (void);
	void get_ip (struct in_addr &in);
	void get_ifinfo (struct ifreq &ifr);	// inet_ntoa((&(ifr.ifr_addr))->sin_addr)
											// inet_ntoa((&(ifr.ifr_broadaddr))->sin_addr)
											// inet_ntoa((&(ifr.ifr_netmask))->sin_addr)
											// inet_ntoa((&(ifr.ifr_hwaddr))->sin_addr)

    cc_debugprint tcpcomm_dbg;              /// cclib debugprint
};

#define CC_TCPCOMM_ERRPR(fmt, args...) \
	{ printf("[%s:%s():%d] ##### ERROR!: " fmt,tcpcomm_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
#define CC_TCPCOMM_WARNPR(fmt, args...) \
	{ printf("[%s:%s():%d] ##### WARNING!: " fmt,tcpcomm_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
#define CC_TCPCOMM_DBGPR(fmt, args...) \
	if (tcpcomm_dbg.enable_flg) { printf("[%s:%s():%d] " fmt,tcpcomm_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }

#endif// __CC_TCPCOMM_H__


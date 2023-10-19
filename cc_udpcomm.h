/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file cc_udpcomm.h
 * @brief udp communication class for C++
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
#include <sys/ioctl.h>
#include <net/if.h>

#include "cc_thread.h"

#undef CC_UDPCOMM_DBGPR
#undef CC_UDPCOMM_ERRPR
#undef CC_UDPCOMM_WARNPR
#define CC_UDPCOMM_ERRPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### ERROR!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }
#define CC_UDPCOMM_WARNPR(fmt, args...)                                         \
    { printf("[%s:%s():%d] ##### WARNING!: " fmt, __FILE__,__FUNCTION__,__LINE__, ## args); }

#if !defined(CC_UDPCOMM_ENB_DBGPR)
#define CC_UDPCOMM_DBGPR(fmt, args...)
#else 
#define CC_UDPCOMM_DBGPR(fmt, args...)  \
    { printf("[%s:%s():%d] " fmt, __FILE__,__FUNCTION__,__LINE__,## args); }
#endif

///
/// ■■■■■　UDP 送信クラス
///
class cc_udpsend {
private:
    int sock;
    struct sockaddr_in addr;
    bool connected;
public:
    cc_udpsend (void);
    virtual ~cc_udpsend ();     // 継承用なのでvirtual

    bool connect (unsigned int port, in_addr_t ipaddr, bool bcast=false);
    bool disconnect (void);
    bool get_status (void);
    void get_ip (struct in_addr &in);
    void get_ifinfo (struct ifreq &ifr);    // inet_ntoa((&(ifr.ifr_addr))->sin_addr)
                                            // inet_ntoa((&(ifr.ifr_broadaddr))->sin_addr)
                                            // inet_ntoa((&(ifr.ifr_netmask))->sin_addr)
                                            // inet_ntoa((&(ifr.ifr_hwaddr))->sin_addr)

    ssize_t send (unsigned char *dptr, int dsize);
};

///
/// ■■■■■　UDP 送信クラス
///
class cc_udprecv : public cc_thread {
    
 private:
    int sock;
    struct sockaddr_in addr;
    bool connected;

 public:
    int  buffersize;
    unsigned char *buffer;

    static void *thread_func (void *arg);
    
    cc_udprecv (void);
    virtual ~cc_udprecv ();     // 継承用なのでvirtual

    bool connect (unsigned int port, in_addr_t ipaddr, int bsize=1024);
    bool disconnect (void);
    bool get_status (void);
    void get_ip (struct in_addr &in);
    void get_ifinfo (struct ifreq &ifr);    // inet_ntoa((&(ifr.ifr_addr))->sin_addr)
                                            // inet_ntoa((&(ifr.ifr_broadaddr))->sin_addr)
                                            // inet_ntoa((&(ifr.ifr_netmask))->sin_addr)
                                            // inet_ntoa((&(ifr.ifr_hwaddr))->sin_addr)

    virtual void datarecv (ssize_t rcvsize, struct sockaddr_in from); // この関数は main thread ではなく thread_func のコンテキストで呼ばれるので注意
};

#endif// __CC_UDPCOMM_H__


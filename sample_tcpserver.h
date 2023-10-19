/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __SAMPLE_TCPSERVER_H__
#define __SAMPLE_TCPSERVER_H__

#include "cc_tcpcomm.h"

#define MSG_KEY_SAMPLE_TCPSERVER_KEY 2001

#define SAMPLE_TCPSERVER_PORT_NO 5000
#define SAMPLE_TCPSERVER_CONNECT_MAX 10

// TCPサーバー、クライアントコネクションクラス
class sample_tcpserver_conn : public cc_tcpconnect {
public:
    void thread_main (void);                // 通信スレッド
    cc_debugprint dbg;                      // cclib debugprint
};

// TCPサーバークラス
class sample_tcpserver : public cc_tcpserver {
public:
    // クライアント通信のクラスは sample_server_conn とし、インスタンスを返す
    cc_tcpconnect *create_conn (void);
    cc_debugprint dbg;                      // cclib debugprint
};

#endif // __SAMPLE_TCPSERVER_H__

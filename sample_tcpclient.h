/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#ifndef __SAMPLE_TCPCLIENT_H__
#define __SAMPLE_TCPCLIENT_H__

#include "cc_tcpcomm.h"

#define MSG_KEY_SAMPLE_TCPCLIENT_KEY 2002

#define SAMPLE_TCPCLIENT_PORT_NO 5000
#define SAMPLE_TCPCLIENT_IP_ADDR "127.0.0.1"
#define SAMPLE_TCPCLIENT_BUFFER_SIZE 256

// SAMPLE_TCPCLIENT 受信クラス
class sample_tcpclient_recv : public cc_tcpconnect {
public:
    void thread_main (void);    // 通信スレッド
    cc_debugprint dbg;                      /// cclib debugprint
};

// SAMPLE_TCPCLIENT キー入力&TCP送信クラス
class sample_tcpclient_send : public cc_thread {
public:
    int clientSocket;
    
    sample_tcpclient_send(void);
    
    void thread_main (void);    // 通信スレッド
    cc_debugprint dbg;                      /// cclib debugprint
};

#endif // __SAMPLE_TCPCLIENT_H__

/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

#include "sample_tcpserver.h"

// カスタムクライアント通信スレッド
void
sample_tcpserver_conn::thread_main (void)
{
    struct timeval timeout;
    fd_set  readSet;

    DBGPR ("cc_tcpserver_conn: thread up\n");
    
    while (loop_continue()) {

        // タイムアウト設定
        timeout.tv_sec  = 0;
        timeout.tv_usec = 100*1000; // 100msec loop

        // 待受対象
        FD_ZERO(&readSet);
        FD_SET(clientSocket, &readSet);

        // select でイベント待ち
        select(FD_SETSIZE, &readSet, 0, 0, &timeout);

        // イベント処理
        if( FD_ISSET(clientSocket, &readSet) ) {
            // クライアントからのデータを受信
            char buffer[1024];
            memset (buffer,0,sizeof(buffer));
            ssize_t bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
            if (bytesRead > 0) {
                // 受信したデータをエコーバック(sample pingpong)
                send(clientSocket, buffer, bytesRead, 0);
                printf (">> %s", buffer);
            } else if (bytesRead == 0) {
                // クライアントが切断されたことを検出
                DBGPR ("cc_tcpserver_conn: client disconnected\n");
                clientSocket = -1;
                thread_loop  = false;
            } else {
                perror("recv() error");
                DBGPR ("cc_tcpserver_conn: recv() error\n");
            }
        }
    }
    DBGPR ("cc_tcpserver_conn: thread down\n");
}

// カスタムクライアント通信クラスのインスタンスを返す,
cc_tcpconnect *
sample_tcpserver::create_conn (void)
{
    return new sample_tcpserver_conn;
}

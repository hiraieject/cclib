/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

#include "sample_tcpclient.h"

// SAMPLE_TCPCLIENT 受信クラス
// 通信スレッド
void
sample_tcpclient_recv::thread_main (void)
{
    struct timeval timeout;
    fd_set  readSet;
    
    printf ("sample_tcpclient_recv: thread up\n");
    
    while (loop_continue()) {

        // タイムアウト設定
        timeout.tv_sec  = 0;
        timeout.tv_usec = 1000*1000; // 1sec loop

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
                printf (">> %s", buffer);
            } else if (bytesRead == 0) {
                // クライアントが切断されたことを検出
                DBGPR ("sample_tcpclient_recv: disconnected\n");
                clientSocket = -1;
                thread_loop  = false;
            } else {
                perror("recv() error");
                DBGPR ("sample_tcpclient_recv: recv() error\n");
            }
        }
    }
    DBGPR ("sample_tcpclient_recv: thread down\n");
}

// SAMPLE_TCPCLIENT キー入力&TCP送信クラス

sample_tcpclient_send::sample_tcpclient_send(void) :
    cc_thread (MSG_KEY_SAMPLE_TCPCLIENT_KEY,
               sizeof(message_basic), "sample_tcpclient")
{
    clientSocket = -1;;

    // debugprint default off
    dbg.enable("sample_tcpclient_send");
}

// 通信スレッド
void
sample_tcpclient_send::thread_main (void)
{
    char inputBuffer[SAMPLE_TCPCLIENT_BUFFER_SIZE];

    printf ("sample_tcpclient_send: thread up\n");

    sleep(1);
    printf ("-------------------------------------------------\n");
    printf ("Please input [quit] or [exit] to close connection\n");
        
    while (loop_continue()) {

        memset(inputBuffer, 0, sizeof(inputBuffer));
        std::cin.getline(inputBuffer, sizeof(inputBuffer));
        if (strcmp(inputBuffer,"quit") == 0
            || strcmp(inputBuffer,"exit") == 0) {
            clientSocket = -1;  // 通信終了状態に遷移
            break;
        }
        strcat (inputBuffer, "\n"); // 開業を付加
        //printf ("<< %s len=%d", inputBuffer, (int)strlen(inputBuffer));
        if (clientSocket != -1)
            send(clientSocket, inputBuffer, strlen(inputBuffer), 0);
    }
    printf ("sample_tcpclient_send: thread down\n");
}


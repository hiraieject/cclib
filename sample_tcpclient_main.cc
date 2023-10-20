/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

#include <iostream>
#include <chrono>
#include <csignal>
#include <unistd.h>

#include "sample_tcpclient.h"

extern "C" {
#include <getopt.h>
}

static volatile bool process_keep = true;

// カスタムのシグナルハンドラ関数
void signalHandler(int signum) {
    printf ("signal %d recved\n", signum);           
    process_keep = false;
}

// コマンドラインヘルプを表示する
void help (char *argv0)
{
    printf ("%s [-i <ip address>] [-p <port no>]\n", argv0);
    printf ("   default ip addr is localhost\n");
    printf ("   default port no is %d\n", SAMPLE_TCPCLIENT_PORT_NO);
}

// メイン関数
int
main (int argc, char *argv[])
{
    // シグナルハンドラの設定
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    //  SIGKILLはキャッチ不可能なので設定なし

    char *ip_addr = (char*)SAMPLE_TCPCLIENT_IP_ADDR;
    unsigned int port_no = SAMPLE_TCPCLIENT_PORT_NO;
    
    // 引数の読み込み
    int opt;
    while ((opt = getopt(argc, argv, "p:i:")) != -1) {
        switch (opt) {
        case 'p':
            if ((port_no = atoi(optarg)) == 0) {
                fprintf (stderr, "error: invalid port no : -p %s\n", optarg);
                exit(1);
            }
            break;
        case 'i':
            ip_addr = optarg;
                break;
                
        case 'h':
            help (argv[0]);
            return 0;
        case '?':
            // 未知のオプションが指定された場合の処理
            printf("ERROR: invalid option : -%c\n", optopt);
            help (argv[0]);
            return 0;
        }
    }
    // getoptで処理された引数を除いた残りの引数
    for (int i = optind; i < argc; ++i) {
        printf("ERROR: invalid argment argv[%d] = %s\n", i, argv[i]);
        help (argv[0]);
        return 1;
    }
    printf("\n");

    // サーバー情報の表示
    printf (" SERVER IPADDR : %s\n", ip_addr);
    printf (" SERVER PORT   : %d\n", port_no);

    // クライアントインスタンスの宣言
    sample_tcpclient_recv client_recv;
    sample_tcpclient_send client_send;

    // サーバーに接続
    printf ("now wakeup server\n");
    if (client_recv.exec_server_connect (port_no, (const char*)ip_addr) == false) {
        printf("ERROR: failed connect server, process exit now.\n");
        return 1;
    }
    client_send.clientSocket = client_recv.clientSocket;

    // 入力＆送信スレッド
    client_send.thread_up();
    client_send.thread_detach(); // ブロックされるので、終了を待たない

    // 終了コマンド待ち
    while (process_keep
           && client_recv.clientSocket != -1
           && client_send.clientSocket != -1) {

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100msec loop
    }

    printf ("process main down\n");

    // clientインスタンスの寿命終了からのデストラクタですべての後処理が行われる
    // exit()で抜けると後処理が行われないので注意
    return 0;
}

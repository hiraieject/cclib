/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

#include <iostream>
#include <chrono>
#include <csignal>
#include <unistd.h>

#include "sample_tcpserver.h"

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
    printf ("%s [-p <port no>] [-m <max connection>]\n", argv0);
    printf ("   default port no is %d\n", SAMPLE_TCPSERVER_PORT_NO);
    printf ("   default max conn is %d\n", SAMPLE_TCPSERVER_CONNECT_MAX);
}

// メイン関数
int
main (int argc, char *argv[])
{
    // シグナルハンドラの設定
    signal(SIGTERM, signalHandler);
    signal(SIGINT,  signalHandler);
    //  SIGKILLはキャッチ不可能なので設定なし

    // 変数の宣言/初期化
    unsigned int port_no = SAMPLE_TCPSERVER_PORT_NO;
    int connect_max      = SAMPLE_TCPSERVER_CONNECT_MAX;

    process_keep         = true;
    
    // 引数の読み込み
    int opt;
    while ((opt = getopt(argc, argv, "p:m:h")) != -1) {
        switch (opt) {
        case 'p':
            if ((port_no = atoi(optarg)) == 0) {
                printf ("ERROR: invalid port no : -p %s\n", optarg);
                return 1;
            }
            break;
        case 'm':
            if ((connect_max = atoi(optarg)) == 0) {
                printf ("ERROR: invalid connect max : -m %s\n", optarg);
                return 1;
            }
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
    printf (" SERVER PORT : %d\n", port_no);
    printf (" CONNECT MAX : %d\n", connect_max);
    
    // サーバーインスタンスの宣言
    sample_tcpserver server;

    // サーバー実行
    printf ("now wakeup server\n");
	if (server.start_server (port_no, connect_max) == false) {
        printf ("ERROR: detect server start failed, process exit now.\n");
        return 1;
    }

    // 終了コマンド待ち
    while (process_keep) {

        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100msec loop
    }
    
    printf ("process main down\n");

    // serverインスタンスの寿命終了からのデストラクタですべての後処理が行われる
    // exit()で抜けると後処理が行われないので注意
    return 0;
}

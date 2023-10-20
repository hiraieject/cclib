/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

#include <iostream>
#include <chrono>
#include <csignal>
#include <unistd.h>

#include <sys/time.h>

#include "sample_tcpclient.h"

volatile bool process_keep = true;

// カスタムのシグナルハンドラ関数
void abort_handler(int signum) {
#if 1
    switch (signum) {
    case SIGTERM:
        printf ("[sample_signalhandler] SIGTERM (%d) recved\n", signum);
        break;
    case SIGINT:
        printf ("[sample_signalhandler] SIGINT (%d) recved\n", signum);
        break;
    default:
        break;
    }
#endif
    process_keep = false;
}
void alarm_handler(int signum) {
#if 1
    switch (signum) {
    case SIGALRM:
        printf ("[sample_signalhandler] SIGALARM (%d) recved\n", signum);
        break;
    case SIGUSR1:
        printf ("[sample_signalhandler] SIGUSR1 (%d) recved\n", signum);
        break;
    default:
        break;
    }
#endif
    {
        // 次回アラームシグナル呼び出しの設定（1秒後）
        struct itimerval timer;
        timer.it_value.tv_sec = 1;
        timer.it_value.tv_usec = 0;
        timer.it_interval = timer.it_value;
        setitimer(ITIMER_REAL, &timer, NULL);
    }
}

// メイン関数
int
main (int argc, char *argv[])
{
    // 終了系のシグナルハンドラを設定
    struct sigaction sa_term, sa_int;

    sa_term.sa_handler = abort_handler;
    sa_term.sa_flags = 0;

    if (sigaction(SIGTERM, &sa_term, NULL) == -1) {
        perror("sigaction for SIGTERM");
        return 1;
    }
    if (sigaction(SIGINT, &sa_term, NULL) == -1) {
        perror("sigaction for SIGINT");
        return 1;
    }

    // ALARMシグナルハンドラを設定
    sa_term.sa_handler = alarm_handler;
    sa_term.sa_flags = 0;

    if (sigaction(SIGALRM, &sa_term, NULL) == -1) {
        perror("sigaction for SIGALRM");
        return 1;
    }
    if (sigaction(SIGUSR1, &sa_term, NULL) == -1) {
        perror("sigaction for SIGUSR1");
        return 1;
    }
    kill(getpid(), SIGUSR1);  // SIGUSR1を投げて初回タイマー設定する

    //  SIGKILLはキャッチ不可能なので設定なし

    // 終了コマンド待ち
    while (process_keep) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // 100msec loop
    }
    
    return 0;
}

/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_pipeexec.h
 * @brief pipe exec class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */

#ifndef __CC_PIPEEXEC_H__
#define __CC_PIPEEXEC_H__

#define AVAILABLE_PIPEEXEC

#ifdef __MINGW64__
#include "cc_pipeexec_mingw64.h"
#undef AVAILABLE_PIPEEXEC
#define AVAILABLE_PIPEEXEC_MINGW64
#endif

#ifdef AVAILABLE_PIPEEXEC

#include <iostream>
#include <cstdio>
#include <vector>
#include <stdexcept>
#include <string>
#include <mutex>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h> // waitpid()関数を使用するために必要

class cc_pipeexec {
public:
    std::mutex mtx;                     ///< 変数保護用MUTEX @n 変数を直接参照する時にはロックすること
    pid_t pid;

    // ------------------- constructor/destructor/内部関数
    cc_pipeexec(void) : pid(0), pipe_fd(-1), outputModified(false) {}
    
    // ------------------- ユーザー向け public methods

    /// コマンド実行
    bool executeCommand(std::string &cmd)
    {
        int pipes[2];             // パイプ

        // 配列を初期化する
        {
            std::lock_guard<std::mutex> lock(mtx); // mutexをロック
            outputLines.clear();
            outputModified = false;
        }

        if (pipe(pipes) == -1) {
            throw std::runtime_error("pipe(): Failed to pipe create");
        }
        pid = fork();
        if( pid == -1 ) {
            close(pipes[0]);
            close(pipes[1]);
            throw std::runtime_error("fork(): returnee error");
        }

        if( pid == 0 ) {

            // 子プロセス
            // --------------------
            close(1);             // 子プロセスの標準出力を閉じて、
            if (dup(pipes[1]) == -1) {
                perror("dup(): returned error");
                throw std::runtime_error("dup(): returnee error");
            }                     // パイプを標準出力に複製する。
            close(2);             // 子プロセスの標準エラー出力も閉じて、
            if (dup(pipes[1]) == -1) {
                perror("dup(): returned error");
                throw std::runtime_error("dup(): returnee error");
            }                     // パイプを標準エラー出力に複製する。
            
            // パイプを閉じる
            close(pipes[0]);
            close(pipes[1]);

            {
                // 引数リストを格納する配列を初期化
                char *args[20]; // 最大引数数を適切に調整してください
                int argCount = 0; // 引数の数をカウント
                char *p = strdup(cmd.c_str()); // cmdがconstだと失敗するのでコピー

                // スペースを区切り文字としてトークンを分割
                char *token = strtok(p, " ");
                while (token != NULL) {
                    args[argCount] = token;
                    argCount++;
                    if (argCount >= (int)sizeof(args) / (int)sizeof(args[0])) {
                        fprintf(stderr, "\n");
                        close(pipes[1]);
                        throw std::runtime_error("execvp(): too many argment");
                    }
                    token = strtok(NULL, " ");
                }
                // 引数リストの末尾にヌルポインタを追加
                args[argCount] = NULL;

                // execlを使用して新しいプログラムを実行
                if (execvp(args[0], args) == -1) {
                    perror("execvp");
                    close(pipes[1]);
                    throw std::runtime_error("execvp(): failed");
                }
            }
        }
        // 親プロセス
        close(pipes[1]);          // 親プロセスでは出力しないので閉じる
        pipe_fd = pipes[0];
        return true;
    }
    
    // パイプを閉じる（子プロセス終了後）
    bool pipeClose(void)
    {
        if (pipe_fd == -1) {
            return false;
        }
        close(pipe_fd);
        pipe_fd = -1;
        return true;
    }

    // 子プロセスの終了まちをする
    void finish(pid_t finpid)
    {
        if (finpid == 0) {
            finpid = pid;
            if (pid == 0) return;
        }
        // 子プロセスのPIDを使用してwaitpidを呼び出し、子プロセスの終了を待つ
        int status;
        while (waitpid(finpid, &status, 0) == -1) {
            if (errno != EINTR) { // EINTR以外のエラーの場合
                perror("waitpid");
                break;
            }
        }
        if (pid == finpid) {
            pid = 0;
        }
    }

    int get_fd(void)
    {
        return pipe_fd;
    }
    
    std::vector<std::string> simpleExecuteCommand(std::string &cmd)
    {
        // exception は呼び元で受ける前提
        if (executeCommand(cmd) == false) {
            outputLines.clear();
            return outputLines;
        }

        // 以下、標準出力・エラーを読み取る処理
        fd_set read_fds;
        FD_ZERO(&read_fds);
        FD_SET(pipe_fd, &read_fds);

        std::string partial_line;
        char buffer[1024];

        while (true) {
            FD_SET(pipe_fd, &read_fds);
            if (select(pipe_fd + 1, &read_fds, NULL, NULL, NULL) > 0) {
                if (FD_ISSET(pipe_fd, &read_fds)) {
                    // パイプからデータを読み込む
                    ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer) - 1);
                    if (bytes_read > 0) {
                        buffer[bytes_read] = '\0';
                        partial_line += buffer;
                    
                        size_t newline_pos;
                        while ((newline_pos = partial_line.find('\n')) != std::string::npos) {
                            std::string line = partial_line.substr(0, newline_pos);
                            partial_line.erase(0, newline_pos + 1);
                        
                            // 行単位で処理
                            {
                                std::lock_guard<std::mutex> lock(mtx);
                                outputLines.push_back(line);
                                outputModified = true;
                            }
                        }
                    } else {
                        // パイプの終端に達した
                        break;
                    }
                }
            } else {
                if (errno == EINTR) {
                    // シグナルによる割り込み処理
                    continue;
                } else if (errno == ETIMEDOUT) {
                    // タイムアウト処理
                    continue;
                } else {
                    // その他のエラー処理
                    throw std::runtime_error("select(): returned error");
                }
            }
        }
        pipeClose();

        finish(0);
    
        return outputLines;
    }

private:
    int pipe_fd;
    bool outputModified;
    std::vector<std::string> outputLines;
};

// ============================================================================================== 以下サンプル実装県UNITテスト

#ifdef UNIT_TEST_SAMPLE
#include <unistd.h>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>

int main()
{
    cc_pipeexec pipeexec;
    
    printf ("----------------------- real time\n");
    
    //std::string command1 = "python3 -c \"import time; [print(f'command1: {{i}}') or time.sleep(1) for i in range(5)]\"";
    std::string command1 = "python3 testcommand1.py";
    printf (">> pipeexec.executeCommand (%s)\n", command1.c_str());
    bool ret = pipeexec.executeCommand (command1);
    printf (">> returned %s\n", ret ? "True" : "False");

    bool loop_continue = true;
    while (loop_continue) {

        // pipe_fd 取得
        int pipe_fd = pipeexec.get_fd();
        if (pipe_fd == -1) {
            printf ("invalid pipe_fd\n");
            loop_continue = false;
            continue;
        }
        
        // select() でイベント待ち
        struct timeval timeout = { 0/*sec*/, 100*1000/*usec*/ };
        fd_set	rfds;
        FD_ZERO(&rfds);
        FD_SET(pipe_fd, &rfds);
        select(FD_SETSIZE, &rfds, 0, 0, &timeout);
        
        if (FD_ISSET(pipe_fd, &rfds)) {
            printf ("fdset detect\n");
            
            // パイプからデータを読み込む
            char buffer[1024];
            ssize_t bytes_read = read(pipe_fd, buffer, sizeof(buffer) - 1);
            if (bytes_read > 0) {
                buffer[bytes_read] = '\0';
                printf ("recv(%ld)[%s]\n", bytes_read, buffer);
                
            } else {
                // パイプの終端に達した
                printf ("command1 finished\n");
                loop_continue = false;
                continue;
            }
        }
    }
    pipeexec.pipeClose();

    printf ("----------------------- non real time\n");
    
    std::string command2 = "python3 testcommand1.py";
    printf (">> pipeexec.simpleExecuteCommand(%s)\n", command2.c_str());
    std::vector<std::string> result = pipeexec.simpleExecuteCommand(command2);
    printf ("command2 finished\n");
    
    for (const auto& str : result) {
        std::cout << str << std::endl;
    }
    return 0;
}
#endif // UNIT_TEST_SAMPLE

#endif // AVAILABLE_PIPEEXEC

#endif // __CC_PIPEEXEC_H__

/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_pipeexec.cc
 * @brief pipe exec class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#include <sys/wait.h> // waitpid()関数を使用するために必要

#include "cclib.h"

#ifdef AVAILABLE_PIPEEXEC

cc_pipeexec::cc_pipeexec(void)
{
    pipe_fd = -1;
    pid = 0;
}
    
bool
cc_pipeexec::executeCommand(std::string &cmd)
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

bool
cc_pipeexec::pipeClose(void)
{
    if (pipe_fd == -1) {
        return false;
    }
    close(pipe_fd);
    pipe_fd = -1;
    return true;
}

bool
cc_pipeexec::getOutputAll(std::vector<std::string> &result)
{
    if (pipe_fd == -1) {
        result.clear();
        throw std::runtime_error("get_outputAll(): pipe fd is ZERO error");
    }
    return true;
}
    
int
cc_pipeexec::get_fd(void)
{
    return pipe_fd;
}

void
cc_pipeexec::finish(pid_t finpid)
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

std::vector<std::string>
cc_pipeexec::simpleExecuteCommand(std::string &cmd)
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

//int
//cc_pipeexec::calltest(void)
//{
//    try {
//        std::vector<std::string> output = executeCommand((char*)"ls -l");
//            
//        std::cout << "外部コマンドの標準出力:\n";
//        for (const std::string& line : output) {
//            std::cout << line;
//        }
//            
//    } catch (const std::exception& e) {
//        std::cerr << "エラー: " << e.what() << std::endl;
//        return 1;
//    }
//    return 0;
//}

#endif // AVAILABLE_PIPEEXEC


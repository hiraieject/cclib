/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_pipeexec.h
 * @brief pipe exec class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_PIPEEXEC_H__
#define __CC_PIPEEXEC_H__

#include <iostream>
#include <cstdio>
#include <vector>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <string.h>

class cc_pipeexec {
private:
    bool outputModified;
    std::vector<std::string> outputLines;
public:
    std::mutex mtx;                     ///< 変数保護用MUTEX @n 変数を直接参照する時にはロックすること

    std::vector<std::string> executeCommand(const char* cmd)
    {
        pid_t pid;                // fork() が返す pid
        int pipes[2];             // パイプ
        char buf[1024];
        FILE *fp;

        // 配列を初期化する
        {
            std::lock_guard<std::mutex> lock(mtx); // mutexをロック
            outputLines.clear();
            outputModified = false;
        }

        if (pipe(pipes) == -1) {
            throw std::runtime_error("PIPEの作成に失敗しました");
        }
        pid = fork();
        if( pid == -1 ) {
            close(pipes[0]);
            close(pipes[1]);
            throw std::runtime_error("forkに失敗しました");
        }
        
        if( pid == 0 ) {

            // 子プロセス
            // --------------------
            close(1);             // 子プロセスの標準出力を閉じて、
            if (dup(pipes[1]) == -1) {
                perror("dup() returned error");
            }                     // パイプを標準出力に複製する。
            close(2);             // 子プロセスの標準エラー出力も閉じて、
            if (dup(pipes[1]) == -1) {
                perror("dup() returned error");
            }                     // パイプを標準エラー出力に複製する。
            
            // パイプを閉じる
            close(pipes[0]);
            close(pipes[1]);

            {
                // 引数リストを格納する配列を初期化
                char *args[20]; // 最大引数数を適切に調整してください
                int argCount = 0; // 引数の数をカウント
                char *p = strdup(cmd); // cmdがconstだと失敗するのでコピー

                // スペースを区切り文字としてトークンを分割
                char *token = strtok(p, " ");
                while (token != NULL) {
                    args[argCount] = token;
                    argCount++;
                    if (argCount >= (int)sizeof(args) / (int)sizeof(args[0])) {
                        fprintf(stderr, "\n");
                        close(pipes[1]);
                        throw std::runtime_error("execvp 引数の数が多すぎます。");
                    }
                    token = strtok(NULL, " ");
                }
                // 引数リストの末尾にヌルポインタを追加
                args[argCount] = NULL;

                // execlを使用して新しいプログラムを実行
                if (execvp(args[0], args) == -1) {
                    perror("execvp");
                    close(pipes[1]);
                    throw std::runtime_error("execvpに失敗しました");
                }
            }
        } else {
            
            // 親プロセス
            // --------------------
            close(pipes[1]);          // 親プロセスでは出力しないので閉じる

            if( (fp = fdopen(pipes[0], "r")) != NULL ) { // 読み込みパイプ
                while( fgets(buf, sizeof(buf), fp) != NULL ) {
                    {
                        std::lock_guard<std::mutex> lock(mtx); // mutexをロック
                        outputLines.push_back(buf);
                        outputModified = true;
                    }
                }
                fclose(fp);
            }
            close(pipes[0]);
        }
        return outputLines;
    }

    bool get_output(std::vector<std::string> &output)
    {
        std::lock_guard<std::mutex> lock(mtx); // mutexをロック
        bool ret = outputModified;
        output = outputLines;
        return ret;
    }

    int calltest(void)
    {
        try {
            std::vector<std::string> output = executeCommand((char*)"ls -l");
            
            std::cout << "外部コマンドの標準出力:\n";
            for (const std::string& line : output) {
                std::cout << line;
            }
            
        } catch (const std::exception& e) {
            std::cerr << "エラー: " << e.what() << std::endl;
            return 1;
        }
        return 0;
    }
};


#endif // __CC_PIPEEXEC_H__

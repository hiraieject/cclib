/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_pipeexec_mingw64.h
 * @brief pipe exec class for C++/mingw64
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_PIPEEXEC_MINGW64_H__
#define __CC_PIPEEXEC_MINGW64_H__

#include <iostream>
#include <string>
#include <thread>
#include <future>
#include <array>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <system_error>
#include <atomic>

class cc_pipeexec {
public:
    cc_pipeexec()
        : pipe_(nullptr), is_running_(false) {}

    ~cc_pipeexec() {
        if (is_running_) {
            stop();
        }
    }

    bool executeCommand(const std::string& command) {
        if (is_running_) {
            std::cerr << "Process is already running." << std::endl;
            return false;
        }

        pipe_ = popen(command.c_str(), "r");
        if (!pipe_) {
            throw std::runtime_error("popen() failed!");
        }

        is_running_ = true;
        output_future_ = std::async(std::launch::async, &ProcessManager::readOutput, this);
        return true;
    }

    void stop() {
        if (pipe_) {
            pclose(pipe_);
            pipe_ = nullptr;
        }
        is_running_ = false;
    }

    bool isRunning() const {
        return is_running_;
    }

    std::string getOutput() {
        if (output_future_.valid()) {
            return output_future_.get();
        }
        return "";
    }

private:
    FILE* pipe_;
    std::atomic<bool> is_running_;
    std::future<std::string> output_future_;

    std::string readOutput() {
        std::array<char, 128> buffer;
        std::string result;
        while (is_running_ && fgets(buffer.data(), buffer.size(), pipe_) != nullptr) {
            result += buffer.data();
            std::cout << buffer.data(); // リアルタイム出力
        }
        return result;
    }
};

// ============================================================================================== 以下サンプル実装県UNITテスト

#ifdef UNIT_TEST_SAMPLE
int main()
{
    cc_piptexec piptexec;
    
    try {

        const std::string command1 = "python3 -c \"import time; [print(f'com1{i}') or time.sleep(1) for i in range(10)]\"";

        if (piptexec.start(command1)) {
            // プロセスが終了するのを待つ
            while (piptexec.isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            std::string output = piptexec.getOutput();
            std::cout << "\nCommand Output:\n" << output << std::endl;
        }

        // 再利用例
        std::string command2 = "python -c \"print('Another command')\"";
        if (piptexec.start(command2)) {
            // プロセスが終了するのを待つ
            while (piptexec.isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }

            std::string output = piptexec.getOutput();
            std::cout << "\nCommand Output:\n" << output << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
#endif

#endif // __CC_PIPEEXEC_MINGW64_H__

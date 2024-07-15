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
#include <vector>
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
        : pipe_(nullptr), is_running_(false), output_modified(false) {}

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
	outputLines.clear();
        pipe_ = popen(command.c_str(), "r");
        if (!pipe_) {
            throw std::runtime_error("popen() failed!");
        }

        is_running_ = true;
        output_future_ = std::async(std::launch::async, &cc_pipeexec::readOutput, this);
        return true;
    }

    void stop() {
        if (is_running_) {
            if (pipe_) {
                pclose(pipe_);
                pipe_ = nullptr;
            }
            is_running_ = false;
        }
    }

    bool isRunning() const {
        return is_running_;
    }

    bool wait(int timeout_msec = -1) {
        std::unique_lock<std::mutex> lock(mtx);
        if (timeout_msec <= 0) {
            cv.wait(lock, [this] { return output_modified == true; });
        } else {
            cv.wait_for(lock, std::chrono::milliseconds(timeout_msec), [this] { return output_modified == true; });
	}
	return output_modified == true;
    }
    std::string get_outputall() {
	output_modified = false;
        if (output_future_.valid()) {
            return output_future_.get();
        }
        return "";
    }
    std::vector<std::string> get_output() {
        std::lock_guard<std::mutex> lock(mtx);
	output_modified = false;
	return outputLines;
    }
    std::string get_appended() {
        std::lock_guard<std::mutex> lock(mtx);
	output_modified = false;
	return outputLines.back();
    }

    std::vector<std::string> simpleExecuteCommand(std::string &cmd)
    {
	if (executeCommand(cmd)) {
	    
            // プロセスが終了するのを待つ
            while (isRunning()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
	else {
	    printf (">> ERROR!!!: simpleExecuteCommand (%s)\n", cmd.c_str());
	    std::cout.flush();
	}
        return outputLines;
    }
    
private:
    FILE* pipe_;
    std::atomic<bool> is_running_;
    std::future<std::string> output_future_;

    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::string> outputLines;
    bool output_modified;

    std::string readOutput() {
        std::array<char, 128> buffer;
        std::string result;
        while (is_running_ && fgets(buffer.data(), buffer.size(), pipe_) != nullptr) {
	    std::lock_guard<std::mutex> lock(mtx);
            result += buffer.data();
            //std::cout << buffer.data() << std::flush; // リアルタイム出力
	    outputLines.push_back(buffer.data());
	    output_modified = true;
        }
        is_running_ = false; // ループ終了後に設定
        return result;
    }
};

// ============================================================================================== 以下サンプル実装県UNITテスト

#ifdef UNIT_TEST_SAMPLE
int main()
{
    printf ("----------------------- start unit test\n");

    cc_pipeexec pipeexec;
    
    try {

	printf ("----------------------- \n");
    
	//std::string command1 = "python3 -c \"import time; [print(f'com1{i}') or time.sleep(1) for i in range(10)]\"";
	std::string command1 = "python3 testcommand1.py";
	
	printf (">> pipeexec.executeCommand (%s)\n", command1.c_str());
	std::cout.flush();
        if (pipeexec.executeCommand(command1)) {

            // リアルタイムで出力を取り込む
            while (pipeexec.isRunning()) {
		if (pipeexec.wait(100/*msec*/)) {
		    std::cout << pipeexec.get_appended() << std::flush; // リアルタイム出力
		    //std::cout << pipeexec.getOutput() << std::flush; // リアルタイム出力
		}
            }
        }
	else {
	    printf (">> ERROR!!!: pipeexec.executeCommand (%s)\n", command1.c_str());
	    std::cout.flush();
	}
	
	printf ("----------------------- \n");
        //std::string command2 = "python -c \"print('Another command')\"";
	std::string command2 = "python3 testcommand1.py";

	printf (">> pipeexec.simpleExecuteCommand(%s)\n", command2.c_str());
	std::cout.flush();
	std::vector<std::string> result = pipeexec.simpleExecuteCommand(command2);
	printf ("command2 finished\n");

	for (const auto& str : result) {
	    std::cout << str;
	}
	
	
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
#endif

#endif // __CC_PIPEEXEC_MINGW64_H__

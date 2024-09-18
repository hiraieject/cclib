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

#include <winsock2.h>
#include <windows.h>

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
#include <string>

// ---------------------------------------------------------------------------------
class cc_pipeexec {
public:
    // ------------------- constructor/destructor/内部関数
    cc_pipeexec();
    ~cc_pipeexec();

    // ------------------- ユーザー向け public methods
    /// コマンド実行
    bool executeCommand(const std::string& command);
    void stop();
    bool isRunning();
    bool wait(int timeout_msec = -1);
    std::string get_outputall();
    std::vector<std::string> get_output();
    std::string get_appended();
    std::vector<std::string> simpleExecuteCommand(std::string &cmd);
    std::string convert_cp932_to_utf8(const std::string &cp932_str);
    
private:
    FILE* pipe_;
    std::atomic<bool> is_running_;
    std::atomic<bool> is_finish_;
    std::future<std::string> output_future_;
    std::mutex mtx;
    std::condition_variable cv;
    std::vector<std::string> outputLines;
    bool output_modified;
    
    std::string readOutput();
};

// ---------------------------------------------------------------------------------
inline 
cc_pipeexec::cc_pipeexec() : pipe_(nullptr), is_running_(false), is_finish_(false), output_modified(false)
{}

inline 
cc_pipeexec::~cc_pipeexec()
{
    if (is_running_) stop();
}

inline bool
cc_pipeexec::executeCommand(const std::string& command)
{
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

inline void
cc_pipeexec::stop()
{
    if (is_running_) {
        if (pipe_) {
            pclose(pipe_);
            pipe_ = nullptr;
        }
        is_running_ = false;
        is_finish_ = false;
    }
}

inline bool
cc_pipeexec::isRunning()
{
    return is_running_;
}

inline bool
cc_pipeexec::wait(int timeout_msec)
{
    std::unique_lock<std::mutex> lock(mtx);
    if (timeout_msec <= 0) {
        cv.wait(lock, [this] { return output_modified == true; });
    } else {
        cv.wait_for(lock, std::chrono::milliseconds(timeout_msec), [this] { return output_modified == true; });
    }
    return output_modified == true;
}

// 全ての出力を取得する
inline std::string
cc_pipeexec::get_outputall()
{
    output_modified = false;
    if (output_future_.valid()) {
        return output_future_.get();
    }
    return "";
}

// 現在の出力行を取得する
inline std::vector<std::string>
cc_pipeexec::get_output()
{
    std::lock_guard<std::mutex> lock(mtx);
    output_modified = false;
    return outputLines;
}

// 追加された最新の出力行を取得する
inline std::string
cc_pipeexec::get_appended()
{
    std::lock_guard<std::mutex> lock(mtx);
    output_modified = false;
    return outputLines.back();
}

inline std::vector<std::string>
cc_pipeexec::simpleExecuteCommand(std::string &cmd)
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

// CP932からUTF-8に変換する関数
inline std::string
cc_pipeexec::convert_cp932_to_utf8(const std::string &cp932_str) {
    // CP932からワイド文字列（UTF-16）へ変換
    int wide_size = MultiByteToWideChar(CP_ACP, 0, cp932_str.c_str(), -1, nullptr, 0);
    std::wstring wide_str(wide_size, 0);
    MultiByteToWideChar(CP_ACP, 0, cp932_str.c_str(), -1, &wide_str[0], wide_size);

    // ワイド文字列（UTF-16）からUTF-8へ変換
    int utf8_size = WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, nullptr, 0, nullptr, nullptr);
    std::string utf8_str(utf8_size, 0);
    WideCharToMultiByte(CP_UTF8, 0, wide_str.c_str(), -1, &utf8_str[0], utf8_size, nullptr, nullptr);

    return utf8_str;
}

inline std::string
cc_pipeexec::readOutput()
{
    std::array<char, 128> buffer;
    std::string result;
    while (is_running_ && fgets(buffer.data(), buffer.size(), pipe_) != nullptr) {
        std::lock_guard<std::mutex> lock(mtx);
        result += buffer.data();
        //std::cout << buffer.data() << std::flush; // リアルタイム出力
        //outputLines.push_back(convert_cp932_to_utf8(buffer.data()));
        outputLines.push_back(buffer.data());
        output_modified = true;
    }
    is_running_ = false; // ループ終了後に設定
    return result;
}

// ============================================================================================== 以下サンプル実装県UNITテスト

#ifdef UNIT_TEST_SAMPLE
int main()
{
    printf ("----------------------- start unit test\n");

    cc_pipeexec pipeexec;
    
    try {

	printf ("----------------------- PIPE EXEC \n");
	//std::string command1 = "python3 -c \"import time; [print(f'com1{i}') or time.sleep(1) for i in range(10)]\"";
	//std::string command1 = "python3 testcommand1.py";
	std::string command1 = "c:\\users\\hirai\\appdata\\roaming\\pstest\\scripts\\python.exe c:\\users\\hirai\\pstest_mb\\test\\scenario\\zz_developers_sample_scenario.py --folder 20240914_234524";
            
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
	
	printf ("----------------------- SIMPLE EXEC \n");
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



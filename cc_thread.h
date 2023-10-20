/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_thread.h
 * @brief thread class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_THREAD_H__
#define __CC_THREAD_H__

#include <cstdio>
#include <iostream>
#include <thread>
#include <mutex>

#include "cc_debugprint.h"
#include "cc_message.h"

/**
 * @class cc_thread
 * @brief C++用 汎用スレッドクラス
 * 
 * More detailed description of the class.
 */
class cc_thread {
protected:
    std::thread thread_obj;                     ///< class thread のインスタンス
    bool thread_loop;                           ///< スレッドループ有効フラグ、falseにするとスレッドループを抜ける
    bool thread_enable;                         ///< スレッド起動中フラグ
    std::mutex mtx;                             ///< cc_threadが保持する変数保護用

    bool loop_continue(void);                   // ループ継続判定
    virtual void thread_main (void) = 0;        // threadのメイン関数、継承先のクラスで必ず定義しなければならない
    
public:
    // public functions
    cc_thread (key_t message_key, std::string nickname);           // コンストラクター
    ~cc_thread ();                              // デストラクター

    void set_loop_continue(bool enb);           // ループ継続判定設定

    // スレッド制御系
    void thread_up (void);                      // スレッド起動
    void thread_down (void);                    // スレッド停止
    void thread_detach (void);                  // スレッドdetache

    // public variables
    std::string nickname;                       ///< ニックネーム
    cc_debugprint thread_dbg;                   ///< cclib debugprint
    cc_message message;                         ///< cclib message
};

#endif// __CC_THREAD_H__


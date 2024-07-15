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

/// デバックプリント エラー表示用マクロ、enableの是非に関わらず表示
#define CC_THREAD_ERRPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### ERROR!: " fmt,thread_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント ワーニング表示用マクロ、enableの是非に関わらず表示
#define CC_THREAD_WARNPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### WARNING!: " fmt,thread_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント デバック表示用マクロ、enableのときだけ表示
#define CC_THREAD_DBGPR(fmt, args...) \
    if (thread_dbg.enable_flg) { printf("[%s:%s():%d] " fmt,thread_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }

/// デバックするときは有効にする、リリース時は無効化すること
#define CC_THREAD_DEBUGPRINT

// ===========================================================================================================
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

    /**
     * @brief thread_main のループ継続判定
     * @return true: 継続, false: ループ終了
     */
    bool loop_continue() 
    {
        std::lock_guard<std::mutex> lock(mtx); // mutexをロック
        return thread_loop == true;
    }
    virtual void thread_main () = 0;        // threadのメイン関数、継承先のクラスで必ず定義しなければならない
    
public:
    // public variables
    std::string nickname;                       ///< ニックネーム
    cc_debugprint thread_dbg;                   ///< cclib debugprint
    cc_message message;                         ///< cclib message

    // ------------------- constructor/destructor/内部関数
    /**
     * @brief コンストラクター
     * @param arg_nickname     ニックネーム文字列(デバックプリントで使用)
     */
    cc_thread (std::string arg_nickname) :
        thread_obj(), thread_loop(false), thread_enable(false),
        nickname(arg_nickname), thread_dbg(arg_nickname), message(arg_nickname)
    {
        CC_THREAD_DBGPR ("instance created\n");
        
        // debugprint をまとめてオンにする
#ifdef CC_THREAD_DEBUGPRINT
        thread_dbg.enable();
#endif
    }
    
    /// デストラクター
    ~cc_thread ()
    {
        if (thread_enable) {
            thread_down ();
        }
        CC_THREAD_DBGPR ("instance deleted\n");
    }

    // ------------------- ユーザー向け public methods

    /**
     * @brief thread_main のループ継続判定の設定関数
     * @param enb true: 継続, false: ループ終了
     */
    void set_loop_continue(bool enb)
    {
        std::lock_guard<std::mutex> lock(mtx); // mutexをロック
        thread_loop = enb;
    }

    /// スレッド起動
    void thread_up ()
    {
        if (thread_enable == false) {
            // wakeup thread
            CC_THREAD_DBGPR ("now wakeup thread\n");
            {
                std::lock_guard<std::mutex> lock(mtx); // mutexをロック
                thread_loop     = true;
                thread_enable   = true;
            }
            thread_obj = std::thread([this]() {this->thread_main();}); // ラムダ式経由でスレッド関数を呼ぶ
        }
    }

    /// スレッド停止
    void thread_down()
    {
        {
            std::lock_guard<std::mutex> lock(mtx); // mutexをロック
            if (thread_enable == true) {
                CC_THREAD_DBGPR ("change thread loop to disable\n");
                thread_enable = false;
                thread_loop   = false;
            }
        }
        if (thread_obj.joinable()) {
            CC_THREAD_DBGPR ("start thread join\n");
            thread_obj.join ();
            CC_THREAD_DBGPR ("success thread join\n");
        }
    }
    
    /**
     * @brief デタッチ処理
     * この関数を呼ぶと、スレッド停止時にJOIN待ちしなくなる
     */
    void thread_detach ()
    {
        thread_obj.detach();
        CC_THREAD_DBGPR ("thread detached\n");
    }
};

#endif// __CC_THREAD_H__


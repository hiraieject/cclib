/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_thread.cc
 * @brief thread class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#include "cc_thread.h"

/// デバックプリント エラー表示用マクロ、enableの是非に関わらず表示
#define CC_THREAD_ERRPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### ERROR!: " fmt,thread_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント ワーニング表示用マクロ、enableの是非に関わらず表示
#define CC_THREAD_WARNPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### WARNING!: " fmt,thread_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント デバック表示用マクロ、enableのときだけ表示
#define CC_THREAD_DBGPR(fmt, args...) \
    if (thread_dbg.enable_flg) { printf("[%s:%s():%d] " fmt,thread_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }

// -------------------------------------------------------------------------------------------

/**
 * @brief コンストラクター
 * @param nickname     ニックネーム文字列(デバックプリントで使用)
 */
//cc_thread::cc_thread (key_t message_key, std::string nickname) :
cc_thread::cc_thread (int message_key, std::string nickname) :
    thread_dbg (nickname)
    //,
    //message(message_key,nickname,true/*master_flag*/)
{
    CC_THREAD_DBGPR ("instance created\n");

    // initialize
    {
        std::lock_guard<std::mutex> lock(mtx); // mutexをロック
        thread_loop     = false;
        thread_enable   = false;
        this->nickname  = nickname;
    }

    // debugprint をまとめてオンにする
    //thread_dbg.enable();
}
/**
 * @brief デストラクター
 */
cc_thread::~cc_thread ()
{
    if (thread_enable) {
        thread_down ();
    }
    CC_THREAD_DBGPR ("instance deleted\n");
}

// -------------------------------------------------------------------------------------------
/**
 * @brief thread_main のループ継続判定
 * @return true: 継続, false: ループ終了
 */
bool
cc_thread::loop_continue(void)
{
    std::lock_guard<std::mutex> lock(mtx); // mutexをロック
    return thread_loop == true;
}
/**
 * @brief thread_main のループ継続判定の設定関数
 * @param enb true: 継続, false: ループ終了
 */
void
cc_thread::set_loop_continue(bool enb)
{
    std::lock_guard<std::mutex> lock(mtx); // mutexをロック
    thread_loop = enb;
}

/**
 * @brief スレッド起動
 */
void
cc_thread::thread_up (void)
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

/**
 * @brief スレッド停止
 */
void
cc_thread::thread_down(void)
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
void
cc_thread::thread_detach (void)
{
    thread_obj.detach();
    CC_THREAD_DBGPR ("thread detached\n");
}



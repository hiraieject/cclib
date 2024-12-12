/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_debugprint.h
 * @brief デバックプリント制御
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_DEBUGPRINT_H__
#define __CC_DEBUGPRINT_H__

#include <iostream>
#include <string>

/**
 * @class cc_debugprint
 * @brief C++用 デバックプリント制御クラス
 * 
 * More detailed description of the class.
 */
class cc_debugprint {
private:
public:
    // public functions
    cc_debugprint(std::string nickname) {       ///< コンストラクタ、ニックネームを設定、デバックプリントはデフォルトオフ
        this->nickname   = nickname;
        this->enable_flg = false;
    }
    cc_debugprint(void) {                       ///< 引数なしのときの保険のコンストラクタ
        this->nickname   = "";
        this->enable_flg = false;
    }
    void enable(void) {                         ///< デバックプリントを有効にする
        this->enable_flg = true;
    }
    void disable(void) {                        ///< デバックプリントを無効にする
        this->enable_flg = false;
    }
    char *ts(void) {                            ///< "[HH:MM:SS]" 時刻文字列を返す
        // 現在時刻を取得
        std::time_t t = std::time(nullptr);
        std::tm* now = std::localtime(&t);
        // 静的バッファに時刻を保存
        static char time_str[20];  // "[HH:MM:SS]" に必要なバッファ長は 10
        std::snprintf(time_str, sizeof(time_str), "[%02d:%02d:%02d]",
                      now->tm_hour, now->tm_min, now->tm_sec);
        return time_str;
    }

    // public variables
    bool enable_flg;                            ///< デバックプリントの有効/無効フラグ
    std::string nickname;                       ///< デバックプリントで表示するニックネーム
};
#define ERRPR(fmt, args...) \
    { printf("%s [%s:%s():%d] ##### ERROR!: " fmt,dbg.ts(),dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); fflush(stdout); }
#define WARNPR(fmt, args...) \
    { printf("%s [%s:%s():%d] ##### WARNING!: " fmt,dbg.ts(),dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); fflush(stdout); }
#define INFOPR(fmt, args...) \
    { printf("%s [%s:%s():%d] " fmt,dbg.ts(),dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); fflush(stdout); }
#define DBGPR(fmt, args...) \
    if (dbg.enable_flg) { printf("%s [%s:%s():%d] " fmt,dbg.ts(),dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); fflush(stdout); }

#endif // __CC_DEBUGPRINT_H__

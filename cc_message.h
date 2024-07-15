/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

#ifndef __CC_MESSAGE_H__
#define __CC_MESSAGE_H__

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>

#include "cc_debugprint.h"

/**
 * @file cc_message.h
 * @brief スレッド間メッセージ通信
 * 
 * Copyright (c) 2024 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 */

class cc_message_manager;
extern cc_message_manager message_mgr;

/// デバックするときはtrueにする、リリース時はfalse化すること
#define CC_MESSAGE_DEBUGPRINT true
//#define CC_MESSAGE_DEBUGPRINT false


/// デバックプリント エラー表示用マクロ、enableの是非に関わらず表示
#define CC_MESSAGE_ERRPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### ERROR!: " fmt,message_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント ワーニング表示用マクロ、enableの是非に関わらず表示
#define CC_MESSAGE_WARNPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### WARNING!: " fmt,message_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント デバック表示用マクロ、enableのときだけ表示
#define CC_MESSAGE_DBGPR(fmt, args...) \
    if (message_dbg.enable_flg || CC_MESSAGE_DEBUGPRINT) { printf("[%s:%s():%d] " fmt,message_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }

// ===========================================================================================================
// 
/**
 * @class cc_message_packet
 * @brief キューで送るメッセージパケットクラス
 */
class cc_message_packet {
private:
    std::string sender;
    nlohmann::json json_obj;
    std::string json_str;
    bool json_obj_valid;
    bool json_str_valid;
    
public:

    // ------------------- constructor/destructor
    cc_message_packet(std::string arg_sender) : sender(arg_sender), json_obj_valid(false), json_str_valid(false) {}

    // ------------------- public methods
    void set_json_obj(const nlohmann::json &arg_json_obj) {
        json_obj       = arg_json_obj;
        json_obj_valid = true;
    }
    void set_json_str(const std::string &arg_json_str) {
        json_str       = arg_json_str;
        json_str_valid = true;
    }
    nlohmann::json *ref_json_obj() {
        if (!json_obj_valid && json_str_valid) {
            try {
                json_obj = nlohmann::json::parse(json_str);
                json_obj_valid = true;
            } catch (const nlohmann::json::parse_error &e) {
                return nullptr; // 変換に失敗した場合は nullptr を返す
            }
        }
        return json_obj_valid ? &json_obj : nullptr;
    }
    std::string *ref_json_str() {
        if (!json_str_valid && json_obj_valid) {
            json_str = json_obj.dump();
            json_str_valid = true;
        }
        return json_str_valid ? &json_str : nullptr;
    }
    std::string *ref_sender() {
        return &sender;
    }
};

// ===========================================================================================================
/**
 * @class cc_message_resource
 * @brief キューリソースクラス(内部処理用)
 *
 * qidと1対1で生成される
 * <br> 複数の cc_message_resource のインスタンスから参照される
 */
class cc_message_resource {
private:
    bool enable_flg;
    std::queue<cc_message_packet> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    // ------------------- constructor/destructor
    // コンストラクタ
    cc_message_resource() : enable_flg(true) {}

    // ------------------- public methods
    /// キューの無効化
    void disable() {
        std::lock_guard<std::mutex> lock(mtx);
        while(!queue.empty()) { // queを空にする
            queue.pop();
        }
        enable_flg = false;
    }
    /// キューの有効化
    void enable() {
        std::lock_guard<std::mutex> lock(mtx);
        enable_flg = true;
    }
    /// QUEにメッセージをPUSH
    void push(cc_message_packet &packet) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!enable_flg)
            return;
        queue.push(packet);
    }
    /// メッセージへの参照を取得
    cc_message_packet *ref_front() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!enable_flg || queue.empty())
            return nullptr;
        cc_message_packet *ptr = &(queue.front());
        return ptr;
    }
    /// QUEのメッセージをPOP
    void pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!enable_flg || queue.empty())
            return;
        queue.pop();
    }

    /// 有効状態の取得
    bool is_enable() {    // const: メソッドはメンバ変数を変更しない
        std::lock_guard<std::mutex> lock(mtx);
        return enable_flg;
    }
    /// 空かどうかの判定
    bool is_empty() {    // const: メソッドはメンバ変数を変更しない
        std::lock_guard<std::mutex> lock(mtx);
        if (!enable_flg)
            return true;        // 無効時は空を返す
        return queue.empty();
    }

    /**
     * @brief メッセージが格納されるまで待機
     * @param[in] timeout タイムアウト時間（ミリ秒）、デフォルトは無限待機
     * @return 処理する受信データがあったらtrueを返す
     */
    bool wait(int timeout_msec = -1) {
        std::unique_lock<std::mutex> lock(mtx);
        if (timeout_msec <= 0) {
            cv.wait(lock, [this] { return !queue.empty(); });
            return !queue.empty();
        } else {
            if (cv.wait_for(lock, std::chrono::milliseconds(timeout_msec), [this] { return !queue.empty(); })) {
                return true;  // なんか受信した
            }
            return false;  // 受信タイムアウト時の処理
        }
    }
};

// ===========================================================================================================
/**
 * @class cc_message
 * @brief ユーザー向けのキュークラス
 *
 * インスタンス内にはキューの実体は無く、 cc_message_resource  インスタンスへの参照となっている
 */
class cc_message {
private:
    int                  que_resource_qid;
    cc_message_resource *que_resource_ptr;
    std::string          nickname;
    cc_debugprint        message_dbg;
    
public:
    
    // ------------------- constructor/destructor/内部関数
    cc_message(std::string arg_nickname, int qid=-1);
    ~cc_message();

    /// 内部関数：QUEリソースをセット
    void set_resource(cc_message_resource *arg_que_resource, int arg_que_resource_qid) {
        que_resource_ptr = arg_que_resource;
        que_resource_qid = arg_que_resource_qid;
    }

    // ------------------- ユーザー向け public methods
    /// QUEにメッセージをPUSH
    void push(cc_message_packet &packet) {
        if (!que_resource_ptr)
            return;
        que_resource_ptr->push(packet);
    }
    /// メッセージへの参照を取得
    cc_message_packet *ref_front() {
        if (!que_resource_ptr)
            return nullptr;
        return que_resource_ptr->ref_front();
    }
    /// QUEにメッセージをPOP
    void pop() {
        if (!que_resource_ptr)
            return;
        que_resource_ptr->pop();
    }

    /// JSON(OBJ)送信
    void send_json (std::string sender, nlohmann::json &json_obj) {
        cc_message_packet packet(sender);
        packet.set_json_obj(json_obj);
        push(packet);
    }
    /// JSON(STR)送信
    void send_json (std::string sender, std::string &json_str) {
        cc_message_packet packet(sender);
        packet.set_json_str(json_str);
        push(packet);
    }

    /// 有効状態の取得
    bool is_enable() const {    // const: メソッドはメンバ変数を変更しない
        if (!que_resource_ptr)
            return false;
        return que_resource_ptr->is_enable();
    }
    /// 空かどうかの判定
    bool is_empty() const {    // const: メソッドはメンバ変数を変更しない
        if (!que_resource_ptr)
            return true;        // 無効時は空を返す
        return que_resource_ptr->is_empty();
    }
    
    /**
     * @brief メッセージが格納されるまで待機
     * @param[in] timeout タイムアウト時間（ミリ秒）、省略時は無限待機
     * @return 処理する受信データがあったらtrueを返す
     */
    bool wait(int timeout_msec=-1) {
        if (!que_resource_ptr)
            return false;
        return que_resource_ptr->wait(timeout_msec);
    }

    /// QID取得
    int get_qid() const {    // const: メソッドはメンバ変数を変更しない
        return que_resource_qid;
    }
};

// ===========================================================================================================
class cc_message_manager {
private:
    std::vector<cc_message_resource *>  message_resources_tbl;
    std::mutex                          message_resources_mtx;

public:
    // コンストラクタ
    cc_message_manager() = default;
    // デストラクタ
    ~cc_message_manager() = default;

    // public メソッド
    bool get_message(cc_message &message, int qid = -1) {
        std::lock_guard<std::mutex> lock(message_resources_mtx);
        if (qid == -1) {
            // 新規生成要求
            for (size_t i = 0; i < message_resources_tbl.size(); ++i) {
                if (!message_resources_tbl[i]->is_enable()) {
                    // 未使用MESSAGE発見、それを有効化して返す
                    message_resources_tbl[i]->enable();
                    message.set_resource(message_resources_tbl[i], static_cast<int>(i));
                    return true;
                }
            }
            // 再利用可能なものがなければ新規割当
            auto new_resourcep = new cc_message_resource;
            message.set_resource(new_resourcep, static_cast<int>(message_resources_tbl.size()));
            message_resources_tbl.push_back(new_resourcep);
            return true;
        } else {
            // 既存MESSAGE取得要求
            if (qid >= static_cast<int>(message_resources_tbl.size()) || qid < 0) {
                message.set_resource(nullptr, -1);
                return false;
            }
            message.set_resource(message_resources_tbl[qid], qid);
            return true;
        }
    }

    void destroy_message(cc_message &message) {
        std::lock_guard<std::mutex> lock(message_resources_mtx);
        int qid = message.get_qid();
        if (qid >= 0 && qid < static_cast<int>(message_resources_tbl.size())) {
            message_resources_tbl[qid]->disable(); // 開放しないで無効化、必要に応じて再利用する
        }
    }
};

// ===========================================================================================================
inline cc_message::cc_message(std::string arg_nickname, int qid) :
    que_resource_qid(-1), que_resource_ptr(nullptr), nickname(arg_nickname), message_dbg("DBG:CC_MESSAGE("+arg_nickname+")")
{
#ifdef CC_MESSAGE_DEBUGPRINT
    message_dbg.enable();
#endif // CC_MESSAGE_DEBUGPRINT

    message_mgr.get_message(*this);    // MESSAGEを確保
    
    CC_MESSAGE_DBGPR("CC_MESSAGE(qid=%d) instance created\n", que_resource_qid);
}

inline cc_message::~cc_message() {
    CC_MESSAGE_DBGPR("CC_MESSAGE(qid=%d) instance deleted\n", que_resource_qid);
}

#endif // __CC_NMSGQ_H__

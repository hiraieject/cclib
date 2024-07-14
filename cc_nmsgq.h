#ifndef __CC_NMSGQ_H__
#define __CC_NMSGQ_H__

#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <vector>
#include <memory>
#include <nlohmann/json.hpp>


// キューで送るメッセージ
class cc_nMsgPacket {
public:
    nlohmann::json json_obj;
    std::string json_str;
    bool json_obj_valid;
    bool json_str_valid;
    int reply_qid;

    // ------------------- constructor/destructor
    cc_nMsgPacket() : json_obj_valid(false), json_str_valid(false), reply_qid(-1) {}

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
        if (json_obj_valid == false) {
            return nullptr;
        }
        return &json_obj;
    }
    std::string *ref_json_str() {
        if (json_str_valid == false) {
            return nullptr;
        }
        return &json_str;
    }
};

/**
 * @class cc_nMsgQueResource
 * @brief キューリソースクラス(内部処理用)
 *
 * qidと1対1で生成される
 * <br> 複数の cc_nMsgQueMgr のインスタンスから参照される
 */
class cc_nMsgQueResource {
private:
    bool enable_flg;
    std::queue<cc_nMsgPacket> queue;
    std::mutex mtx;
    std::condition_variable cv;

public:
    // ------------------- constructor/destructor
    // コンストラクタ
    cc_nMsgQueResource() : enable_flg(true) {}

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
    void push(cc_nMsgPacket &packet) {
        std::lock_guard<std::mutex> lock(mtx);
        if (!enable_flg)
            return;
        queue.push(packet);
    }
    /// メッセージへの参照を取得
    cc_nMsgPacket *ref_front() {
        std::lock_guard<std::mutex> lock(mtx);
        if (!enable_flg || queue.empty())
            return nullptr;
        cc_nMsgPacket *ptr = &(queue.front());
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

/**
 * @class cc_nMsgQue
 * @brief ユーザー向けのキュークラス
 *
 * インスタンス内にはキューの実体は無く、cc_nMsgQueResource インスタンスへの参照となっている
 */
class cc_nMsgQue {
public:
    int                 que_resource_qid;
    cc_nMsgQueResource *que_resource_ptr;
    
    // ------------------- constructor/destructor/内部関数
    cc_nMsgQue() : que_resource_qid(-1), que_resource_ptr(nullptr) {}

    /// 内部関数：QUEリソースをセット
    void set_resource(cc_nMsgQueResource *arg_que_resource, int arg_que_resource_qid) {
        que_resource_ptr = arg_que_resource;
        que_resource_qid = arg_que_resource_qid;
    }

    // ------------------- ユーザー向け public methods
    /// QUEにメッセージをPUSH
    void push(cc_nMsgPacket &packet) {
        if (!que_resource_ptr)
            return;
        que_resource_ptr->push(packet);
    }
    /// メッセージへの参照を取得
    cc_nMsgPacket *ref_front() {
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

    // ----------------------------------------------- API for receiver
    //std::string receiver_recv_json_str ();

    // ----------------------------------------------- API for sender
    //std::string send_json (nlohmann::json &send_json_obj);
    //std::string send_json (bool reply_required, std::string sender, std::string send_json_str);
    //std::string send_json (bool reply_required, std::string sender, nlohmann::json &send_json_obj);
};

class cc_nMsgQueMgr {
private:
    std::vector<cc_nMsgQueResource *> que_resources_tbl;
    std::mutex que_resources_mtx;

public:
    // コンストラクタ
    cc_nMsgQueMgr() = default;
    // デストラクタ
    ~cc_nMsgQueMgr() = default;

    // public メソッド
    bool get_que(cc_nMsgQue &que, int qid = -1) {
        std::lock_guard<std::mutex> lock(que_resources_mtx);
        if (qid == -1) {
            // 新規生成要求
            for (size_t i = 0; i < que_resources_tbl.size(); ++i) {
                if (!que_resources_tbl[i]->is_enable()) {
                    // 未使用QUE発見、それを有効化して返す
                    que_resources_tbl[i]->enable();
                    que.set_resource(que_resources_tbl[i], static_cast<int>(i));
                    return true;
                }
            }
            // 再利用可能なものがなければ新規割当
            auto new_resourcep = new cc_nMsgQueResource;
            que.set_resource(new_resourcep, static_cast<int>(que_resources_tbl.size()));
            que_resources_tbl.push_back(new_resourcep);
            return true;
        } else {
            // 既存QUE取得要求
            if (qid >= static_cast<int>(que_resources_tbl.size()) || qid < 0) {
                que.set_resource(nullptr, -1);
                return false;
            }
            que.set_resource(que_resources_tbl[qid], qid);
            return true;
        }
    }

    void destroy_que(cc_nMsgQue &que) {
        std::lock_guard<std::mutex> lock(que_resources_mtx);
        int qid = que.get_qid();
        if (qid >= 0 && qid < static_cast<int>(que_resources_tbl.size())) {
            que_resources_tbl[qid]->disable(); // 開放しないで無効化、必要に応じて再利用する
        }
    }
};


#endif // __CC_NMSGQ_H__


#include <unistd.h>  // sleep() を使用するために必要

#include "cc_thread.h"
#include "cc_nmsgq.h"

cc_nMsgQueMgr quemgr;

// ===========================================================
class Thread1 : public cc_thread {
private:
    cc_debugprint dbg;                      ///< cclib debugprint

    void thread_main (void);
    
public:
    cc_nMsgQue que;
    bool polling;

    Thread1() :
        cc_thread(0, "thread1"),
        dbg("thread1")
    {
        dbg.enable();
        thread_dbg.disable();
        quemgr.get_que(que);    // QUEを確保

        // スレッドを立ち上げる
        thread_up();
    }
    ~Thread1(void) {
        thread_down();
        quemgr.destroy_que(que);
    }
};

// ===========================================================
class Thread2 : public cc_thread {
private:
    cc_debugprint dbg;                      ///< cclib debugprint

    void thread_main (void);
    
public:
    cc_nMsgQue que;
    bool polling;
   
    Thread2() :
        cc_thread(0, "thread2"),
        dbg("thread2")
    {
        dbg.enable();
        thread_dbg.disable();
        quemgr.get_que(que);    // QUEを確保

        // スレッドを立ち上げる
        thread_up();
    }
    ~Thread2(void) {
        thread_down();
        quemgr.destroy_que(que);
    }
};

Thread1 th1;
Thread2 th2;

// ===========================================================
void Thread1::thread_main (void) {
    DBGPR ("thread up\n");
    sleep(1);

    auto lasttime = std::chrono::system_clock::now();   // 現在時刻を取得

    try {
        while (loop_continue()) {
        
            // QUE受信処理
            que.wait(100/*msec*/);
            while (!que.is_empty()) {
                // 受信データあり
                cc_nMsgPacket *packetp = que.ref_front();
                if (packetp) {
                    nlohmann::json *json_objp = packetp->ref_json_obj();
                    if (json_objp) {
                        std::string action = (*json_objp)["action"];

                        if (action == "result") {
                            std::string value = (*json_objp)["value"];
                            DBGPR ("recv: action=%s value=%s\n", action.c_str(), value.c_str());

                        } else {
                            DBGPR ("recv: unknown action=%s\n", action.c_str());
                        }
                    
                    } else {
                        DBGPR ("recv: empty str\n");
                    }
                }
                que.pop();;
            }

            // 定期処理 100msec or less cycle
            {
            
            }
            // 定期処理 1sec cycle
            auto now = std::chrono::system_clock::now();   // 現在時刻を取得
            auto elapsed_msec = std::chrono::duration_cast<std::chrono::milliseconds>(now - lasttime);    // 経過時間を計算（単位はミリ秒）
            if (elapsed_msec.count() >= 1000) {
                lasttime = now;
                // １秒以上の間隔でこのブロックの処理を実行
            
                // 定期コマンド送信
                nlohmann::json json_obj;
                json_obj["action"] = "command";
                cc_nMsgPacket packet;
                packet.set_json_obj(json_obj);
                th2.que.push(packet);
            }
        }

    } catch (const std::exception& e) {
        DBGPR ("Exception: %s\n", e.what());
    } catch (...) {
        DBGPR ("Unknown exception caught\n");
    }
}
// - - - - - - - - - - - - - - - - - - - - - 
void Thread2::thread_main (void) {
    DBGPR ("thread up\n");
    sleep(1);
    
    auto lasttime = std::chrono::system_clock::now();   // 現在時刻を取得

    try {
        while (loop_continue()) {
        
            // QUE受信処理
            que.wait(100/*msec*/);
            while (!que.is_empty()) {
                // 受信データあり
                cc_nMsgPacket *packetp = que.ref_front();
                if (packetp) {
                    nlohmann::json *json_objp = packetp->ref_json_obj();
                    if (json_objp) {
                        std::string action = (*json_objp)["action"];

                        if (action == "command") {
                            DBGPR ("recv: action=%s\n", action.c_str());

                            // 返信送信
                            nlohmann::json json_obj;
                            json_obj["action"] = "result";
                            json_obj["value"]  = "ok";
                            cc_nMsgPacket packet;
                            packet.set_json_obj(json_obj);
                            th1.que.push(packet);
                        
                        } else {
                            DBGPR ("recv: unknown action=%s\n", action.c_str());
                        }
                    
                    } else {
                        DBGPR ("recv: empty str\n");
                    }
                }
                que.pop();;
            }

            // 定期処理 100msec or less cycle
            {
            }

            // 定期処理 1sec cycle
            auto now = std::chrono::system_clock::now();   // 現在時刻を取得
            auto elapsed_msec = std::chrono::duration_cast<std::chrono::milliseconds>(now - lasttime);    // 経過時間を計算（単位はミリ秒）
            if (elapsed_msec.count() >= 1000) {
                lasttime = now;
                // １秒以上の間隔でこのブロックの処理を実行
            
            }
        }

    } catch (const std::exception& e) {
        DBGPR ("Exception: %s\n", e.what());
    } catch (...) {
        DBGPR ("Unknown exception caught\n");
    }
};

// ===========================================================
// テスト用のメイン関数
int main()
{
    th1.polling = true;
    sleep(10);
    th1.polling = false;

    sleep(1);
    //th2.thread_down();
    //th1.thread_down();
    
    return 0;
    
}

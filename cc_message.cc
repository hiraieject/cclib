/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_message.cc
 * @brief message send/recv class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#include <sys/msg.h>            // for IPC_CREAT etc
#include <fcntl.h>              // for O_RDWR etc
#include <unistd.h>             // for open/close etc
#include <sys/stat.h>           // for mkfifo etc

#include "cc_message.h"

/// デバックプリント エラー表示用マクロ、enableの是非に関わらず表示
#define CC_MESSAGE_ERRPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### ERROR!: " fmt,message_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント ワーニング表示用マクロ、enableの是非に関わらず表示
#define CC_MESSAGE_WARNPR(fmt, args...) \
    { printf("[%s:%s():%d] ##### WARNING!: " fmt,message_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }
/// デバックプリント デバック表示用マクロ、enableのときだけ表示
#define CC_MESSAGE_DBGPR(fmt, args...) \
    if (message_dbg.enable_flg) { printf("[%s:%s():%d] " fmt,message_dbg.nickname.c_str(),__FUNCTION__,__LINE__, ## args); }


cc_message_base::cc_message_base (std::string nickname)
{
    this->nickname = nickname;
}
cc_message_base::~cc_message_base ()
{}

cc_message::cc_message (key_t send_key, std::string nickname, bool master_flag) :
    cc_message_base (nickname)
{
    this->master_flag = master_flag;

    this->send_qid = msgget (send_key , 0666 | IPC_CREAT);
    if (this->send_qid == -1) {
        perror("msgget()");
        CC_MESSAGE_ERRPR ("reply rqid create error\n");
        goto FINISH;
    }
    send_fd = open_fifo (this->send_qid, master_flag ? O_RDWR : O_WRONLY);


 FINISH:
    return;
}

cc_message::~cc_message ()
{
    close_fifo (send_fd);
    if (master_flag) {
        destroy_fifo(send_qid);
    }
}

int
cc_message::reciever_get_fd (void)
{
    return send_fd;
}

std::string
cc_message::reciever_recv_json_str (void)
{
    std::string json_str;
    
    // メッセージ受信
    message_packet packet = {0};
    int ret = read (send_fd, (void*)&packet, sizeof(packet)); // 受信
    if (ret == -1) {
        perror("read()");
        CC_MESSAGE_ERRPR ("message read error\n");
        goto FINISH;
    }
    
#if defined(ENABLE_RECVLOG)
    CC_MESSAGE_DBGPR ("recved message [%s -> %s]\n",
                      packet.sender, packet.recver);
#endif
    json_str = packet.json_str;

 FINISH:
    return json_str;
}

// ----------------------------------------------- newAPI
std::string
cc_message::send_json (nlohmann::json &send_json_obj)
{
    std::string reply_json_str = "";
    int reply_qid = -1;
    int reply_fd  = -1;

    bool reply_required = send_json_obj.at("reply_required");
    
    if (reply_required) {
        reply_qid = msgget (IPC_PRIVATE , 0666 | IPC_CREAT);
        if (reply_qid == -1) {
            perror("msgget()");
            CC_MESSAGE_ERRPR ("reply qid create error\n");
            return "";
        }
        send_json_obj["reply_qid"] = reply_qid;
    }
    // メッセージパケットを生成・送信
    {
        message_packet packet = {0};
        packet.com = COM_COMMON_JSON;
        char *p;
        std::string sender = send_json_obj["sender"];
        p = stpncpy (packet.sender, sender.c_str(),
                     CC_MESSAGE_SENDERNAME_MAXLEN);
        *p = '\0'; // 終端
        std::string reciever = send_json_obj["reciever"];
        p = stpncpy (packet.reciever, reciever.c_str(),
                     CC_MESSAGE_RECIEVERNAME_MAXLEN);
        *p = '\0'; // 終端
        p = stpncpy (packet.json_str,
                     send_json_obj.dump().c_str(),
                     CC_MESSAGE_JSON_MAXLEN);
        *p = '\0'; // 終端

        // メッセージを送信
#if defined(ENABLE_SENDLOG)
        CC_MESSAGE_DBGPR ("now send message [%s -> %s]\n",
                          packet.sender, packet.reciever);
#endif
        int ret = write (send_fd, (void*)&packet, sizeof(packet)); // 送信
        if (ret == -1) {
            perror("write()");
            CC_MESSAGE_ERRPR ("message send error\n");
            goto FINISH;
        }
    }

    if (reply_qid == -1) {
        // 返信不要ならここで終了
        goto FINISH;
    }

    // 返信パケットを受信
    {
        // 返信FIFOをオープン
        reply_fd = open_fifo (reply_qid, O_RDONLY);

        // select() でイベント待ち
        struct timeval timeout = { 5/*sec*/, 0/*usec*/ };
        fd_set  rfds;
        FD_ZERO(&rfds);
        FD_SET(reply_fd, &rfds);
        CC_MESSAGE_DBGPR ("select wait start\n");
        select(FD_SETSIZE, &rfds, 0, 0, &timeout);
    
        // 受信して即FIFOクローズ、消去
        if (!FD_ISSET(reply_fd, &rfds)) {
            // FDSETされてない -> timeout
            CC_MESSAGE_ERRPR ("recv reply timed out\n");
            goto FINISH;
        }
        message_packet packet = {0};
        int ret = read (reply_fd, (void*)&packet, sizeof(packet)); // 受信
        if (ret == -1) {
            perror("read()");
            CC_MESSAGE_ERRPR ("reply recv error\n");
            goto FINISH;
        }
        reply_json_str = packet.json_str;
    }

 FINISH:
    if (reply_fd != -1) {
        close_fifo(reply_fd);
    }
    if (reply_qid != -1) {
        destroy_fifo(reply_qid);
    }
    return reply_json_str;
}
std::string
cc_message::send_json (bool reply_required, std::string sender, std::string send_json_str)
{
    nlohmann::json send_json_obj = nlohmann::json::parse(send_json_str);
    return send_json (reply_required, sender, send_json_obj);
}
std::string
cc_message::send_json (bool reply_required, std::string sender, nlohmann::json &send_json_obj)
{
    send_json_obj["sender"]   = sender;
    send_json_obj["reciever"] = nickname;
    send_json_obj["reply_required"] = reply_required;
    
    return send_json (send_json_obj);
}


int
cc_message::open_fifo (int qid, int option)
{
    int fd = -1;

    // FIFO ファイル名
    std::ostringstream oss;
    oss << "/tmp/fifo." << qid;
    char *c_fifoname = (char*)oss.str().c_str();
    
    // FIFO 作成
    if (access(c_fifoname,F_OK) != 0) {
        // fifoファイルがない場合、fifoを新規に作成する
        if ((mkfifo(c_fifoname, 0666) < 0) && (errno != EEXIST)) {
            CC_MESSAGE_ERRPR("fifo create error, filename=%s\n", c_fifoname);
            perror("mkfifo()");
            goto FINISH;
        }
    }
    
    // FIFO オープン
    if ((fd = open(c_fifoname, option | O_NONBLOCK, 0)) == -1) {
        CC_MESSAGE_ERRPR("fifo open error, filename=%s\n", c_fifoname);
        perror("open()");
        goto FINISH;
    }
    CC_MESSAGE_DBGPR ("FIFO %s opened\n", c_fifoname);
 FINISH:
    return fd;
}
void
cc_message::close_fifo (int fd)
{
    close (fd);
}
void
cc_message::destroy_fifo (int qid)
{
    // FIFO ファイル名
    std::ostringstream oss;
    oss << "/tmp/fifo." << qid;
    std::string fifoname = oss.str();

    // ファイルを消去
    unlink (fifoname.c_str());
}


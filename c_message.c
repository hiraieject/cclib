/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file c_message.c
 * @brief message send/recv class for C
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "c_message.h"

// ------------------------------------------------------- debug print macro
#undef ERRPR
#undef WARNPR
#undef DBGPR
// 
#define ERRPR(fmt, args...)                                             \
    { printf("[%s:%s():%d] ##### ERROR!: " fmt,"c_message",__FUNCTION__,__LINE__, ## args); }
#define WARNPR(fmt, args...)                                            \
    { printf("[%s:%s():%d] ##### WARNING!: " fmt,"c_message",__FUNCTION__,__LINE__, ## args); }
#define DBGPR(fmt, args...)                                             \
    if (enable_dbgpr) { printf("[%s:%s():%d] " fmt,"c_message",__FUNCTION__,__LINE__,## args); }
// -------------------------------------------------------

#define ENABLE_SENDLOG 1
//#define ENABLE_RECVLOG 1
//　 ２重にログがでるので、通常は送信のみでデバックする

static bool enable_dbgpr = true;

// ---------------------------------------------------------------------------------------------------

// 簡易JSON処理
void
json_add_property(char *json_str, const char *key, const char *value) {
    // Find the position of the last '}' character in the JSON string
    char *last_brace = strrchr(json_str, '}');
    
    if (last_brace != NULL) {
        // Create a new JSON string with the added property using sprintf
        sprintf(last_brace, ", \"%s\": \"%s\" }", key, value);
    }
}

void
json_get_value(const char *json_str, const char *key, char *value_buffer, size_t value_buffer_size)
{
    // キーをダブルクオーテーションで囲んだ文字列を構築
    char quoted_key[100]; // バッファーサイズを適切に調整
    snprintf(quoted_key, sizeof(quoted_key), "\"%s\":", key);

    const char *key_start = strstr(json_str, quoted_key); // ダブルクオーテーションで囲まれたキーを検索

    value_buffer[0] = '\0';     // デフォルトの空文字列を設定
    
    if (key_start != NULL) {
        key_start += strlen(quoted_key); // キーの終端の直後に移動

        // 値の開始位置を検索
        const char *value_start = strchr(key_start, '"');
        if (value_start != NULL) {
            value_start++; // " の直後に移動

            // 値の終端の " を検索
            const char *value_end = strchr(value_start, '"');
            if (value_end != NULL) {
                // 値をバッファーにコピーして NULL 終端する
                size_t value_length = value_end - value_start;
                if (value_length < value_buffer_size) {
                    strncpy(value_buffer, value_start, value_length);
                    value_buffer[value_length] = '\0';
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------

static message_packet send_packet;              // でかいかもなのでスタックには置かない、なおスレッドフリーは目指してない
static message_packet reply_packet;

bool
send_json (bool reply_required, char *sender, char *reciever, int reciever_key, char *send_json_str, char **reply_json_str)
{
    bool ret_bool = false;
    int reply_qid = -1;
    int reply_fd  = -1;
    int send_fd  = -1;

    *reply_json_str = (char*)"";

    // FIFO オープン
    int reciever_qid = msgget (reciever_key , 0666 | IPC_CREAT);
    if (reciever_qid == -1) {
        perror("msgget()");
        ERRPR ("message reciever qid create error\n");
        goto FINISH;
    }
    // 既存のFIFOをオープン、FIFOファイルがなければエラーになる
    char fifoname[128];
    snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%d", reciever_qid);
    if ((send_fd = open (fifoname, O_WRONLY|O_NONBLOCK)) == -1) {
        ERRPR("send fifo open error, filename=%s\n", fifoname);
        perror("open()");
        goto FINISH;
    }
    DBGPR ("FIFO %s opened\n", fifoname);

    // バッファークリアー
    memset ((void*)&send_packet,  0, sizeof(send_packet));
    memset ((void*)&reply_packet, 0, sizeof(reply_packet));

    // メッセージパケットの構築
    char *p;
    send_packet.com = COM_COMMON_JSON;
    p = stpncpy (send_packet.sender, sender,
                 CC_MESSAGE_SENDERNAME_MAXLEN);
    *p = '\0'; // 終端

    p = stpncpy (send_packet.reciever, reciever,
                 CC_MESSAGE_RECIEVERNAME_MAXLEN);
    *p = '\0'; // 終端

    p = stpncpy (send_packet.json_str,
                 send_json_str,
                 CC_MESSAGE_JSON_MAXLEN);
    *p = '\0'; // 終端

    // 返信FIFOを作成してオープン
    if (reply_required) {
        reply_qid = msgget (IPC_PRIVATE , 0666 | IPC_CREAT);
        if (reply_qid == -1) {
            perror("msgget()");
            ERRPR ("reply qid create error\n");
            goto FINISH;
        }
        char reply_qid_str[20];
        sprintf (reply_qid_str, "%d", reply_qid);
        json_add_property(send_packet.json_str, "reply_qid", reply_qid_str);

        // FIFO作成
        snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%d", reciever_qid);
        if (access(fifoname,F_OK) != 0) {
            // fifoファイルがない場合、fifoを新規に作成する
            if ((mkfifo(fifoname, 0666) < 0) && (errno != EEXIST)) {
                perror("mkfifo()");
                ERRPR("reply fifo create error, filename=%s\n", fifoname);
                goto FINISH;
            }
        }
        // オープン
        if ((reply_fd = open (fifoname, O_RDONLY)) == -1) {
            ERRPR("reply fifo open error, filename=%s\n", fifoname);
            perror("open()");
            goto FINISH;
        }
        DBGPR ("FIFO %s opened\n", fifoname);
    }

    // メッセージを送信
#if defined(ENABLE_SENDLOG)
    DBGPR ("now send message [%s -> %s]\n",
                      send_packet.sender, send_packet.reciever);
#endif
    int ret = write (send_fd, (void*)&send_packet, sizeof(send_packet)); // 送信
    if (ret == -1) {
        perror("write()");
        ERRPR ("message send error\n");
        ret_bool = true;
        goto FINISH;
    }

    // ------------------------------- 返信不要ならここで終了
    if (reply_required == false) {
        goto FINISH;
    }

    // ------------------------------- ここから返信パケット受信処理

    // select() でイベント待ち
    struct timeval timeout = { 5/*sec*/, 0/*usec*/ };
    fd_set      rfds;
    FD_ZERO(&rfds);
    FD_SET(reply_fd, &rfds);
    DBGPR ("select wait start\n");
    select(FD_SETSIZE, &rfds, 0, 0, &timeout);
    
    // 受信
    if (!FD_ISSET(reply_fd, &rfds)) {
        // FDSETされてない -> timeout
        ERRPR ("recv reply timed out\n");
        goto FINISH;
    }
    ret = read (reply_fd, (void*)&reply_packet, sizeof(reply_packet)); // 受信
    if (ret == -1) {
        perror("read()");
        ERRPR ("reply recv error\n");
        goto FINISH;
    }
    *reply_json_str = reply_packet.json_str;

    ret_bool = true;

 FINISH:
    if (send_fd != -1) {
        close(send_fd);
    }
    if (reply_qid != -1) {
        msgctl(reply_qid, IPC_RMID, NULL);
    }
    if (reply_fd != -1) {
        close(reply_fd);
        unlink(fifoname);       // ファイル消去
    }
    return ret_bool;
}


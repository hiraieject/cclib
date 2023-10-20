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
#define CC_MESSAGE_ERRPR(fmt, args...) \
        { printf("[%s:%s():%d] ##### ERROR!: " fmt,"c_message",__FUNCTION__,__LINE__, ## args); }
#define CC_MESSAGE_WARNPR(fmt, args...)                                                                                 \
        { printf("[%s:%s():%d] ##### WARNING!: " fmt,"c_message",__FUNCTION__,__LINE__, ## args); }
#define CC_MESSAGE_DBGPR(fmt, args...)  \
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

static message_packet send_packet;
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
        CC_MESSAGE_ERRPR ("message reciever qid create error\n");
        goto FINISH;
    }
    // 既存のFIFOをオープン、FIFOファイルがなければエラーになる
    char fifoname[128];
    snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%d", reciever_qid);
    if ((send_fd = open (fifoname, O_WRONLY|O_NONBLOCK)) == -1) {
        CC_MESSAGE_ERRPR("send fifo open error, filename=%s\n", fifoname);
        perror("open()");
        goto FINISH;
    }
    CC_MESSAGE_DBGPR ("FIFO %s opened\n", fifoname);

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
            CC_MESSAGE_ERRPR ("reply qid create error\n");
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
                CC_MESSAGE_ERRPR("reply fifo create error, filename=%s\n", fifoname);
                goto FINISH;
            }
        }
        // オープン
        if ((reply_fd = open (fifoname, O_RDONLY)) == -1) {
            CC_MESSAGE_ERRPR("reply fifo open error, filename=%s\n", fifoname);
            perror("open()");
            goto FINISH;
        }
        CC_MESSAGE_DBGPR ("FIFO %s opened\n", fifoname);
    }

    // メッセージを送信
#if defined(ENABLE_SENDLOG)
    CC_MESSAGE_DBGPR ("now send message [%s -> %s]\n",
                      send_packet.sender, send_packet.reciever);
#endif
    int ret = write (send_fd, (void*)&send_packet, sizeof(send_packet)); // 送信
    if (ret == -1) {
        perror("write()");
        CC_MESSAGE_ERRPR ("message send error\n");
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
    CC_MESSAGE_DBGPR ("select wait start\n");
    select(FD_SETSIZE, &rfds, 0, 0, &timeout);
    
    // 受信
    if (!FD_ISSET(reply_fd, &rfds)) {
        // FDSETされてない -> timeout
        CC_MESSAGE_ERRPR ("recv reply timed out\n");
        goto FINISH;
    }
    ret = read (reply_fd, (void*)&reply_packet, sizeof(reply_packet)); // 受信
    if (ret == -1) {
        perror("read()");
        CC_MESSAGE_ERRPR ("reply recv error\n");
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

#if 0
// ---------------------------------------------------------------- 下請け非公開API
/**
 * @brief FIFOをオープンする
 * @param[in] cm c_messageリソースポインタ
 * @param[in] qid FIFOのQID
 * @param[in] option O_RDONLY, O_WRONLY, O_RDWR
 * @return FIFOのファイルディスクリプタ -1:失敗
 */
static int
c_message_open_fifo (c_message *cm, int qid, int option)
{
    int fd = -1;
    // 内部関数なのでパラメーターチェックはなし

    // FIFO 作成
    char fifoname[128];
    snprintf(fifoname, sizeof(fifoname), "/tmp/fifo.%d", qid);
    if (access(fifoname,F_OK) != 0) {
        // fifoファイルがない場合、fifoを新規に作成する
        if ((mkfifo(fifoname, 0666) < 0) && (errno != EEXIST)) {
            CC_MESSAGE_ERRPR("fifo create error, filename=%s\n", fifoname);
            perror("mkfifo()");
            goto FINISH;
        }
    }
    
    // FIFO オープン
    if ((fd = open(fifoname, option | O_NONBLOCK, 0)) == -1) {
        //  -> WR側が先にオープンするケースでRDがオープンするまでブロックされるようにNONBLOCKは指定しない
        //if ((fd = open(fifoname, option, 0)) == -1) {
        CC_MESSAGE_ERRPR("fifo open error, filename=%s\n", fifoname);
        perror("open()");
        goto FINISH;
    }
    CC_MESSAGE_DBGPR ("FIFO %s opened\n", fifoname);
 FINISH:
    return fd;
}

// ---------------------------------------------------------------- debug
/**
 * @brief debug: ニックネームを設定して、デバックプリントをオンにする
 */
void
c_message_enable_dbg (c_message *cm)
{
    // パラメーターチェック
    if (!cm) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // フラグ書き換え
    cm->enable_dbgpr = true;

 FINISH:
    return;
}

/**
 * @brief debug: ニックネームを設定して、デバックプリントをオフにする
 */
void
c_message_disable_dbg (c_message *cm)
{
    // パラメーターチェック
    if (!cm) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // フラグ書き換え
    cm->enable_dbgpr = false;

 FINISH:
    return;
}

// ---------------------------------------------------------------- 送信・受信共通API

/**
 * @brief メッセージインスタンス生成
 */
void
c_message_mdel (c_message *cm)
{
    // パラメーターチェック
    if (!cm) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // あとしまつ
    close (cm->mfd);
    free (cm->nickname);
    free (cm);
    if (cm->masterflg) {
        // master側だけのあとしまつ
    }
    cm = NULL;
    CC_MESSAGE_DBGPR ("message resource deleted\n");
 FINISH:
    return;
}

/**
 * @brief メッセージインスタンス破棄
 */
c_message *
c_message_mnew (int message_key, int message_size, char *nickname, bool masterflg)
{
    c_message *cm = NULL;
    
    // パラメーターチェック
    if (message_size <= 0 || !nickname) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }

    // alloc c_message resource
    cm = (c_message*)malloc(sizeof(c_message));
    if (cm == NULL) {
        //CC_MESSAGE_ERRPR ("message resource create error\n");
        printf ("message resource create error\n");
        perror("malloc()");
        goto FINISH;
    }
    memset (cm,0,sizeof(c_message)); // initialize
    cm->mqid = -1;
    cm->mfd  = -1;
    cm->rqid = -1;
    cm->masterflg = masterflg;
    cm->nickname  = strdup(nickname);
    cm->enable_dbgpr = false;
    
    // get message qid
    cm->mqid = msgget (message_key , 0666 | IPC_CREAT);
    if (cm->mqid == -1) {
        CC_MESSAGE_ERRPR ("message mqid create error\n");
        perror("msgget()");
        c_message_mdel (cm);
        cm = NULL;
        goto FINISH;
    }
    // open message fifo
    if ((cm->mfd=c_message_open_fifo(cm, cm->mqid, masterflg ? O_RDWR : O_WRONLY)) == -1) {
        CC_MESSAGE_ERRPR ("message fifo open error\n");
        c_message_mdel (cm);
        cm = NULL;
        goto FINISH;
    }
    CC_MESSAGE_DBGPR ("message fifo opened\n");

    // save message size
    cm->msize = (size_t)message_size;

    // 返信qidの生成
    if (message_key != IPC_PRIVATE) {
        // 親キーがPRIVATEじゃないときだけ作成
        cm->rqid = msgget (IPC_PRIVATE , 0666 | IPC_CREAT);
        if (cm->rqid == -1) {
            perror("msgget()");
            CC_MESSAGE_ERRPR ("reply rqid create error\n");
            c_message_mdel (cm);
            cm = NULL;
            goto FINISH;
        }
    }

 FINISH:
    return cm;
}

// ---------------------------------------------------------------- 送信側API

/**
 * @brief 送信メッセージを初期化する
 */
void
c_message_init_message (c_message *cm, void *message_buffer, int com, char *sender, bool reply_required)
{
    // パラメーターチェック
    if (!cm || !message_buffer || !sender) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // メッセージバッファー初期化
    memset (message_buffer, 0, cm->msize);
    ((message_packet*)message_buffer)->com  = com;
    ((message_packet*)message_buffer)->rqid = reply_required ? cm->rqid : -1;

    char *p;
    p = stpncpy (((message_packet*)message_buffer)->sender,
                 sender, CC_MESSAGE_SENDERNAME_MAXLEN);     // バッファーサイズ-1をコピー
    *p = '\0';                                              // 終端
 FINISH:
    return;
}

/**
 * @brief 送信メッセージを初期化する(JSON)
 */
void
c_message_init_message_json (c_message *cm, void *message_buffer, char *json_str, char *sender, bool reply_required)
{
    // パラメーターチェック
    if (!cm || !message_buffer || !json_str || !sender) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    int com = COM_COMMON_JSON;

    // メッセージバッファー初期化
    memset (message_buffer, 0, cm->msize);
    ((message_packet*)message_buffer)->com  = com;
    ((message_packet*)message_buffer)->rqid = reply_required ? cm->rqid : -1;

    char *p;
    p = stpncpy (((message_packet*)message_buffer)->sender,
                 sender, CC_MESSAGE_SENDERNAME_MAXLEN);     // バッファーサイズ-1をコピー
    *p = '\0';                                              // 終端
    p = stpncpy (((message_packet*)message_buffer)->json_str,
                 json_str, CC_MESSAGE_JSON_MAXLEN);         // バッファーサイズ-1をコピー
    *p = '\0';                                              // 終端
 FINISH:
    return;
}

/**
 * @brief 送信メッセージを送信する
 */
int
c_message_send_message (c_message *cm, void *message_buffer)
{
    int ret = -1;

    // パラメーターチェック
    if (!cm || !message_buffer) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // メッセージを送信
#if defined(ENABLE_SENDLOG)
    CC_MESSAGE_DBGPR ("now send message com=%d [%s -> %s]\n",
                      ((message_packet*)message_buffer)->com,
                      ((message_packet*)message_buffer)->sender, cm->nickname);
#endif
    ret = write (cm->mfd,message_buffer,cm->msize);
    if (ret == -1) {
        perror("write()");
        CC_MESSAGE_ERRPR ("message send error\n");
        goto FINISH;
    }
 FINISH:
    return ret;
}

/**
 * @brief 送信メッセージ(コマンドのみ)を送信する
 */
int
c_message_send_message_comonly (c_message *cm, int com, char *sender, bool reply_required)
{
    int ret = -1;

    // メッセージバッファーの確保・初期化
    void *message_buffer = malloc (cm->msize);
    if (message_buffer == NULL) {
        perror("malloc()");
        CC_MESSAGE_ERRPR ("message buffer malloc error\n");
        goto FINISH;
    }
    // メッセージ初期化＆送信、エラーチェックはAPIにおまかせ
    c_message_init_message (cm, message_buffer, com, sender, reply_required);
    ret = c_message_send_message (cm, message_buffer);
    
 FINISH:
    if (message_buffer) free(message_buffer);
    return ret;
}

/**
 * @brief 送信メッセージ(JSON)を送信する
 */
int  c_message_send_message_json (c_message *cm, char *json_str, char *sender, bool reply_required)
{
    int ret = -1;
    
    // メッセージバッファーの確保・初期化
    void *message_buffer = malloc (cm->msize);
    if (message_buffer == NULL) {
        perror("malloc()");
        CC_MESSAGE_ERRPR ("message buffer malloc error\n");
        goto FINISH;
    }
    // メッセージ初期化＆送信、エラーチェックはAPIにおまかせ
    c_message_init_message_json (cm, message_buffer, json_str, sender, reply_required);
    ret = c_message_send_message (cm, message_buffer);
    
 FINISH:
    if (message_buffer) free(message_buffer);
    return ret;
}

/**
 * @brief 返信メッセージを受信する
 */
int
c_message_recv_reply (c_message *cm, void *reply_buffer, void *sended_message)
{
    int ret = -1;
    int fd  = -1;

    // パラメーターチェック
    if (!cm || !reply_buffer || !sended_message) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    int rqid = ((message_packet*)sended_message)->rqid;
    if (rqid == -1) {
        CC_MESSAGE_ERRPR ("rqid is -1 in sended message\n");
        goto FINISH;
    }
    // 一時的に返信FIFOをオープンする
    if ((fd=c_message_open_fifo(cm, rqid, O_RDONLY)) == -1) {
        CC_MESSAGE_ERRPR ("fifo for reply open error\n");
        goto FINISH;
    }
    // select() でイベント待ち
    struct timeval timeout = { 3/*sec*/, 0/*usec*/ };
    fd_set      rfds;
    FD_ZERO(&rfds);
    FD_SET(fd, &rfds);
    CC_MESSAGE_DBGPR ("select wait start\n");
    select(FD_SETSIZE, &rfds, 0, 0, &timeout);
    
    // 受信して即FIFOクローズ
    if (!FD_ISSET(fd, &rfds)) {
        // FDSETされてない -> timeout
        CC_MESSAGE_ERRPR ("recv reply time out\n");
        goto FINISH;
    }
    ret = read (fd, reply_buffer, cm->msize); // 受信
    if (ret == -1) {
        perror("read()");
        CC_MESSAGE_ERRPR ("reply recv error\n");
        goto FINISH;
    }
#if defined(ENABLE_RECVLOG)
    CC_MESSAGE_DBGPR ("recved reply [%s -> %s:%d]\n",
                      ((message_packet*)reply_buffer)->sender, ((message_packet*)sended_message)->sender, rqid);
#endif
    
 FINISH:
    if (fd != -1) close(fd);
    return ret;
}

// ---------------------------------------------------------------- 受信側API

/**
 * @brief 送信メッセージを受信する
 */
int
c_message_recv_message (c_message *cm, void *message_buffer)
{
    int ret = -1;
    
    // パラメーターチェック
    if (!cm || !message_buffer) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // メッセージ受信
    ret = read (cm->mfd,message_buffer,cm->msize); // 受信
    if (ret == -1) {
        perror("read()");
        CC_MESSAGE_ERRPR ("message read error\n");
        goto FINISH;
    } else {
#if defined(ENABLE_RECVLOG)
        CC_MESSAGE_DBGPR ("recved message com=%d [%s -> %s]\n",
                          ((message_packet*)(message_buffer))->com,
                          ((message_packet*)message_buffer)->sender, cm->nickname);
#endif
    }
 FINISH:
    return ret;
}

/**
 * @brief 返信メッセージを初期化する
 */
void
c_message_init_reply (c_message *cm, void *reply_buffer, int result, char *sender)
{
    // パラメーターチェック
    if (!cm || !reply_buffer || !sender) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // 返信バッファー初期化
    memset (reply_buffer, 0, cm->msize);
    ((message_packet*)reply_buffer)->com = 1;          // replyのcomは１固定
    ((message_packet*)reply_buffer)->result = result;
    ((message_packet*)reply_buffer)->rqid =  -1;

    char *p;
    p = stpncpy (((message_packet*)reply_buffer)->sender,
                 sender, CC_MESSAGE_SENDERNAME_MAXLEN);     // バッファーサイズ-1をコピー
    *p = '\0';                                              // 終端

 FINISH:
    return;
}

/**
 * @brief 返信メッセージを初期化する(JSON)
 */
void c_message_init_reply_json (c_message *cm, void *reply_buffer, char *json_str, int result, char *sender)
{
    // パラメーターチェック
    if (!cm || !reply_buffer || !json_str || !sender) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // 返信バッファー初期化
    memset (reply_buffer, 0, cm->msize);
    ((message_packet*)reply_buffer)->com = 1;          // replyのcomは１固定
    ((message_packet*)reply_buffer)->result = result;
    ((message_packet*)reply_buffer)->rqid =  -1;

    char *p;
    p = stpncpy (((message_packet*)reply_buffer)->sender,
                 sender, CC_MESSAGE_SENDERNAME_MAXLEN);     // バッファーサイズ-1をコピー
    *p = '\0';                                              // 終端
    p = stpncpy (((message_packet*)reply_buffer)->json_str,
                 json_str, CC_MESSAGE_JSON_MAXLEN);         // バッファーサイズ-1をコピー
    *p = '\0';                                              // 終端

 FINISH:
    return;
}

/**
 * @brief 返信メッセージを送信する
 */
int
c_message_send_reply (c_message *cm, void *reply_buffer, void *recved_message)
{
    int ret = -1;
    int fd  = -1;

    // パラメーターチェック
    if (!cm || !reply_buffer || !recved_message) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    int rqid = ((message_packet*)recved_message)->rqid;
    if (rqid == -1) {
        CC_MESSAGE_ERRPR ("rqid is -1 in recved message\n");
        goto FINISH;
    }
    // 一時的に返信FIFOをオープンする
    if ((fd=c_message_open_fifo(cm, rqid, O_WRONLY)) == -1) {
        CC_MESSAGE_ERRPR ("fifo for reply open error\n");
        goto FINISH;
    }
    // 送信する
#if defined(ENABLE_SENDLOG)
    CC_MESSAGE_DBGPR ("now send reply [%s -> %s:%d], \n",
                      ((message_packet*)reply_buffer)->sender, ((message_packet*)recved_message)->sender, rqid);
#endif
    ret = write (fd,reply_buffer,cm->msize); // 送信
    if (ret == -1) {
        perror("write()");
        CC_MESSAGE_ERRPR ("message send error\n");
        goto FINISH;
    }

 FINISH:
    // FIFOは即クローズ
    if (fd != -1) close (fd);
    return ret;
}

/**
 * @brief 返信メッセージ(resultのみ)を送信する
 */
int
c_message_send_reply_resultonly (c_message *cm, int result, char *sender, void *recved_message)
{
    int ret = -1;
    void *reply_buffer = NULL;

    // パラメーターチェック
    if (!cm || !sender || !reply_buffer) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // 送信バッファーの確保
    reply_buffer = malloc (cm->msize);
    if (reply_buffer == NULL) {
        perror("malloc()");
        CC_MESSAGE_ERRPR("send buffer malloc error");
        goto FINISH;
    } 
    c_message_init_reply (cm, reply_buffer, result, sender);
    ret = c_message_send_reply (cm, reply_buffer, recved_message);

 FINISH:
    if (reply_buffer) free(reply_buffer);
    return ret;
}

/**
 * @brief 返信メッセージ(resultのみ)を送信する
 */
int
c_message_send_reply_json (c_message *cm, char *json_str, int result, char *sender, void *recved_message)
{
    int ret = -1;
    void *reply_buffer = NULL;

    // パラメーターチェック
    if (!cm || !json_str || !sender || !recved_message) {
        CC_MESSAGE_ERRPR ("parameter error\n");
        goto FINISH;
    }
    // 送信バッファーの確保
    reply_buffer = malloc (cm->msize);
    if (reply_buffer == NULL) {
        perror("malloc()");
        CC_MESSAGE_ERRPR("send buffer malloc error");
        goto FINISH;
    } 
    c_message_init_reply_json (cm, reply_buffer, json_str, result, sender);
    ret = c_message_send_reply (cm, reply_buffer, recved_message);

 FINISH:
    if (reply_buffer) free(reply_buffer);
    return ret;
}

#endif


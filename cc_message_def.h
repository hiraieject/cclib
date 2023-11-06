/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_message_def.h
 * @brief c/c++ common message definition 
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __C_MESSAGE_def_H__
#define __C_MESSAGE_def_H__

#ifdef __cplusplus
#include <cstddef>  // for size_t
#else
#include <stddef.h> // for size_t
#endif

#ifdef __cplusplus
extern "C" {
#if 0
} // dummy
#endif
#endif /* __cplusplus */

// コマンド 共通定義
enum {
    COM_COMMON_INVALID,
    COM_COMMON_JSON,
    COM_COMMON_USERCOM,
};

#define CC_MESSAGE_SENDERNAME_MAXLEN 15
#define CC_MESSAGE_RECEIVERNAME_MAXLEN 15
#define CC_MESSAGE_JSON_MAXLEN 400

typedef struct message_packet {
    // basic only
    int  com;                 //< q command   : must not 0
    int  rqid;                //< reply qid   : -1:invalid, other:valid
    int  result;              //< result core : 0:OK, other:NG
    char sender[CC_MESSAGE_SENDERNAME_MAXLEN+1];
    char receiver[CC_MESSAGE_RECEIVERNAME_MAXLEN+1];
    char json_str[CC_MESSAGE_JSON_MAXLEN+1];  ///< JSON 文字列
} message_packet;

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __C_MESSAGE_def_H__

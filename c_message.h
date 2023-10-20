/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file c_message.h
 * @brief message send/recv class for C
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __C_MESSAGE_H__
#define __C_MESSAGE_H__

#include <stdbool.h>

#include "cc_message_def.h"

#ifdef __cplusplus
extern "C" {
#if 0
} // dummy
#endif
#endif /* __cplusplus */

// ---------------------------------------------------------------- new API
// JSON文字列を送信し、必要なら返信を返す
// 　スレッドフリーではないので、必要なら呼ぶ側でmutexで囲むなどしてください
bool send_json (bool reply_required, char *sender, char *reciever, int reciever_key, char *send_json_str, char **reply_json_str);

void json_add_property(char *json_str, const char *key, const char *value);
void json_get_value(const char *json_str, const char *key, char *value_buffer, size_t value_buffer_size);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif // __C_MESSAGE_H__

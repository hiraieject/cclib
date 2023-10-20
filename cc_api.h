/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_jaction.h
 * @brief test core class
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_JACTION_H__
#define __CC_JACTION_H__

#include <string>
#include <nlohmann/json.hpp>

/**
 * @class cc_jaction
 * @brief JSONデータを受け取って処理するAPIを提供するクラス
 */
class cc_api {
public:
    // JSON ACTION API
    virtual bool jaction(std::string &param_json_str, std::string &result_json_str)
    {
        // JSONデータ
        nlohmann::json param_json_data  = nlohmann::json::parse(param_json_str);
        nlohmann::json result_json_data;

        // API呼び出し
        bool ret = jaction(param_json_data, result_json_data);

        // JSON文字列に変換
        // TODO result_json_data -> result_json_str

        return ret;
    };
    virtual bool jaction(nlohmann::json &param_json_data, nlohmann::json &result_json_data) {
        // TODO
        return true;
    };
};

#endif // __CC_JACTION_H__

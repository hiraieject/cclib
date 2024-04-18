/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_message.h
 * @brief message send/recv class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_MESSAGE_H__
#define __CC_MESSAGE_H__

#include <nlohmann/json.hpp>
#include <sstream>		// for std::ostringstream

#include "cc_message_def.h"
#include "cc_debugprint.h"


#define MKSENDSTR(_sender,_receiver,_json_str)                            \
    (R"({"sender":")" _sender R"(","receiver":")" _receiver R"(",)" _json_str R"(})")
    

class cc_message_data {
public:
    cc_message_data(int arg_com=0);
    virtual int get_size(void);
};

class cc_message_base {
private:
public:
    cc_message_base (std::string nickname);
    virtual ~cc_message_base ();

    // ----------------------------------------------- API for receiver
    virtual int receiver_get_fd (void) {return -1;};
    virtual std::string receiver_recv_json_str (void) {return "";};

    // ----------------------------------------------- API for sender
    std::string send_json (nlohmann::json &send_json_obj) {return "";};
    std::string send_json (bool reply_required, std::string sender, std::string send_json_str) {return "";};

    std::string nickname;
    
    cc_debugprint message_dbg;                   ///< cclib debugprint
    void enable_dbg(void) {
        message_dbg.enable();
    }
    void disable_dbg(void) {
        message_dbg.disable();
    }
};

class cc_message : public cc_message_base{
private:
    int send_qid;
    int send_fd;
    int master_flag;

    int open_fifo (int qid, int option);
    void close_fifo (int fd);
    void destroy_fifo (int qid);

public:
    cc_message (key_t send_key, std::string nickname, bool master_falg);
    ~cc_message ();

    // ----------------------------------------------- API for receiver
    int receiver_get_fd (void);
    std::string receiver_recv_json_str (void);

    // ----------------------------------------------- API for sender
    std::string send_json (nlohmann::json &send_json_obj);
    std::string send_json (bool reply_required, std::string sender, std::string send_json_str);
    std::string send_json (bool reply_required, std::string sender, nlohmann::json &send_json_obj);
};

#endif // __CC_MESSAGE_H__


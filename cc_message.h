/* -*- Mode: C++; tab-width: 4; c-basic-offset: 4 -*- */

/**
 * @file cc_message.h
 * @brief message send/recv class for C++
 */

#ifndef __CC_MESSAGE_H__
#define __CC_MESSAGE_H__

#include <nlohmann/json.hpp>

#include "cc_custom_def.h"
#include "cc_message_def.h"
#include "cc_debugprint.h"


#define MKSENDSTR(_sender,_reciever,_json_str)                            \
    (R"({"sender":")" _sender R"(","reciever":")" _reciever R"(",)" _json_str R"(})")
    

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

    // ----------------------------------------------- API for reciever
    virtual int reciever_get_fd (void) {return -1;};
    virtual std::string reciever_recv_json_str (void) {return "";};

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

    // ----------------------------------------------- API for reciever
    int reciever_get_fd (void);
    std::string reciever_recv_json_str (void);

    // ----------------------------------------------- API for sender
    std::string send_json (nlohmann::json &send_json_obj);
    std::string send_json (bool reply_required, std::string sender, std::string send_json_str);
};

#endif // __CC_MESSAGE_H__


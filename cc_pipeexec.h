/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_pipeexec.h
 * @brief pipe exec class for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_PIPEEXEC_H__
#define __CC_PIPEEXEC_H__

#define AVAILABLE_PIPEEXEC
#ifndef __MINGW64__
#undef AVAILABLE_PIPEEXEC
#endif


#ifdef AVAILABLE_PIPEEXEC

#include <iostream>
#include <cstdio>
#include <vector>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <string.h>

class cc_pipeexec {
private:
    int pipe_fd;
    bool outputModified;
    std::vector<std::string> outputLines;
public:
    std::mutex mtx;                     ///< 変数保護用MUTEX @n 変数を直接参照する時にはロックすること
    pid_t pid;

    cc_pipeexec(void);
    
    bool executeCommand(std::string &cmd);
    bool getOutputAll(std::vector<std::string> &result);
    bool pipeClose(void);
    void finish(pid_t finpid);

    std::vector<std::string> simpleExecuteCommand(std::string &cmd);

    int get_fd(void);
};

#endif // AVAILABLE_PIPEEXEC

#endif // __CC_PIPEEXEC_H__

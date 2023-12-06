/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

/**
 * @file cc_misc.h
 * @brief misc functions for C++
 * 
 * Copyright (c) 2023 Yoshikazu Hirai
 * Released under the MIT license
 * https://opensource.org/licenses/mit-license.php
 * 
 */

#ifndef __CC_MISC_H__
#define __CC_MISC_H__

#include <string>
#include <vector>

bool cc_checkConnection(const std::string& hostname, int port, int timeout);

enum class cc_SortOrder {
    None,            // ソートなし
    Ascending,       // 昇順
    Descending,      // 降順
    DateAscending,   // 日付昇順
    DateDescending   // 日付降順
};
void cc_getFilelist (const std::string& dir_path, std::vector<std::string>& filelist, int maxfiles, const std::string &extention, cc_SortOrder order = cc_SortOrder::None);
void cc_getFolderlist (const std::string& dir_path, std::vector<std::string>& folderlist, int maxfolders, cc_SortOrder order);

std::string cc_readFileBody(const std::string& filename);

void cc_writeFileBody(const std::string& filename, const std::string& content);


#endif // __CC_MISC_H__

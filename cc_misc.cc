#include "cc_misc.h"

#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <dirent.h>
#include <algorithm> // sort関数のため

bool
cc_checkTcpConnection(const std::string& hostname, int port, int timeout)
{
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // IPv4とIPv6の両方を許可
    hints.ai_socktype = SOCK_STREAM; // TCP

    if (getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
        std::cerr << "Error resolving host\n";
        return false;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        freeaddrinfo(res);
        return false;
    }

    // タイムアウトの設定
    if (timeout > 0) {
        struct timeval tv;
        tv.tv_sec = timeout;    // 秒
        tv.tv_usec = 0;         // 0ミリ秒
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    }

    bool ret = true;
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Connection failed\n";
        ret = false;
    }
    close(sock);
    freeaddrinfo(res);

    return ret;
}


void
cc_getFilelist (const std::string& dir_path, std::vector<std::string>& filelist, int maxfiles, const std::string &extention, cc_SortOrder order)
{
    DIR* dir = opendir(dir_path.c_str());
    struct dirent* entry;
    if (dir == nullptr) {
        return;
    }
    int count = 0;
    while ((entry = readdir(dir)) != nullptr) {
        std::string filename(entry->d_name);
        if (filename.size() >= extention.size() && filename.compare(filename.size() - extention.size(), extention.size(), extention) == 0) {
            filelist.push_back(filename);
        }
        if (++count > maxfiles) {
            break;              // 最大数リミッター
        }
    }
    closedir(dir);

    // ソート順に基づいてリストをソート
    if (order == cc_SortOrder::Ascending) {
        std::sort(filelist.begin(), filelist.end());
    } else if (order == cc_SortOrder::Descending) {
        std::sort(filelist.begin(), filelist.end(), std::greater<std::string>());
    }
}

std::string
cc_readFileBody(const std::string& filename)
{
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file\n\nFilename: " + filename);
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    std::string content = buffer.str();

    // 改行コードを \n に統一する
    size_t pos = 0;
    while ((pos = content.find("\r\n", pos)) != std::string::npos) {
        content.replace(pos, 2, "\n");
        pos += 1; // '\n'の分だけ進める
    }

    return content;
}

void
cc_writeFileBody(const std::string& filename, const std::string& content)
{
    std::ofstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Failed to open file\n\nFilename: " + filename);
    }
    file << content;
}

void
cc_removeFile(const std::string& filename)
{
    if (remove(filename.c_str()) != 0) {
        throw std::runtime_error("Failed to delete file\n\nFilename: " + filename);
    }
}

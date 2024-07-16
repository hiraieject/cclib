/* -*- Mode: C++; tab-width: 8; c-basic-offset: 4 -*- */

#include "cc_misc.h"

#include <iostream>
#include <sys/stat.h>
#include <unistd.h>
#include <fstream>
#include <sstream>
#include <cstring>
#include <dirent.h>
#include <algorithm> // sort関数のため
#include <vector>
#include <string>
#include <filesystem>

#ifdef __MINGW64__
#include <winsock2.h>
#include <ws2tcpip.h>
#else // Linux
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#endif

bool
cc_checkTcpConnection(const std::string& hostname, int port, int timeout)
{
#ifdef __MINGW64__
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return false;
    }

    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;    // IPv4とIPv6の両方を許可
    hints.ai_socktype = SOCK_STREAM; // TCP

    if (getaddrinfo(hostname.c_str(), std::to_string(port).c_str(), &hints, &res) != 0) {
        std::cerr << "Error resolving host\n";
        WSACleanup();
        return false;
    }

    int sock = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock < 0) {
        std::cerr << "Socket creation error\n";
        freeaddrinfo(res);
        WSACleanup();
        return false;
    }

    // タイムアウトの設定
    if (timeout > 0) {
        DWORD tv = timeout * 1000; // ミリ秒に変換
        setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
    }

    bool ret = true;
    if (connect(sock, res->ai_addr, res->ai_addrlen) < 0) {
        std::cerr << "Connection failed\n";
        ret = false;
    }

    closesocket(sock);
    freeaddrinfo(res);
    WSACleanup();

    return ret;
#else
    // Linux
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
#endif // else __MINGW64__
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
    } else if (order == cc_SortOrder::DateAscending) {
        // 日付昇順にソート
        std::sort(filelist.begin(), filelist.end(), [](const std::string& a, const std::string& b) {
            struct stat statA, statB;
            stat(a.c_str(), &statA);
            stat(b.c_str(), &statB);
            return statA.st_mtime < statB.st_mtime;
        });
    } else if (order == cc_SortOrder::DateDescending) {
        // 日付降順にソート
        std::sort(filelist.begin(), filelist.end(), [](const std::string& a, const std::string& b) {
            struct stat statA, statB;
            stat(a.c_str(), &statA);
            stat(b.c_str(), &statB);
            return statA.st_mtime > statB.st_mtime;
        });
    }
}

void
cc_getFolderlist (const std::string& dir_path, std::vector<std::string>& folderlist, int maxfolders, cc_SortOrder order)
{
    // C++17
    namespace fs = std::filesystem;
  
    fs::path path(dir_path);
    if (!fs::exists(path) || !fs::is_directory(path)) {
	return;
    }
  
    int count = 0;
    for (const auto& entry : fs::directory_iterator(path)) {
	if (entry.is_directory()) {
	    std::string foldername = entry.path().filename().string();
	    if (foldername != "." && foldername != "..") {
		folderlist.push_back(foldername);
	    }
	}
	if (++count >= maxfolders) {
	    break; // 最大数リミッター
	}
    }
  
    // ソート順に基づいてリストをソート
    if (order == cc_SortOrder::Ascending) {
        std::sort(folderlist.begin(), folderlist.end());
    } else if (order == cc_SortOrder::Descending) {
        std::sort(folderlist.begin(), folderlist.end(), std::greater<std::string>());
    } else if (order == cc_SortOrder::DateAscending) {
        // 日付昇順にソート
        std::sort(folderlist.begin(), folderlist.end(), [&dir_path](const std::string& a, const std::string& b) {
            struct stat statA, statB;
            stat((dir_path + "/" + a).c_str(), &statA);
            stat((dir_path + "/" + b).c_str(), &statB);
            return statA.st_mtime < statB.st_mtime;
        });
    } else if (order == cc_SortOrder::DateDescending) {
        // 日付降順にソート
        std::sort(folderlist.begin(), folderlist.end(), [&dir_path](const std::string& a, const std::string& b) {
            struct stat statA, statB;
            stat((dir_path + "/" + a).c_str(), &statA);
            stat((dir_path + "/" + b).c_str(), &statB);
            return statA.st_mtime > statB.st_mtime;
        });
    }
    
#if 0 // POSIX
    DIR* dir = opendir(dir_path.c_str());
    struct dirent* entry;
    if (dir == nullptr) {
        return;
    }
    int count = 0;
    while ((entry = readdir(dir)) != nullptr) {
        if (entry->d_type == DT_DIR) {
            std::string foldername(entry->d_name);
            // '.' または '..' のエントリは無視する
            if (foldername != "." && foldername != "..") {
                folderlist.push_back(foldername);
            }
        }
        if (++count > maxfolders) {
            break;              // 最大数リミッター
        }
    }
    closedir(dir);

    // ソート順に基づいてリストをソート
    if (order == cc_SortOrder::Ascending) {
        std::sort(folderlist.begin(), folderlist.end());
    } else if (order == cc_SortOrder::Descending) {
        std::sort(folderlist.begin(), folderlist.end(), std::greater<std::string>());
    } else if (order == cc_SortOrder::DateAscending) {
        // 日付昇順にソート
        std::sort(folderlist.begin(), folderlist.end(), [](const std::string& a, const std::string& b) {
            struct stat statA, statB;
            stat(a.c_str(), &statA);
            stat(b.c_str(), &statB);
            return statA.st_mtime < statB.st_mtime;
        });
    } else if (order == cc_SortOrder::DateDescending) {
        // 日付降順にソート
        std::sort(folderlist.begin(), folderlist.end(), [](const std::string& a, const std::string& b) {
            struct stat statA, statB;
            stat(a.c_str(), &statA);
            stat(b.c_str(), &statB);
            return statA.st_mtime > statB.st_mtime;
        });
    }
#endif
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

// WindowsスタイルのパスをMINGWスタイルのパスに変換する関数
std::string
cc_winPathToMingwPath(const std::string& windowsPath) {
    if (windowsPath.length() < 2 || windowsPath[1] != ':') {
        // パスが "C:\..." のような形式でない場合はエラー
        return "";
    }

    std::string mingwPath = "/" + std::string(1, tolower(windowsPath[0])) + windowsPath.substr(2);
    std::replace(mingwPath.begin(), mingwPath.end(), '\\', '/');
    return mingwPath;
}

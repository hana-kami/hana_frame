/*
 * @Author: hanakami
 * @Date: 2025-05-08 17:08:00
 * @email: hanakami@163.com
 * @LastEditTime: 2025-05-28 15:26:22
 * @FilePath: /hana_frame/src/function/json_func.cc
 * @Description: 
 * Copyright (c) 2025 by hanakami, All Rights Reserved. 
 */

#include "json_func.h"
#include <cdlog.h>

#include <iostream>
#include <fstream>
#include <ghc/filesystem.hpp>


bool convertStringToJson(const std::string& str, Json::Value& root) {
    Json::Reader reader(Json::Features::strictMode());
    if (reader.parse(str, root)) {
        return true;
    }
    LOGE("Json format error!!!!");
    return false;
}

bool convertJsonToString(const Json::Value& root, std::string& str) {
    Json::StreamWriterBuilder writer;
    str = Json::writeString(writer, root);
    return true;
}

bool loadLocalJson(const std::string& filePath, Json::Value& root) {
    // 文件不存在或者是一个目录
    ghc::filesystem::path  checkFilePath(filePath);
    if (!ghc::filesystem::exists(checkFilePath) || ghc::filesystem::is_directory(checkFilePath)) {
        LOGE("Filepath no exists.");
        return false;
    }

    // 打开文件
    std::ifstream  pfile;
    pfile.open(filePath, std::ios::binary);
    if (!pfile.is_open()) {
        LOGE("Can not open %s", filePath.c_str());
        return false;
    }

    // 获取文件大小
    pfile.seekg(0, std::ios::end);
    std::streampos fileSize = pfile.tellg();
    pfile.seekg(0, std::ios::beg);
    pfile.clear();

    // 转储文件 
    char data[1024];
    std::string content;
    do {
        pfile.read(data, sizeof(data));
        size_t extracted = pfile.gcount();
        content.append(data, extracted);
    } while (!pfile.eof());
    pfile.close();

    return convertStringToJson(content, root);
}

bool saveLocalJson(const std::string& filePath, const Json::Value& root) {
    // 路径检测
    ghc::filesystem::path  checkFilePath(filePath);
    if (ghc::filesystem::exists(checkFilePath)) {
        if (ghc::filesystem::is_regular_file(checkFilePath)) {
            // 如果是同名文件则删除文件
            ghc::filesystem::remove(checkFilePath);
        } else if (ghc::filesystem::is_directory(checkFilePath)) {
            LOGE("A directory with the same name already exists.");
            return false;
        }
    }

    // 检测文件是否打开成功
    std::ofstream outputFile(filePath);
    if (!outputFile.is_open()) {
        LOG(ERROR) << "Failed to open output file for writing";
        return false;
    }

    // 检测json是否转换成功
    std::string jsonString;
    if (!convertJsonToString(root, jsonString)) {
        LOG(ERROR) << "Failed to transion json to string";
        return false;
    }

    // 保存文件
    outputFile << jsonString;
    outputFile.close();

#if 1    
    sync();
#else
    // 获取文件描述符并刷新数据到磁盘
    int fileDescriptor = outputFile.rdbuf()->pubseekoff(0, outputFile.beg, outputFile.out);
    if (fileDescriptor != -1) {
        if (fsync(fileDescriptor) == -1) {
            LOG(ERROR) << "Failed to flush data to disk";
            return false;
        }
    } else {
        LOG(ERROR) << "Failed to get file descriptor";
        return false;
    }
#endif

    return true;
}

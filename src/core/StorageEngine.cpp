#include "StorageEngine.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <algorithm>

namespace checkstore {

namespace fs = std::filesystem;

LocalStorageEngine::LocalStorageEngine(const fs::path& root_path) : root_path_(root_path) {
    // 确保根目录存在
    if (!fs::exists(root_path_)) {
        fs::create_directories(root_path_);
    }
}

Result<std::vector<SafetensorsFileInfo>> LocalStorageEngine::scanSafetensors(const std::string& directory) {
    std::vector<SafetensorsFileInfo> files;
    try {
        for (const auto& entry : fs::directory_iterator(directory)) {
            if (entry.is_regular_file() && entry.path().extension() == ".safetensors") {
                SafetensorsFileInfo info;
                info.name = entry.path().filename().string();
                info.original_size = entry.file_size();
                info.md5 = ""; // TODO: 计算 MD5，由 IntegrityVerifier 处理
                info.compressed_size = 0; // 扫描时未压缩
                info.storage_key = ""; // 扫描时未存储
                files.push_back(info);
            }
        }
        return {true, "", files};
    } catch (const std::exception& e) {
        return {false, "Failed to scan directory: " + std::string(e.what()), {}};
    }
}

Result<SafetensorsFileInfo> LocalStorageEngine::writeSafetensors(
    const std::string& model,
    const std::string& tag,
    const std::string& file_name,
    const std::vector<uint8_t>& compressed_data) {

    try {
        fs::path dir = getCheckpointDir(model, tag);
        fs::create_directories(dir);
        fs::path file_path = dir / file_name;
        std::ofstream file(file_path, std::ios::binary);
        if (!file) {
            return {false, "Failed to open file for writing", {}};
        }
        file.write(reinterpret_cast<const char*>(compressed_data.data()), compressed_data.size());
        file.close();

        SafetensorsFileInfo info;
        info.name = file_name;
        info.md5 = ""; // TODO: 计算压缩后 MD5
        info.original_size = 0; // 写入时未知
        info.compressed_size = compressed_data.size();
        info.storage_key = file_path.string();
        return {true, "", info};
    } catch (const std::exception& e) {
        return {false, "Failed to write file: " + std::string(e.what()), {}};
    }
}

Result<std::vector<uint8_t>> LocalStorageEngine::readSafetensors(
    const std::string& model,
    const std::string& tag,
    const std::string& file_name) {

    try {
        fs::path file_path = getCheckpointDir(model, tag) / file_name;
        if (!fs::exists(file_path)) {
            return {false, "File does not exist", {}};
        }
        std::ifstream file(file_path, std::ios::binary | std::ios::ate);
        if (!file) {
            return {false, "Failed to open file for reading", {}};
        }
        std::streamsize size = file.tellg();
        file.seekg(0, std::ios::beg);
        std::vector<uint8_t> data(size);
        file.read(reinterpret_cast<char*>(data.data()), size);
        return {true, "", data};
    } catch (const std::exception& e) {
        return {false, "Failed to read file: " + std::string(e.what()), {}};
    }
}

Result<bool> LocalStorageEngine::deleteCheckpoint(const std::string& model, const std::string& tag) {
    try {
        fs::path dir = getCheckpointDir(model, tag);
        if (fs::exists(dir)) {
            fs::remove_all(dir);
        }
        return {true, "", true};
    } catch (const std::exception& e) {
        return {false, "Failed to delete checkpoint: " + std::string(e.what()), false};
    }
}

std::string LocalStorageEngine::getCheckpointDir(const std::string& model, const std::string& tag) const {
    return (root_path_ / model / tag).string();
}

} // namespace checkstore
#ifndef CHECKSTORE_STORAGE_ENGINE_H
#define CHECKSTORE_STORAGE_ENGINE_H

#include "Common.h"
#include "Metadata.h"
#include <vector>

namespace checkstore {

    class StorageEngine {
    public:
        virtual ~StorageEngine() = default;

        // 扫描目录，返回 safetensors 文件列表 (对应文档 4.4 节)
        virtual Result<std::vector<SafetensorsFileInfo>> scanSafetensors(const std::string& directory) = 0;

        // 写入压缩数据 (对应文档 4.4 节)
        virtual Result<SafetensorsFileInfo> writeSafetensors(
            const std::string& model,
            const std::string& tag,
            const std::string& file_name,
            const std::vector<uint8_t>& compressed_data) = 0;

        // 读取压缩数据
        virtual Result<std::vector<uint8_t>> readSafetensors(
            const std::string& model,
            const std::string& tag,
            const std::string& file_name) = 0;

        // 删除整个 checkpoint 的文件
        virtual Result<bool> deleteCheckpoint(const std::string& model, const std::string& tag) = 0;

        // 获取 checkpoint 的存储目录路径
        virtual std::string getCheckpointDir(const std::string& model, const std::string& tag) const = 0;
    };

    // 基于本地文件系统的简单实现 (原型用)
    class LocalStorageEngine : public StorageEngine {
    public:
        explicit LocalStorageEngine(const std::filesystem::path& root_path);
        // ... 实现上述虚函数
    private:
        std::filesystem::path root_path_;
    };

} // namespace checkstore
#endif
#ifndef CHECKSTORE_COMMON_H
#define CHECKSTORE_COMMON_H

#include <string>
#include <vector>
#include <cstdint>
#include <optional>
#include <unordered_map>

namespace checkstore {

    // 压缩策略枚举 (与文档 1.3 节对应)
    enum class CompressionStrategy {
        ZIPNN,      // 对应文档中的 `zipnn`
        XOR,        // 对应文档中的 `xor`
        OUR_METHOD  // 对应文档中的 `Our_Method` (待设计)
    };

    // Checkpoint 状态枚举 (与文档 3.1 节状态机对应)
    enum class CheckpointStatus {
        REGISTERED,
        COMPRESSING,
        STORED,
        AVAILABLE,
        DELETING,
        DELETED,
        FAILED
    };

    // Safetensors 文件信息 (对应文档 3.2 节元数据中的 `safetensors_files` 数组项)
    struct SafetensorsFileInfo {
        std::string name;             // 文件名，如 "model-00001-of-00030.safetensors"
        std::string md5;              // 该文件的 MD5 哈希值
        uint64_t original_size;       // 原始文件大小 (字节)
        uint64_t compressed_size;     // 压缩后文件大小 (字节)
        std::string storage_key;      // (可选) 在存储系统中的唯一标识或路径
    };

    // 操作结果通用结构
    template<typename T>
    struct Result {
        bool success = false;
        std::string error_message; // 失败时的错误信息
        T value;                    // 成功时的返回值
    };

    // 辅助函数：枚举转字符串
    std::string to_string(CompressionStrategy strategy);
    std::string to_string(CheckpointStatus status);
    CheckpointStatus from_string(const std::string& str);

    // JSON 序列化支持 (需要 nlohmann/json)
    #include <nlohmann/json.hpp>
    void to_json(nlohmann::json& j, const SafetensorsFileInfo& info);
    void from_json(const nlohmann::json& j, SafetensorsFileInfo& info);

} // namespace checkstore
#endif
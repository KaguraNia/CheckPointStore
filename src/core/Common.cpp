#include "Common.h"
#include <string>
#include <nlohmann/json.hpp>

namespace checkstore {

// 实现 CompressionStrategy 的字符串转换
std::string to_string(CompressionStrategy strategy) {
    switch (strategy) {
        case CompressionStrategy::ZIPNN:
            return "zipnn";
        case CompressionStrategy::XOR:
            return "xor";
        case CompressionStrategy::OUR_METHOD:
            return "Our_Method";
        default:
            return "unknown";
    }
}

// 实现 CheckpointStatus 的字符串转换
std::string to_string(CheckpointStatus status) {
    switch (status) {
        case CheckpointStatus::REGISTERED:
            return "REGISTERED";
        case CheckpointStatus::COMPRESSING:
            return "COMPRESSING";
        case CheckpointStatus::STORED:
            return "STORED";
        case CheckpointStatus::AVAILABLE:
            return "AVAILABLE";
        case CheckpointStatus::DELETING:
            return "DELETING";
        case CheckpointStatus::DELETED:
            return "DELETED";
        case CheckpointStatus::FAILED:
            return "FAILED";
        default:
            return "UNKNOWN";
    }
}

CheckpointStatus from_string(const std::string& str) {
    if (str == "REGISTERED") return CheckpointStatus::REGISTERED;
    if (str == "COMPRESSING") return CheckpointStatus::COMPRESSING;
    if (str == "STORED") return CheckpointStatus::STORED;
    if (str == "AVAILABLE") return CheckpointStatus::AVAILABLE;
    if (str == "DELETING") return CheckpointStatus::DELETING;
    if (str == "DELETED") return CheckpointStatus::DELETED;
    if (str == "FAILED") return CheckpointStatus::FAILED;
    return CheckpointStatus::FAILED; // default
}

// 实现 SafetensorsFileInfo 的 JSON 序列化
void to_json(nlohmann::json& j, const SafetensorsFileInfo& info) {
    j = nlohmann::json{
        {"name", info.name},
        {"md5", info.md5},
        {"original_size", info.original_size},
        {"compressed_size", info.compressed_size},
        {"storage_key", info.storage_key}
    };
}

void from_json(const nlohmann::json& j, SafetensorsFileInfo& info) {
    j.at("name").get_to(info.name);
    j.at("md5").get_to(info.md5);
    j.at("original_size").get_to(info.original_size);
    j.at("compressed_size").get_to(info.compressed_size);
    j.at("storage_key").get_to(info.storage_key);
}

} // namespace checkstore
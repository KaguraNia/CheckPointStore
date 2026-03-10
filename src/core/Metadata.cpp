#include "Metadata.h"
#include "Common.h"
#include <nlohmann/json.hpp>

namespace checkstore {

// 实现 CheckpointMetadata 的 JSON 序列化
void to_json(nlohmann::json& j, const CheckpointMetadata& meta) {
    j = nlohmann::json{
        {"model", meta.model},
        {"tag", meta.tag},
        {"version", meta.version},
        {"status", to_string(meta.status)},
        {"compression_strategy", to_string(meta.compression_strategy)},
        {"base_tag", meta.base_tag},
        {"original_size", meta.original_size},
        {"compressed_size", meta.compressed_size},
        {"md5_original", meta.md5_original},
        {"created_at", std::chrono::system_clock::to_time_t(meta.created_at)},
        {"updated_at", std::chrono::system_clock::to_time_t(meta.updated_at)},
        {"file_path", meta.file_path},
        {"safetensors_files", meta.safetensors_files},
        {"dependencies", meta.dependencies}
    };
}

void from_json(const nlohmann::json& j, CheckpointMetadata& meta) {
    j.at("model").get_to(meta.model);
    j.at("tag").get_to(meta.tag);
    j.at("version").get_to(meta.version);

    std::string status_str = j.at("status").get<std::string>();
    meta.status = from_string(status_str);

    std::string strat_str = j.at("compression_strategy").get<std::string>();
    if (strat_str == "zipnn") meta.compression_strategy = CompressionStrategy::ZIPNN;
    else if (strat_str == "xor") meta.compression_strategy = CompressionStrategy::XOR;
    else meta.compression_strategy = CompressionStrategy::OUR_METHOD;

    if (j.contains("base_tag") && !j["base_tag"].is_null()) {
        meta.base_tag = j["base_tag"].get<std::string>();
    } else {
        meta.base_tag = std::nullopt;
    }

    j.at("original_size").get_to(meta.original_size);
    j.at("compressed_size").get_to(meta.compressed_size);
    j.at("md5_original").get_to(meta.md5_original);

    time_t created_t = j.at("created_at").get<time_t>();
    meta.created_at = std::chrono::system_clock::from_time_t(created_t);

    time_t updated_t = j.at("updated_at").get<time_t>();
    meta.updated_at = std::chrono::system_clock::from_time_t(updated_t);

    j.at("file_path").get_to(meta.file_path);
    j.at("safetensors_files").get_to(meta.safetensors_files);
    j.at("dependencies").get_to(meta.dependencies);
}

} // namespace checkstore
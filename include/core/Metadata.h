#ifndef CHECKSTORE_METADATA_H
#define CHECKSTORE_METADATA_H

#include "Common.h"
#include <chrono>
#include <vector>

namespace checkstore {

    // Checkpoint 元数据 (对应文档 3.2 节 JSON 结构)
    struct CheckpointMetadata {
        std::string model;                     // 模型名，如 "llama-7b"
        std::string tag;                       // Checkpoint 标签，如 "epoch-10"
        int version = 1;                       // 元数据版本
        CheckpointStatus status;               // 生命周期状态
        CompressionStrategy compression_strategy; // 使用的压缩方法
        std::optional<std::string> base_tag;   // 基准 checkpoint 标签 (XOR 模式用)
        uint64_t original_size;                // 原始总大小
        uint64_t compressed_size;              // 压缩后总大小
        std::string md5_original;              // 原始文件的聚合 MD5 (文档中提及)
        std::chrono::system_clock::time_point created_at; // 创建时间
        std::chrono::system_clock::time_point updated_at; // 更新时间
        std::string file_path;                 // 存储路径
        std::vector<SafetensorsFileInfo> safetensors_files; // 包含的文件列表
        std::vector<std::string> dependencies; // 依赖的 checkpoint 列表 (XOR 模式用)
    };

} // namespace checkstore
#endif
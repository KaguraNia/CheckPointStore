#ifndef CHECKSTORE_COMPRESSION_SCHEDULER_H
#define CHECKSTORE_COMPRESSION_SCHEDULER_H

#include "Common.h"
#include "Metadata.h"
#include <vector>

namespace checkstore {

    // 压缩任务结构
    struct CompressionTask {
        std::string model;
        std::string tag;
        std::vector<SafetensorsFileInfo> files;
        CompressionStrategy strategy;
        std::optional<std::string> base_tag;
    };

    // 压缩结果结构
    struct CompressResults {
        bool success = false;
        std::string error_message;
        std::vector<SafetensorsFileInfo> compressed_files;
    };

    // 解压任务结构
    struct DecompressionTask {
        std::string model;
        std::string tag;
        std::vector<SafetensorsFileInfo> files;
        CompressionStrategy strategy;
        std::optional<std::string> base_tag;
    };

    // 解压结果结构
    struct DecompressResults {
        bool success = false;
        std::string error_message;
        std::vector<std::pair<std::string, std::vector<uint8_t>>> decompressed_data; // 文件名 -> 数据
    };

    // 压缩调度器 (对应文档 4.5 节)
    class CompressionScheduler {
    public:
        virtual ~CompressionScheduler() = default;

        // 调度压缩任务
        virtual Result<CompressResults> scheduleCompression(const CompressionTask& task) = 0;

        // 调度解压任务
        virtual Result<DecompressResults> scheduleDecompression(const DecompressionTask& task) = 0;
    };

    // 简单实现 (原型用，模拟调用 Compression Engine)
    class SimpleCompressionScheduler : public CompressionScheduler {
    public:
        // ... 实现上述虚函数
    };

} // namespace checkstore
#endif

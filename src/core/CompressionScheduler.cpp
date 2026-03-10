#include "CompressionScheduler.h"
#include <spdlog/spdlog.h>

namespace checkstore {

Result<CompressResults> SimpleCompressionScheduler::scheduleCompression(const CompressionTask& task) {
    SPDLOG_INFO("Scheduling compression for model: {}, tag: {}", task.model, task.tag);

    CompressResults results;
    results.success = true;
    results.compressed_files = task.files;

    // 模拟压缩：简单地将 compressed_size 设为 original_size / 2
    for (auto& file : results.compressed_files) {
        file.compressed_size = file.original_size / 2;
        file.storage_key = "/simulated/path/" + task.model + "/" + task.tag + "/" + file.name;
    }

    SPDLOG_INFO("Compression completed for model: {}, tag: {}", task.model, task.tag);
    return {true, "", results};
}

Result<DecompressResults> SimpleCompressionScheduler::scheduleDecompression(const DecompressionTask& task) {
    SPDLOG_INFO("Scheduling decompression for model: {}, tag: {}", task.model, task.tag);

    DecompressResults results;
    results.success = true;

    // 模拟解压：返回模拟数据
    for (const auto& file : task.files) {
        std::vector<uint8_t> data(file.original_size, 0xAA); // 模拟数据
        results.decompressed_data.emplace_back(file.name, data);
    }

    SPDLOG_INFO("Decompression completed for model: {}, tag: {}", task.model, task.tag);
    return {true, "", results};
}

} // namespace checkstore

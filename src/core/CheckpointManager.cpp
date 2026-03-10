#include "CheckpointManager.h"
#include "StorageEngine.h"
#include "DependencyGraph.h"
#include "MetadataService.h"
#include "CompressionScheduler.h"
#include "IntegrityVerifier.h"
#include "GlobalConfigLoader.h"
#include <spdlog/spdlog.h> // 示例使用 spdlog 进行日志记录

namespace checkstore {

    Result<CheckpointMetadata> CheckpointManagerImpl::registerCheckpoint(
        const std::string& model,
        const std::string& tag,
        const std::string& source_path,
        const std::optional<std::string>& base_tag) {

        SPDLOG_INFO("Registering checkpoint. Model: {}, Tag: {}, Path: {}", model, tag, source_path);

        // 1. 合法性校验 (对应文档 4.2 节)
        auto existing_info = metadata_service_->getCheckpointInfo(model, tag);
        if (existing_info.success) {
            return Result<CheckpointMetadata>{false, "Checkpoint with the same model and tag already exists.", {}};
        }

        // 2. XOR 模式依赖校验
        auto config = GlobalConfigLoader::getInstance().getConfig();
        if (config.compression_strategy == CompressionStrategy::XOR) {
            if (!base_tag.has_value()) {
                return Result<CheckpointMetadata>{false, "XOR compression requires a base checkpoint tag.", {}};
            }
            // 检查 base checkpoint 是否存在且可用
            auto base_info = metadata_service_->getCheckpointInfo(model, base_tag.value());
            if (!base_info.success) {
                return Result<CheckpointMetadata>{false, "Base checkpoint not found.", {}};
            }
        }

        // 3. 扫描源目录文件
        auto scan_result = storage_engine_->scanSafetensors(source_path);
        if (!scan_result.success) {
            return Result<CheckpointMetadata>{false, "Failed to scan source directory: " + scan_result.error_message, {}};
        }
        auto& file_infos = scan_result.value;

        // 4. 计算原始总大小和 MD5 (简化)
        uint64_t total_original_size = 0;
        for (const auto& info : file_infos) {
            total_original_size += info.original_size;
            // TODO: 调用 IntegrityVerifier 计算每个文件的 MD5
        }
        // TODO: 计算聚合 MD5 (md5_original)

        // 5. 创建初始元数据，状态为 REGISTERED
        CheckpointMetadata metadata;
        metadata.model = model;
        metadata.tag = tag;
        metadata.status = CheckpointStatus::REGISTERED;
        metadata.compression_strategy = config.compression_strategy;
        metadata.base_tag = base_tag;
        metadata.original_size = total_original_size;
        metadata.compressed_size = 0; // 压缩后填充
        metadata.created_at = std::chrono::system_clock::now();
        metadata.updated_at = metadata.created_at;
        metadata.file_path = storage_engine_->getCheckpointDir(model, tag);
        metadata.safetensors_files = file_infos;
        if (base_tag) {
            metadata.dependencies.push_back(base_tag.value());
        }

        // 6. 持久化元数据 (注册到 Metadata Service)
        auto persist_result = metadata_service_->persistMetadata(metadata);
        if (!persist_result.success) {
            return Result<CheckpointMetadata>{false, "Failed to persist metadata: " + persist_result.error_message, {}};
        }

        // 7. 更新依赖图
        if (base_tag) {
            auto dep_result = dependency_graph_->addNode(model, tag, base_tag);
            if (!dep_result.success) {
                // 回滚：删除已持久化的元数据？(简化处理，记录错误)
                SPDLOG_ERROR("Failed to add dependency, but metadata is persisted. Model: {}, Tag: {}, Base: {}",
                           model, tag, base_tag.value());
            }
        }

        // 8. 提交压缩任务到调度器 (异步)
        // 此处是核心扩展点，任务提交后立即返回，实际压缩由后台进行
        auto task_submit_result = compression_scheduler_->submitCompressionTask(model, tag, source_path, file_infos);
        if (!task_submit_result.success) {
            metadata.status = CheckpointStatus::FAILED;
            metadata_service_->updateMetadata(metadata); // 更新状态为失败
            return Result<CheckpointMetadata>{false, "Failed to submit compression task.", metadata};
        }

        SPDLOG_INFO("Checkpoint registered successfully. Compression task submitted.");
        return Result<CheckpointMetadata>{true, "", metadata};
    }

} // namespace checkstore
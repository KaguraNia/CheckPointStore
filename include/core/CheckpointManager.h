#ifndef CHECKSTORE_CHECKPOINT_MANAGER_H
#define CHECKSTORE_CHECKPOINT_MANAGER_H

#include "Common.h"
#include "Metadata.h"
#include <memory>

namespace checkstore {

    // 对应文档 4.2 节 Checkpoint Manager 的核心接口
    class CheckpointManager {
    public:
        virtual ~CheckpointManager() = default;

        // 注册新 checkpoint (对应文档 4.2 节接口)
        virtual Result<CheckpointMetadata> registerCheckpoint(
            const std::string& model,
            const std::string& tag,
            const std::string& source_path,
            const std::optional<std::string>& base_tag = std::nullopt) = 0;

        // 恢复 checkpoint
        virtual Result<bool> restoreCheckpoint(
            const std::string& model,
            const std::string& tag,
            const std::string& output_path) = 0;

        // 删除 checkpoint
        virtual Result<bool> deleteCheckpoint(const std::string& model, const std::string& tag) = 0;

        // 查询 checkpoint 列表
        virtual Result<std::vector<CheckpointMetadata>> listCheckpoints(
            const std::optional<std::string>& model = std::nullopt) = 0;

        // 获取单个 checkpoint 详情
        virtual Result<CheckpointMetadata> getCheckpointInfo(
            const std::string& model, const std::string& tag) = 0;

        // 更新状态 (内部使用)
        virtual Result<bool> updateStatus(
            const std::string& model,
            const std::string& tag,
            CheckpointStatus new_status) = 0;
    };

    // 管理器的核心实现类
    class CheckpointManagerImpl : public CheckpointManager {
    public:
        // 构造函数，依赖注入各个组件
        CheckpointManagerImpl(
            std::shared_ptr<StorageEngine> storage,
            std::shared_ptr<DependencyGraph> dep_graph,
            // ... 其他组件依赖
        );
        // ... 实现上述接口，并包含文档 4.2 节提到的“操作合法性校验”逻辑
    };

} // namespace checkstore
#endif
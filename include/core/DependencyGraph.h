#ifndef CHECKSTORE_DEPENDENCY_GRAPH_H
#define CHECKSTORE_DEPENDENCY_GRAPH_H

#include "Common.h"
#include "Metadata.h"
#include <memory>
#include <unordered_map>

namespace checkstore {

    // 依赖关系图引擎 (对应文档 4.3 节)
    class DependencyGraph {
    public:
        virtual ~DependencyGraph() = default;

        // 添加节点 (checkpoint)
        virtual Result<bool> addNode(
            const std::string& model,
            const std::string& tag,
            const std::optional<std::string>& base_tag) = 0;

        // 移除节点
        virtual Result<bool> removeNode(const std::string& model, const std::string& tag) = 0;

        // 获取依赖于指定 checkpoint 的所有 checkpoint
        virtual Result<std::vector<std::string>> getDependents(
            const std::string& model, const std::string& tag) = 0;

        // 获取指定 checkpoint 的基准 (base) checkpoint
        virtual Result<std::optional<std::string>> getBase(
            const std::string& model, const std::string& tag) = 0;

        // 校验依赖约束 (例如：base 不能依赖其他)
        virtual Result<bool> validateDependency(
            const std::string& model,
            const std::string& tag,
            const std::optional<std::string>& intended_base) = 0;
    };

    // 基于内存的简单实现 (原型用)
    class InMemoryDependencyGraph : public DependencyGraph {
    private:
        // 使用 model -> (tag -> metadata) 的两层映射存储
        std::unordered_map<std::string,
            std::unordered_map<std::string, CheckpointMetadata>> nodes_;
        // 依赖关系: model -> (child_tag -> base_tag)
        std::unordered_map<std::string,
            std::unordered_map<std::string, std::string>> dependencies_;
        // ... 实现上述虚函数
    };

} // namespace checkstore
#endif
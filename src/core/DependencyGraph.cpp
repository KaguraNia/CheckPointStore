#include "DependencyGraph.h"
#include "Metadata.h"
#include <algorithm>

namespace checkstore {

Result<bool> InMemoryDependencyGraph::addNode(
    const std::string& model,
    const std::string& tag,
    const std::optional<std::string>& base_tag) {

    // 检查是否已存在
    if (nodes_[model].find(tag) != nodes_[model].end()) {
        return {false, "Node already exists", false};
    }

    // 如果是base checkpoint (base_tag 为空)
    if (!base_tag.has_value()) {
        // 检查该模型是否已有base checkpoint
        for (const auto& [t, meta] : nodes_[model]) {
            if (!meta.base_tag.has_value()) {
                return {false, "Model already has a base checkpoint", false};
            }
        }
        // 添加base节点
        CheckpointMetadata meta;
        meta.model = model;
        meta.tag = tag;
        meta.base_tag = std::nullopt;
        meta.status = CheckpointStatus::REGISTERED; // 默认状态
        nodes_[model][tag] = meta;
        // dependencies_ 不设置或设为空
    } else {
        // 非base，检查base是否存在且是base
        auto base_it = nodes_[model].find(base_tag.value());
        if (base_it == nodes_[model].end()) {
            return {false, "Base checkpoint does not exist", false};
        }
        if (base_it->second.base_tag.has_value()) {
            return {false, "Specified base is not a base checkpoint", false};
        }
        // 添加非base节点
        CheckpointMetadata meta;
        meta.model = model;
        meta.tag = tag;
        meta.base_tag = base_tag;
        meta.status = CheckpointStatus::REGISTERED;
        nodes_[model][tag] = meta;
        dependencies_[model][tag] = base_tag.value();
    }

    return {true, "", true};
}

Result<bool> InMemoryDependencyGraph::removeNode(const std::string& model, const std::string& tag) {
    // 检查是否存在
    if (nodes_[model].find(tag) == nodes_[model].end()) {
        return {false, "Node does not exist", false};
    }

    // 检查是否有依赖它的节点
    for (const auto& [child, base] : dependencies_[model]) {
        if (base == tag) {
            return {false, "Cannot remove node that has dependents", false};
        }
    }

    // 移除
    nodes_[model].erase(tag);
    dependencies_[model].erase(tag);

    return {true, "", true};
}

Result<std::vector<std::string>> InMemoryDependencyGraph::getDependents(
    const std::string& model, const std::string& tag) {

    std::vector<std::string> dependents;
    for (const auto& [child, base] : dependencies_[model]) {
        if (base == tag) {
            dependents.push_back(child);
        }
    }
    return {true, "", dependents};
}

Result<std::optional<std::string>> InMemoryDependencyGraph::getBase(
    const std::string& model, const std::string& tag) {

    auto dep_it = dependencies_[model].find(tag);
    if (dep_it != dependencies_[model].end()) {
        return {true, "", dep_it->second};
    }
    return {true, "", std::nullopt};
}

Result<bool> InMemoryDependencyGraph::validateDependency(
    const std::string& model,
    const std::string& tag,
    const std::optional<std::string>& intended_base) {

    if (!intended_base.has_value()) {
        // 检查是否已有base
        for (const auto& [t, meta] : nodes_[model]) {
            if (!meta.base_tag.has_value()) {
                return {false, "Model already has a base checkpoint", false};
            }
        }
        return {true, "", true};
    } else {
        // 检查base是否存在且是base
        auto base_it = nodes_[model].find(intended_base.value());
        if (base_it == nodes_[model].end()) {
            return {false, "Intended base does not exist", false};
        }
        if (base_it->second.base_tag.has_value()) {
            return {false, "Intended base is not a base checkpoint", false};
        }
        return {true, "", true};
    }
}

} // namespace checkstore
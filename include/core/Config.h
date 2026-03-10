#ifndef CHECKSTORE_CONFIG_H
#define CHECKSTORE_CONFIG_H

#include "Common.h"
#include <string>
#include <filesystem>

namespace checkstore {

    // 系统配置结构 (对应文档 4.1 节)
    struct Config {
        CompressionStrategy compression_strategy = CompressionStrategy::ZIPNN;
        std::filesystem::path root_path;        // 存储根路径
        bool verify_md5_on_save = true;         // 保存时校验 MD5
        bool verify_md5_on_restore = true;       // 恢复时校验 MD5
        int max_compression_retries = 3;        // 压缩最大重试次数
        // ... 可扩展其他配置项
    };

    class GlobalConfigLoader {
    public:
        // 单例模式获取配置实例
        static GlobalConfigLoader& getInstance();

        // 从指定路径加载并校验配置文件
        Result<Config> loadConfig(const std::string& config_path);

        // 获取当前配置
        const Config& getConfig() const;

    private:
        GlobalConfigLoader() = default;
        Config current_config_;
        bool is_loaded_ = false;
    };

} // namespace checkstore
#endif
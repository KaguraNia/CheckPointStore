#include "Config.h"
#include <yaml-cpp/yaml.h>
#include <filesystem>
#include <stdexcept>

namespace checkstore {

GlobalConfigLoader& GlobalConfigLoader::getInstance() {
    static GlobalConfigLoader instance;
    return instance;
}

Result<Config> GlobalConfigLoader::loadConfig(const std::string& config_path) {
    try {
        YAML::Node config = YAML::LoadFile(config_path);
        Config cfg;

        // 解析 compression_strategy
        if (config["compression_strategy"]) {
            std::string strat = config["compression_strategy"].as<std::string>();
            if (strat == "zipnn") {
                cfg.compression_strategy = CompressionStrategy::ZIPNN;
            } else if (strat == "xor") {
                cfg.compression_strategy = CompressionStrategy::XOR;
            } else if (strat == "Our_Method") {
                cfg.compression_strategy = CompressionStrategy::OUR_METHOD;
            } else {
                return {false, "Invalid compression_strategy: " + strat, {}};
            }
        }

        // 解析 root_path
        if (config["root_path"]) {
            cfg.root_path = config["root_path"].as<std::string>();
        } else {
            return {false, "root_path is required", {}};
        }

        // 解析 verify_md5_on_save
        if (config["verify_md5_on_save"]) {
            cfg.verify_md5_on_save = config["verify_md5_on_save"].as<bool>();
        }

        // 解析 verify_md5_on_restore
        if (config["verify_md5_on_restore"]) {
            cfg.verify_md5_on_restore = config["verify_md5_on_restore"].as<bool>();
        }

        // 解析 max_compression_retries
        if (config["max_compression_retries"]) {
            cfg.max_compression_retries = config["max_compression_retries"].as<int>();
        }

        current_config_ = cfg;
        is_loaded_ = true;
        return {true, "", cfg};
    } catch (const YAML::Exception& e) {
        return {false, "YAML parsing error: " + std::string(e.what()), {}};
    } catch (const std::exception& e) {
        return {false, "Error loading config: " + std::string(e.what()), {}};
    }
}

const Config& GlobalConfigLoader::getConfig() const {
    if (!is_loaded_) {
        throw std::runtime_error("Config not loaded");
    }
    return current_config_;
}

} // namespace checkstore
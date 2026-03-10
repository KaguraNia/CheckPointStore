#include "core/MainDaemon.h"
#include "core/GlobalConfigLoader.h"
#include <iostream>

int main(int argc, char* argv[]) {
    // 1. 加载配置
    auto& config_loader = checkstore::GlobalConfigLoader::getInstance();
    auto config_result = config_loader.loadConfig("config.yaml");
    if (!config_result.success) {
        std::cerr << "Failed to load config: " << config_result.error_message << std::endl;
        return 1;
    }

    // 2. 初始化并运行守护进程
    checkstore::MainDaemon daemon;
    if (!daemon.initialize()) {
        std::cerr << "Failed to initialize daemon." << std::endl;
        return 1;
    }

    std::cout << "CheckStore Daemon starting..." << std::endl;
    daemon.run(); // 进入事件循环，例如监听 Unix Socket 或 HTTP
    std::cout << "CheckStore Daemon stopped." << std::endl;

    return 0;
}
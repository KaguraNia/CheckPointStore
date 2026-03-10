#include "cli/CLI.h"
#include <cxxopts.hpp> // 推荐使用 cxxopts 库解析命令行参数
#include <iostream>

namespace checkstore::cli {

    int handleSaveCommand(const cxxopts::ParseResult& result) {
        // 解析 --model, --tag, --path 等参数
        // 构造请求，通过 Unix Socket/HTTP 发送给 Daemon
        // 接收并打印结果
        std::cout << "Save command received. (Prototype placeholder)" << std::endl;
        return 0;
    }

    int handleRestoreCommand(const cxxopts::ParseResult& result) { /* ... */ }
    int handleListCommand(const cxxopts::ParseResult& result) { /* ... */ }
    // ... 其他命令处理函数

    int runCLI(int argc, char* argv[]) {
        cxxopts::Options options("checkstore", "CheckStore - Checkpoint Storage and Recovery System");

        // 定义子命令和参数 (对应文档第5章 CLI设计)
        options.add_options()
            ("command", "Subcommand to execute", cxxopts::value<std::string>())
            ("model", "Model name", cxxopts::value<std::string>())
            ("tag", "Checkpoint tag", cxxopts::value<std::string>())
            ("path", "Path to checkpoint files", cxxopts::value<std::string>())
            ("output", "Output path for restore", cxxopts::value<std::string>())
            ("h,help", "Print usage");

        options.parse_positional({"command"});

        try {
            auto result = options.parse(argc, argv);

            if (result.count("help") || result["command"].count() == 0) {
                std::cout << options.help() << std::endl;
                return 0;
            }

            std::string command = result["command"].as<std::string>();
            if (command == "save") {
                return handleSaveCommand(result);
            } else if (command == "restore") {
                return handleRestoreCommand(result);
            } else if (command == "list") {
                return handleListCommand(result);
            } else if (command == "info") {
                return handleInfoCommand(result);
            } else if (command == "delete") {
                return handleDeleteCommand(result);
            } else if (command == "gc") {
                return handleGCCommand(result);
            } else if (command == "daemon") {
                return handleDaemonCommand(result);
            } else {
                std::cerr << "Unknown command: " << command << std::endl;
                return 1;
            }
        } catch (const cxxopts::exceptions::exception& e) {
            std::cerr << "Error parsing options: " << e.what() << std::endl;
            return 1;
        }
    }
}
#include "MainDaemon.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace checkstore {

MainDaemon::MainDaemon() : running_(false) {}

MainDaemon::~MainDaemon() {
    stop();
}

bool MainDaemon::initialize() {
    // 初始化组件，如创建CheckpointManager等
    // 由于组件未完全实现，暂时返回true
    std::cout << "MainDaemon initialized." << std::endl;
    return true;
}

void MainDaemon::run() {
    running_ = true;
    std::cout << "MainDaemon running..." << std::endl;
    while (running_) {
        // 模拟事件循环，检查任务等
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

void MainDaemon::stop() {
    running_ = false;
    std::cout << "MainDaemon stopped." << std::endl;
}

} // namespace checkstore
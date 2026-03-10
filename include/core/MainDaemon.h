#ifndef CHECKSTORE_MAIN_DAEMON_H
#define CHECKSTORE_MAIN_DAEMON_H

#include <memory>

namespace checkstore {

    class MainDaemon {
    public:
        MainDaemon();
        ~MainDaemon();

        // 初始化守护进程
        bool initialize();

        // 运行守护进程（事件循环）
        void run();

        // 停止守护进程
        void stop();

    private:
        bool running_;
        // 组件指针，如 std::unique_ptr<CheckpointManager> checkpoint_manager_;
        // 但由于未实现，暂时空
    };

} // namespace checkstore
#endif
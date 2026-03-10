# # 编译与运行
# mkdir build && cd build
# cmake ..
# make -j4

# # 运行守护进程 (后台)
# ./checkstored

# # 运行 CLI 客户端 (示例)
# ./checkstore save --model llama-7b --tag epoch-10 --path ./my_checkpoint/
# ./checkstore list

cmake_minimum_required(VERSION 3.20)
project(checkstore_prototype VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖 (示例)
find_package(spdlog CONFIG REQUIRED) # 用于日志
find_package(cxxopts CONFIG REQUIRED) # 用于 CLI 参数解析
# find_package(SQLite3 REQUIRED) # 未来用于 Metadata Service

# 包含目录
include_directories(${PROJECT_SOURCE_DIR}/include)

# 核心库
add_library(checkstore_core STATIC
    src/core/Config.cpp
    src/core/Metadata.cpp
    src/core/CheckpointManager.cpp
    src/core/StorageEngine.cpp
    src/core/DependencyGraph.cpp
    src/core/CompressionScheduler.cpp
    src/core/IntegrityVerifier.cpp
    src/core/GarbageCollector.cpp
    src/core/MetadataService.cpp
    src/core/MainDaemon.cpp
)
target_link_libraries(checkstore_core PRIVATE spdlog::spdlog) # 链接日志库

# 守护进程可执行文件
add_executable(checkstored src/main.cpp)
target_link_libraries(checkstored PRIVATE checkstore_core)

# CLI 客户端可执行文件
add_executable(checkstore src/cli/CLI.cpp)
target_link_libraries(checkstore PRIVATE checkstore_core cxxopts::cxxopts)
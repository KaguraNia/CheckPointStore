# CheckPointStore
# CheckStore
高性能的Checkpoint存储与恢复系统，支持GB级别checkpoint文件的高效压缩存储、按需恢复与统一生命周期管理，提供便捷的CLI操作工具。

## 项目简介
CheckStore 核心解决大模型Checkpoint文件存储占用大、恢复效率低、管理混乱的问题，通过**ZipNN**/**XOR**等压缩策略实现高效存储，基于MD5校验保障数据完整性，提供从注册、压缩、存储到恢复、删除、垃圾回收的全生命周期管理能力，同时支持通过CLI工具完成所有操作，适配大模型训练/推理场景的Checkpoint管理需求。

### 核心目标
1. **高效存储与压缩**：支持GB级别Checkpoint文件的多策略压缩，降低存储占用
2. **按需恢复**：根据模型名和标签精准恢复Checkpoint，严格保证数据一致性
3. **统一管理**：提供CLI工具实现Checkpoint的注册、恢复、查询、删除等全操作
4. **自动化生命周期**：支持Checkpoint状态管理、依赖校验、自动垃圾回收

### 核心特性
- 多压缩策略支持：ZipNN（单文件压缩）、XOR（差异压缩，基于基准Checkpoint存储差异）
- 数据完整性保障：全程MD5校验，保存/恢复阶段均验证数据一致性
- 依赖关系管理：维护Checkpoint依赖图，支持XOR模式的基准依赖约束
- 全生命周期状态机：精细化管理Checkpoint从注册到删除的所有状态
- 安全的删除机制：软删除+垃圾回收分离，支持误删恢复
- 便捷CLI操作：提供简洁的命令行工具，覆盖所有核心功能
- 元数据持久化：基于SQLite存储元数据，支持索引与事务，保证数据可靠性

## 系统架构
### 整体架构
CheckStore采用**分层组件化架构**，各组件职责解耦，通过标准化接口交互，核心分为五层：
1. **CLI Client**：用户交互入口，通过Unix Socket/HTTP与守护进程通信
2. **CheckStore Daemon**：核心服务进程，整合所有业务逻辑组件
3. **核心业务组件**：包含Checkpoint Manager、Dependency Graph Engine等核心模块
4. **基础服务组件**：包含Compression Engine、Storage Engine、Metadata Service等基础模块
5. **Local Storage**：持久化存储层，存放压缩后的Checkpoint文件与元数据

### 核心组件
| 组件 | 核心职责 |
|------|----------|
| Global Config Loader | 解析/校验配置文件，动态分发配置参数到各组件 |
| Checkpoint Manager | 核心管控，管理Checkpoint生命周期，提供统一API，校验操作合法性 |
| Dependency Graph Engine | 维护Checkpoint依赖关系，辅助垃圾回收与恢复 |
| Storage Engine | 负责文件的扫描、读写、删除，管理存储目录结构（不参与压缩逻辑） |
| Compression Scheduler | 调度压缩/解压任务，管理任务队列，控制并发度 |
| Compression Engine | 独立Python进程，执行实际的ZipNN/XOR压缩与解压操作 |
| Metadata Service | 基于SQLite持久化元数据，提供查询/索引/事务能力 |
| Integrity Verifier | 计算并校验MD5码，验证文件完整性 |
| Garbage Collector | 识别可回收Checkpoint，清理无效文件，释放存储空间 |

## 核心概念
### Checkpoint生命周期状态机
Checkpoint从注册到删除的全生命周期包含7个状态，状态流转严格可控，失败操作支持重试/人工处理：
| 状态 | 描述 | 失败处理 |
|------|------|----------|
| REGISTERED | 元数据已持久化，压缩任务等待调度 | - |
| COMPRESSING | 压缩任务正在执行中 | 可重试（最多3次） |
| STORED | 压缩完成，文件已写入存储层 | 可重试 |
| AVAILABLE | Checkpoint完全可用，可执行恢复操作 | - |
| DELETING | 删除操作进行中 | 标记为删除失败，需人工处理 |
| DELETED | 已软删除（终态，等待GC清理） | - |
| FAILED | 操作失败，需根据具体情况处理 | - |

### 模型-Checkpoint关系约束
- 每个模型**仅能有一个base Checkpoint**（基准Checkpoint）
- 非base Checkpoint必须依赖同一模型的其他Checkpoint
- XOR压缩的差异基于base或中间版本Checkpoint计算
- 禁止删除被其他Checkpoint依赖的base Checkpoint

### 压缩策略
| 策略 | 标识符 | 描述 |
|------|--------|------|
| ZipNN | zipnn | 对Checkpoint中的每个safetensors文件单独进行ZipNN压缩 |
| XOR | xor | 第一个base Checkpoint用ZipNN压缩，后续Checkpoint基于base执行XOR异或，仅存储差异部分 |
| Our_Method | - | 待设计扩展 |

## 数据模型
### 核心元数据结构
Checkpoint元数据采用JSON格式存储，包含核心信息、文件信息与依赖信息，示例如下：
```json
{
  "model": "llama-7b",
  "tag": "epoch-10",
  "version": 1,
  "status": "AVAILABLE",
  "compression_strategy": "xor",
  "base_tag": "epoch-0",
  "original_size": 15420000000,
  "compressed_size": 8200000000,
  "md5_original": "a1b2c3d4e5f6...",
  "created_at": "2024-01-15T10:30:00Z",
  "updated_at": "2024-01-15T10:35:00Z",
  "file_path": "/data/checkstore/llama-7b/ckpt_epoch10/",
  "safetensors_files": [
    {
      "name": "model-00001-of-00030.safetensors",
      "md5": "xxx...",
      "original_size": 500000000,
      "compressed_size": 250000000
    }
  ],
  "dependencies": ["epoch-0"]
}
```

### 数据库表设计
基于SQLite实现元数据持久化，包含3张核心表，通过外键/索引保证数据一致性与查询效率：
1. **checkpoints（主表）**：存储Checkpoint核心元数据，`model+tag`为联合唯一索引
2. **checkpoint_files（子表）**：存储Checkpoint下各safetensors文件的详细信息，外键关联主表，支持级联删除
3. **checkpoint_dependencies（关系表）**：存储Checkpoint之间的依赖关系，外键约束保证不允许删除被依赖的Checkpoint

## 快速开始
### 核心CLI命令
CheckStore提供简洁的CLI工具，所有操作通过`checkstore`命令完成，核心命令如下：

| 命令 | 描述 | 基础用法 |
|------|------|----------|
| save | 注册并保存Checkpoint（自动压缩） | `checkstore save --model <模型名> --tag <标签> --path <本地路径>` |
| restore | 恢复Checkpoint到指定路径（自动解压+MD5校验） | `checkstore restore --model <模型名> --tag <标签> --output <输出路径>` |
| list | 列出系统中所有/指定模型的Checkpoint信息 | `checkstore list [--model <模型名>]` |
| info | 查看指定Checkpoint的详细元数据 | `checkstore info --model <模型名> --tag <标签>` |
| delete | 软删除Checkpoint（标记为DELETED，等待GC） | `checkstore delete --model <模型名> --tag <标签>` |
| daemon | 管理CheckStore守护进程 | `checkstore daemon [status/start/stop/restart]` |
| gc | 手动触发垃圾回收，清理已标记为DELETED的Checkpoint | `checkstore gc` |
| config | 管理系统配置 | `checkstore config [show/set] <配置项> <值>` |

### 典型操作示例
#### 1. 保存Checkpoint
将本地`llama-7b`的`epoch-10` Checkpoint注册并保存到系统，自动采用配置的压缩策略：
```bash
checkstore save --model llama-7b --tag epoch-10 --path ./epoch10/
```

#### 2. 恢复Checkpoint
将`llama-7b`的`epoch-10` Checkpoint恢复到本地`./restore/`路径，自动解压并校验MD5：
```bash
checkstore restore --model llama-7b --tag epoch-10 --output ./restore/
```

#### 3. 查看Checkpoint列表
```bash
# 查看所有模型的Checkpoint
checkstore list
# 查看指定模型的Checkpoint
checkstore list --model llama-7b
```

#### 4. 删除Checkpoint
```bash
checkstore delete --model llama-7b --tag epoch-10
```

#### 5. 手动垃圾回收
清理所有已软删除的Checkpoint（物理删除文件+元数据）：
```bash
checkstore gc
```

#### 6. 查看守护进程状态
```bash
checkstore daemon status
```

## 核心设计原则
1. **职责解耦**：各组件独立负责单一功能，通过标准化接口交互，便于扩展与维护
2. **数据安全**：全程MD5校验，软删除+垃圾回收分离，防止误删与数据损坏
3. **约束优先**：严格的模型-Checkpoint依赖约束、状态流转约束，保证系统一致性
4. **性能优化**：独立的压缩进程、任务调度机制，支持并发处理，适配大文件场景
5. **易用性**：简洁的CLI工具，覆盖所有核心操作，无需深入了解内部逻辑
6. **可扩展性**：预留自定义压缩策略（Our_Method）、复杂依赖关系的扩展接口

## 垃圾回收机制
### 触发条件
1. **手动触发**：执行`checkstore gc`命令
2. **自动触发**：可配置的定时任务（系统配置中设置）

### 回收策略
1. 仅回收**状态为DELETED**的Checkpoint，未标记的Checkpoint不参与回收
2. 回收前检查依赖关系，确保被依赖的Checkpoint未被标记为DELETED
3. 先批量收集所有可回收的Checkpoint，再统一执行物理删除（文件+元数据）
4. 回收过程中标记状态为DELETING，失败则标记为删除失败，需人工处理

### 设计初衷
将**软删除**与**物理删除**分离，防止用户误删Checkpoint：若GC未执行，用户可通过修改状态为AVAILABLE恢复已标记为DELETED的Checkpoint，避免数据丢失。

## 配置管理
系统配置通过`config.yaml`文件管理，由**Global Config Loader**解析并校验，核心配置项如下：
| 配置项 | 类型 | 说明 | 约束 |
|--------|------|------|------|
| compression_strategy | string | 默认压缩策略 | 必须为zipnn/xor/Our_Method之一 |
| root_path | string | Checkpoint存储根路径 | 路径必须存在且可写 |
| verify_md5_on_save | bool | 保存时是否校验MD5 | 仅布尔值（true/false） |
| verify_md5_on_restore | bool | 恢复时是否校验MD5 | 仅布尔值（true/false） |
| gc_cron | string | 自动GC的定时表达式 | 可选，如`0 0 * * *`（每天凌晨执行） |
| compress_concurrency | int | 压缩任务最大并发数 | 正整数 |

可通过CLI工具管理配置：
```bash
# 查看所有配置
checkstore config show
# 设置默认压缩策略
checkstore config set compression_strategy xor
```

## 开发与扩展
### 核心扩展点
1. **自定义压缩策略**：实现`Our_Method`压缩接口，对接Compression Engine即可集成
2. **存储层扩展**：适配分布式存储（如S3、HDFS），修改Storage Engine的文件读写接口
3. **元数据存储扩展**：将SQLite替换为MySQL/PostgreSQL，适配大规模部署场景
4. **监控扩展**：增加Prometheus/Grafana监控指标，监控Checkpoint状态、压缩效率、存储占用等
5. **API扩展**：在CLI基础上增加RESTful API，支持平台化集成

### 核心数据结构
系统定义了一系列核心数据结构，作为组件间交互的标准：
- CheckpointMetadata：Checkpoint元数据结构
- Dependency Graph：Checkpoint依赖关系图
- CompressionTask/CompressResults：压缩任务与结果
- DecompressionTask/DecompressResults：解压任务与结果
- Config：系统配置结构
- SafetensorsFileInfo：safetensors文件元数据结构

## 注意事项
1. **base Checkpoint保护**：删除base Checkpoint前必须先删除所有依赖的Checkpoint，否则系统会拒绝操作
2. **XOR模式约束**：使用XOR压缩时，必须指定有效的base Checkpoint（同一模型下的AVAILABLE状态Checkpoint）
3. **MD5校验**：建议开启`verify_md5_on_save`和`verify_md5_on_restore`，保证数据完整性（少量性能损耗）
4. **大文件处理**：系统适配GB级别Checkpoint，但建议保证存储目录有足够的磁盘空间，压缩过程中会产生临时文件
5. **并发控制**：压缩任务的并发数建议根据服务器性能配置，避免占用过多CPU/IO资源

## 版本说明
- **v1.1**：当前原型系统版本，实现核心ZipNN/XOR压缩、CLI操作、生命周期管理、垃圾回收等功能，预留自定义压缩策略扩展接口
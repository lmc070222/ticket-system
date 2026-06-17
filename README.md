# Ticket-System-2026 (火车票管理系统)

本项目为上海交通大学 ACM 班数据结构课程大作业。系统基于 **B+ 树** 实现高效的底层数据持久化存储，并在此基础上构建了一个支持用户管理、火车票订购、车次查询及订单管理的命令行火车票管理系统。


##  项目架构

项目的目录结构规范如下：

```text
Ticket-System-2026/
├── BPT/                  # B+ 树底层数据结构实现
├── build/                # CMake 编译构建目录（本地自动生成）
├── include/              # 头文件目录
│   ├── map/              # 自定义关联容器 (Map) 实现
│   ├── paser.hpp         # 指令解析器头文件
│   ├── train.hpp         # 车次基础类定义
│   ├── trainmanager.hpp  # 车次及业务逻辑管理
│   ├── user.hpp          # 用户基础类定义
│   ├── usermanger.hpp    # 用户管理逻辑
│   └── utils.hpp         # 辅助工具函数
├── src/                  # 源文件目录
│   ├── main.cpp          # 程序主入口
│   ├── paser.cpp         # 指令解析器实现
│   ├── train.cpp         # 车次相关实现
│   ├── trainmanager.cpp  # 车次管理实现
│   ├── user.cpp          # 用户相关实现
│   └── usermanager.cpp   # 用户管理实现
├── testcases/            # 测试用例目录
├── CMakeLists.txt        # CMake 配置文件
├── code                  # 编译生成的二进制可执行文件
├── management_system.md  # 管理系统阶段说明文档
└── run-test              # 自动化测试脚本


项目使用 CMake 进行构建，要求支持 C++17 或更高版本的编译器。

1. 本地编译
在项目根目录下执行以下命令进行开箱即用的常规编译：

Bash


mkdir -p build && cd build
cmake ..
make -j4
编译完成后，会在根目录下生成名为 code 的可执行文件。

2. 运行系统
你可以直接运行编译出的二进制文件，并通过标准输入交互：

Bash


./code
3. 运行测试用例
若要使用自动化脚本对 testcases/ 中的数据进行正确性与性能压力测试，可直接运行：

Bash


chmod +x run-test
./run-test
 核心模块实现
底层存储 (B+ Tree)：支持外部内存（Disk-based）的高效索引结构，确保在有限的内存限制下，所有数据（用户、车次、订单）均能正确实现文件持久化与快速检索。

用户管理 (UserManager)：实现用户的注册、登录、权限切换（由首个注册的管理员向下扩展）以及信息的查询与修改。

车次与车票逻辑 (TrainManager)：处理复杂的车次发布、车票预订、退票、候补（Queue）机制，并支持基于最少时间或最少花费的区间线段树/动态规划优化查询。

指令解析 (Parser)：高效解析标准的命令行输入，将其转换为系统内部对应的业务实体函数。
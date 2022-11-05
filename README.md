### 简介

玩具小型分布式数据库

interact_client：客户端，接收用户输入，发送至访问服务器处理

interact_server：访问服务器，解析用户输入，调用逻辑服务器的rpc服务进行处理

logic_server： 逻辑服务器，提供rpc服务，包括单条数据CRUD

- 水平分片：数据按主键hash至不同逻辑服务器进行处理（目前有4个分片，可增加分片）
- 数据存储：采用B+树进行数据持久化存储

### 构建与运行

操作系统：Ubuntu 22.04.1 LTS 64 位

处理器：Intel® Core™ i5-7400 CPU @ 3.00GHz × 4

依赖：grpc，可按照 [官网链接](https://grpc.io/docs/languages/cpp/quickstart) 进行安装

构建：cd run && ./build.sh

运行：可参考 build.sh 提示（注意各脚本均需独立终端）

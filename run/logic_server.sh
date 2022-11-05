#!/bin/bash

. defs.sh

# 分片索引
shard_idx=$1

target_dir=${build}/logic_server

# 工作目录，需要存放数据文件
workdir=${target_dir}/${shard_idx}_db
mkdir $workdir 2>/dev/null

# run
${target_dir}/logic_server $workdir ${logic_ports[shard_idx]}

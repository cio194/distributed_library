#!/bin/bash

. defs.sh

# 搜集rpc server地址
for i in 0 1 2 3; do
  rpc_servers="${rpc_servers} ${logic_addrs[i]}:${logic_ports[i]}"
done

# run
target=${build}/interact_server/interact_server
$target $interact_port $rpc_servers

#!/bin/bash

. defs.sh

target=${build}/interact_client/interact_client

# run
$target $interact_addr $interact_port

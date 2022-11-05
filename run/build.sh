#!/bin/bash

. defs.sh

cmake .. -B $build
cmake --build $build -j4

echo ""
echo "To run the program, please follow this order:"
echo "    logic_server.sh 0"
echo "    logic_server.sh 1"
echo "    logic_server.sh 2"
echo "    logic_server.sh 3"
echo "    interact_server.sh"
echo "    interact_client.sh"

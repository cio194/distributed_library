cmake_minimum_required(VERSION 3.19)
project(distributed_library)

set(CMAKE_CXX_STANDARD 17)

add_subdirectory(common)
#add_subdirectory(client)
add_subdirectory(interact_server)
add_subdirectory(logic_server)
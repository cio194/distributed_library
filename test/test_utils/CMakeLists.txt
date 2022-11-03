add_library(test_utils test_utils.cpp test_utils.h)
target_include_directories(test_utils PUBLIC .)
target_link_libraries(test_utils PUBLIC common logic_server_static)
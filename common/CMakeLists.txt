project(common)

# 收集源文件
FILE(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS *.cpp *.h)

# 指定库
SET(LIBRARIES book_proto)

add_library(${PROJECT_NAME} STATIC ${ALL_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARIES})
set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)

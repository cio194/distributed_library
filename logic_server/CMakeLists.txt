project(logic_server)

include(../proto/grpc.cmake)

# 收集源文件
FILE(GLOB_RECURSE ALL_SRC CONFIGURE_DEPENDS *.cpp *.h)
FILE(GLOB MAIN_SRC main.cpp)
FOREACH (F ${ALL_SRC})
    IF (NOT ${F} STREQUAL ${MAIN_SRC})
        SET(LIB_SRC ${LIB_SRC} ${F})
    ENDIF ()
ENDFOREACH (F)

# 指定库
SET(LIBRARIES common book_grpc_proto ${_REFLECTION}
        ${_GRPC_GRPCPP} ${_PROTOBUF_LIBPROTOBUF})

# 可执行文件
add_executable(${PROJECT_NAME} ${ALL_SRC})
target_include_directories(${PROJECT_NAME} PUBLIC .)
target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRARIES})

# 库，用于测试
set(LIB_TARGET ${PROJECT_NAME}_static)
add_library(${LIB_TARGET} STATIC ${LIB_SRC})
target_include_directories(${LIB_TARGET} PUBLIC .)
target_link_libraries(${LIB_TARGET} PUBLIC ${LIBRARIES})
set_target_properties(${LIB_TARGET} PROPERTIES LINKER_LANGUAGE CXX)

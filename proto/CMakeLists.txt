project(BookProto)

include(grpc.cmake)

# Proto file
get_filename_component(book_proto "book.proto" ABSOLUTE)
get_filename_component(book_proto_path "${book_proto}" PATH)

# Generated sources
set(book_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/book.pb.cc")
set(book_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/book.pb.h")
set(book_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/book.grpc.pb.cc")
set(book_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/book.grpc.pb.h")
add_custom_command(
        OUTPUT "${book_proto_srcs}" "${book_proto_hdrs}" "${book_grpc_srcs}" "${book_grpc_hdrs}"
        COMMAND ${_PROTOBUF_PROTOC}
        ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
        --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
        -I "${book_proto_path}"
        --plugin=protoc-gen-grpc="${_GRPC_CPP_PLUGIN_EXECUTABLE}"
        "${book_proto}"
        DEPENDS "${book_proto}")

# book_proto
add_library(book_proto ${book_proto_srcs} ${book_proto_hdrs})
target_link_libraries(book_proto ${_PROTOBUF_LIBPROTOBUF})
target_include_directories(book_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

# book_grpc_proto
add_library(book_grpc_proto
        ${book_grpc_srcs}
        ${book_grpc_hdrs}
        ${book_proto_srcs}
        ${book_proto_hdrs})
target_link_libraries(book_grpc_proto
        ${_REFLECTION}
        ${_GRPC_GRPCPP}
        ${_PROTOBUF_LIBPROTOBUF})
target_include_directories(book_grpc_proto PUBLIC ${CMAKE_CURRENT_BINARY_DIR})
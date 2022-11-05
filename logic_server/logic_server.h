#ifndef DISTRIBUTED_LIBRARY_LOGIC_SERVER_H
#define DISTRIBUTED_LIBRARY_LOGIC_SERVER_H

#include "book.grpc.pb.h"
#include "defs/defs.h"
#include "table/table.h"

#include <memory>
#include <string>
#include <thread>

#include <grpcpp/ext/proto_server_reflection_plugin.h>
#include <grpcpp/grpcpp.h>
#include <grpcpp/health_check_service_interface.h>

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;

class TableServiceImpl final : public book::Table::Service {
public:
  Status Insert(::grpc::ServerContext *context, const ::book::Book *request,
                ::book::RetCode *response) override;

  Status Delete(::grpc::ServerContext *context, const ::book::BookName *request,
                ::book::RetCode *response) override;

  // 仅更新borrow相关字段
  Status Update(::grpc::ServerContext *context, const ::book::Book *request,
                ::book::RetCode *response) override;

  Status Select(::grpc::ServerContext *context, const ::book::BookName *request,
                ::book::Book *response) override;

public:
  ~TableServiceImpl() { table_.Flush(); }

private:
  Table table_;
};

class LogicServer {
public:
  void Run(uint16_t port);

private:
  std::unique_ptr<Server> server_;
};

#endif //DISTRIBUTED_LIBRARY_LOGIC_SERVER_H

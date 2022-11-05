#ifndef DISTRIBUTED_LIBRARY_LOGIC_SERVER_H
#define DISTRIBUTED_LIBRARY_LOGIC_SERVER_H

#include "book.grpc.pb.h"
#include "defs/defs.h"
#include "table/table.h"

#include <memory>
#include <string>

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
private:
  Table table_;
};

class LogicServer {
public:
  void Run(uint16_t port);
};

#endif //DISTRIBUTED_LIBRARY_LOGIC_SERVER_H

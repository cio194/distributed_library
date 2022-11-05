#include "logic_server.h"

Status
TableServiceImpl::Insert(::grpc::ServerContext *context,
                         const ::book::Book *request,
                         ::book::RetCode *response) {
  int code = table_.Insert(Book::FromProto(*request));
  response->set_code(code);
  return Status::OK;
}

Status
TableServiceImpl::Delete(::grpc::ServerContext *context, const ::book::BookName *request, ::book::RetCode *response) {
  int code = table_.Delete(request->name());
  response->set_code(code);
  return Status::OK;
}

void LogicServer::Run(uint16_t port) {
  std::string server_address = std::string("0.0.0.0") + ":" +
                               std::to_string(port);
  TableServiceImpl service;

  grpc::EnableDefaultHealthCheckService(true);
  grpc::reflection::InitProtoReflectionServerBuilderPlugin();
  ServerBuilder builder;
  // Listen on the given address without any authentication mechanism.
  builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
  // Register "service" as the instance through which we'll communicate with
  // clients. In this case it corresponds to an *synchronous* service.
  builder.RegisterService(&service);
  // Finally assemble the server.
  std::unique_ptr<Server> server(builder.BuildAndStart());
  std::cout << "Server listening on " << server_address << std::endl;

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server->Wait();
}

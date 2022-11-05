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
TableServiceImpl::Delete(::grpc::ServerContext *context,
                         const ::book::BookName *request,
                         ::book::RetCode *response) {
  int code = table_.Delete(request->name());
  response->set_code(code);
  return Status::OK;
}

Status
TableServiceImpl::Select(::grpc::ServerContext *context,
                         const ::book::BookName *request,
                         ::book::Book *response) {
  auto b = table_.Select(request->name());
  *response = Book::ToProto(b);
  return Status::OK;
}

Status
TableServiceImpl::Update(::grpc::ServerContext *context,
                         const ::book::Book *request,
                         ::book::RetCode *response) {
  auto b = table_.Select(request->name());
  int code;
  // 记录不存在
  if (b.name[0] == '\0') {
    code = -1;
  } else {
    // 记录存在
    memcpy(b.borrower, request->borrower().c_str(), request->borrower().size() + 1);
    memcpy(b.borrow_date, request->borrow_date().c_str(), request->borrow_date().size() + 1);
    code = table_.Update(b);
  }
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
  server_ = builder.BuildAndStart();
  std::cout << "Server listening on " << server_address << std::endl;
  std::cout << "Press q to quit" << std::endl;

  // 创建控制台线程，用于终止服务器
  std::thread t([&] {
    std::string line;
    while (std::getline(std::cin, line)) {
      if (line == "q") {
        server_->Shutdown();
        break;
      } else {
        std::cout << "wrong command" << std::endl;
      }
    }
  });

  // Wait for the server to shutdown. Note that some other thread must be
  // responsible for shutting down the server for this call to ever return.
  server_->Wait();
  t.join();
}

#ifndef DISTRIBUTED_LIBRARY_RPC_CLIENT_H
#define DISTRIBUTED_LIBRARY_RPC_CLIENT_H

#include "book.grpc.pb.h"
#include "defs/defs.h"
#include "book/book.h"

#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

class TableClient {
public:
  explicit TableClient(std::shared_ptr<Channel> channel)
      : stub_(book::Table::NewStub(channel)) {}

  std::string Insert(const book::Book &b);

  std::string Delete(const book::BookName &bookname);

  std::string Update(const book::Book &b);

  std::string Select(const book::BookName &bookname);

private:
  std::unique_ptr<book::Table::Stub> stub_;

private:
  static constexpr const char *const kSuccess_ = "success";
  static constexpr const char *const kInternalError_ = "internal error";
  static constexpr const char *const kNotFound_ = "not found";
  static constexpr const char *const kAlreadyExist_ = "the same name entry already exist";
};

#endif //DISTRIBUTED_LIBRARY_RPC_CLIENT_H

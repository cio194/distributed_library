#include "rpc_client.h"

std::string TableClient::Insert(const book::Book &b) {
  book::RetCode reply;
  ClientContext context;
  Status status = stub_->Insert(&context, b, &reply);
  // Act upon its status.
  if (status.ok()) {
    if (reply.code() == 0) {
      return kSuccess_;
    } else {
      return kAlreadyExist_;
    }
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return kInternalError_;
  }
}

std::string TableClient::Delete(const book::BookName &name) {
  book::RetCode reply;
  ClientContext context;
  Status status = stub_->Delete(&context, name, &reply);
  // Act upon its status.
  if (status.ok()) {
    if (reply.code() == 0) {
      return kSuccess_;
    } else {
      return kNotFound_;
    }
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return kInternalError_;
  }
}

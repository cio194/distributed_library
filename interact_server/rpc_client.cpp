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

std::string TableClient::Delete(const book::BookName &bookname) {
  book::RetCode reply;
  ClientContext context;
  Status status = stub_->Delete(&context, bookname, &reply);
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

std::string TableClient::Update(const book::Book &b) {
  book::RetCode reply;
  ClientContext context;
  Status status = stub_->Update(&context, b, &reply);
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


std::string TableClient::Select(const book::BookName &bookname) {
  book::Book reply;
  ClientContext context;
  Status status = stub_->Select(&context, bookname, &reply);
  // Act upon its status.
  if (status.ok()) {
    if (reply.name() == bookname.name()) {
      return Book::ToPresent(Book::FromProto(reply));
    } else {
      return kNotFound_;
    }
  } else {
    std::cout << status.error_code() << ": " << status.error_message()
              << std::endl;
    return kInternalError_;
  }
}

#include "utils/custom_exceptions.h"
#include "book.grpc.pb.h"
#include "defs/defs.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <string>
#include <memory>

#include <grpcpp/grpcpp.h>

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;

std::string ProcessLine(const std::string &line) {
  /**
   * insert into book values ("book_name", "author_name", "publisher_name", "publish_time", "borrower_name", "borrower_time")
   * select * from book where book_name="xxx"
   * 暂定 select * from book where xxx(not book name)="xxx"
   * 暂定 select * where book_name like "正则"
   * 暂定 select * where borrower_time < "xxx"
   * update book set borrower_name="xxx", borrow_time="xxx" where book_name="xxx"
   * delete from book where book_name="xxx"
   */
  return std::string("processed");
}

class TableClient {
public:
  explicit TableClient(std::shared_ptr<Channel> channel)
      : stub_(book::Table::NewStub(channel)) {}

  int Insert(const book::Book &b) {
    book::RetCode reply;
    ClientContext context;
    Status status = stub_->Insert(&context, b, &reply);
    // Act upon its status.
    if (status.ok()) {
      return reply.code();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return -2;
    }
  }

  int Delete(const book::BookName &name) {
    book::RetCode reply;
    ClientContext context;
    Status status = stub_->Delete(&context, name, &reply);
    // Act upon its status.
    if (status.ok()) {
      return reply.code();
    } else {
      std::cout << status.error_code() << ": " << status.error_message()
                << std::endl;
      return -2;
    }
  }

private:
  std::unique_ptr<book::Table::Stub> stub_;
};

int main() {
//  int listenfd, connfd;
//  socklen_t clilen;
//  struct sockaddr_in cliaddr, servaddr;
//
//  listenfd = socket(AF_INET, SOCK_STREAM, 0);
//  if (listenfd < 0) throw unix_sys_error("socket");
//  // 打开端口复用
//  int one = 1;
//  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
//    throw unix_sys_error("setsockopt");
//
//  bzero(&servaddr, sizeof(servaddr));
//  servaddr.sin_family = AF_INET;
//  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
//  servaddr.sin_port = htons(kInteractPort);
//  if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
//    throw unix_sys_error("bind");
//
//  if (listen(listenfd, 5) == -1) throw unix_sys_error("listen");
//
//  while (true) {
//    // 获取客户连接
//    clilen = sizeof(cliaddr);
//    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
//    if (connfd == -1) {
//      if (errno == EINTR) continue;
//      else throw unix_sys_error("accept");
//    }
//    // 对客户连接进行处理（串行）
//    int readn = 0;
//    char read_buf[kMaxRequestLen];
//    std::string line;
//    while ((readn = read(connfd, read_buf, kMaxRequestLen)) > 0) {
//      // 读取到一行，进行处理
//      std::string result = ProcessLine(std::string(read_buf, readn));
//      int rn = result.size();
//      write(connfd, result.c_str(), std::min(rn, kMaxRequestLen));
//    }
//    close(connfd);
//  }

  std::string target_str = kLogicAddr + std::string(":") + std::to_string(kLogicPort);
  TableClient client(
      grpc::CreateChannel(target_str, grpc::InsecureChannelCredentials())
  );
  // insert 测试
  book::Book b;
  b.set_name("顺其自然");
  b.set_author("simple");
  b.set_publisher("1");
  b.set_publish_date("2022-11-04");
  b.set_borrower("1");
  b.set_borrow_date("2022-11-04");
  std::cout << "ret code: " << client.Insert(b) << std::endl;
}

#include "inseract_server.h"

InteractServer::InteractServer(uint16_t interact_port,
                               const std::vector<std::string> &rpc_targets)
    : kInteractPort_(interact_port) {
  for (const auto &rpc_target : rpc_targets) {
    clients_.emplace_back(grpc::CreateChannel(
        rpc_target, grpc::InsecureChannelCredentials()));
  }
}

void InteractServer::Run() {
  int listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) throw unix_sys_error("socket");

  // 打开端口复用
  int one = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
    throw unix_sys_error("setsockopt");

  // 初始化服务器地址
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(kInteractPort_);
  if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
    throw unix_sys_error("bind");

  // 监听端口
  if (listen(listenfd, 5) == -1) throw unix_sys_error("listen");

  while (true) {
    // 获取客户连接
    clilen = sizeof(cliaddr);
    connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);
    if (connfd == -1) {
      if (errno == EINTR) continue;
      else throw unix_sys_error("accept");
    }
    // 对客户连接进行处理（串行）
    int readn = 0;
    char read_buf[df::kMaxRequestLen];
    std::string line;
    while ((readn = read(connfd, read_buf, df::kMaxRequestLen)) > 0) {
      // 读取到一行，进行处理
      std::string result = ProcessLine(std::string(read_buf, readn));
      int rn = result.size() < df::kMaxRequestLen ? result.size() : df::kMaxRequestLen;
      write(connfd, result.c_str(), rn);
    }
    close(connfd);
  }
}

/**
  * insert into book values ("book_name", "author_name", "publisher_name", "publish_time", "borrower_name", "borrower_time")
  * select * from book where book_name="xxx"
  * 暂定 select * from book where xxx(not book name)="xxx"
  * 暂定 select * where book_name like "正则"
  * 暂定 select * where borrower_time < "xxx"
  * update book set borrower_name="xxx", borrow_time="xxx" where book_name="xxx"
  * delete from book where book_name="xxx"
  */
std::string InteractServer::ProcessLine(const std::string &line) {
  // 使用scanf进行简单解析
  static const std::string kBadLine = "bad command";
  if (strncmp(line.c_str(), "insert", 6) == 0) {
    // 解析
    Book b;
    if (sscanf(line.c_str(), Book::kInsertFormat,
               b.name, b.author, b.publisher, b.publish_date,
               b.borrower, b.borrow_date) == EOF) {
      return kBadLine;
    }
    // 执行
    return Shard(b.name).Insert(Book::ToProto(b));

  } else if (strncmp(line.c_str(), "delete", 6) == 0) {
    // 解析
    char name[Book::kNameLen] = {0};
    if (sscanf(line.c_str(), Book::kDeleteFormat, name) == EOF) {
      return kBadLine;
    }
    // 执行
    book::BookName bookname;
    bookname.set_name(name);
    return Shard(name).Delete(bookname);

  } else if (strncmp(line.c_str(), "update", 6) == 0) {
    // 解析
    Book b;
    if (sscanf(line.c_str(), Book::kUpdateFormat,
               b.name, b.borrower, b.borrow_date) == EOF) {
      return kBadLine;
    }
    // 执行
    return Shard(b.name).Update(Book::ToProto(b));

  } else if (strncmp(line.c_str(), "select", 6) == 0) {
    // 解析
    char name[Book::kNameLen] = {0};
    if (sscanf(line.c_str(), Book::kSelectFormat, name) == EOF) {
      return kBadLine;
    }
    // 执行
    book::BookName bookname;
    bookname.set_name(name);
    return Shard(name).Select(bookname);

  } else {
    return kBadLine;
  }
}

TableClient &InteractServer::Shard(const char *name) {
  // BKDR Hash Function
  uint32_t seed = 131; // 31 131 1313 13131 131313 etc..
  uint32_t hash = 0;
  while (*name) hash = hash * seed + (*name++);
  hash = hash & 0x7FFFFFFF;

  auto idx = hash % df::kShardNum;
  std::cout << "server " << idx << " handle request" << std::endl;
  return clients_[idx];
}

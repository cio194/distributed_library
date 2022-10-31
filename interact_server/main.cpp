#include "utils/custom_exceptions.h"
#include "defs/defs.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string>

std::string ProcessLine(const std::string &line) {
  return std::string("processed");
}

int main() {
  int listenfd, connfd;
  socklen_t clilen;
  struct sockaddr_in cliaddr, servaddr;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  if (listenfd < 0) throw unix_sys_error("socket");
  // 打开端口复用
  int one = 1;
  if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one)) < 0)
    throw unix_sys_error("setsockopt");

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  servaddr.sin_port = htons(kInteractPort);
  if (bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
    throw unix_sys_error("bind");

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
    char read_buf[kMaxRequestLen];
    std::string line;
    while ((readn = read(connfd, read_buf, kMaxRequestLen)) > 0) {
      // 读取到一行，进行处理
      std::string result = ProcessLine(std::string(read_buf, readn));
      int rn = result.size();
      write(connfd, result.c_str(), std::min(rn, kMaxRequestLen));
    }
    close(connfd);
  }
}

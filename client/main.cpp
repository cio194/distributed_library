#include "defs/defs.h"
#include "utils/custom_exceptions.h"
#include "utils/simple_utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>

inline void PrintCmd() {
  std::cout << "library> ";
}

void ProcessLine(const std::string &line) {
  if (line == "exit") exit(0);
  // 应与server通信，返回处理结果
  std::cout << "processed" << std::endl;
}

int main() {
  int connfd;
  struct sockaddr_in servaddr;

  connfd = socket(AF_INET, SOCK_STREAM, 0);
  if (connfd < 0) throw unix_sys_error("socket");

  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  if (inet_pton(AF_INET, kInteractAddr, &servaddr.sin_addr) != 1)
    throw unix_sys_error("inet_pton");
  servaddr.sin_port = htons(kInteractPort);

  if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    throw unix_sys_error("connect");

  PrintCmd();
  std::string line;
  char buf[kMaxRequestLen];
  while (std::getline(std::cin, line)) {
    // 错误处理输入
    if (line.empty()) continue;
    if (line.size() > kMaxRequestLen) {
      std::cout << "max line len: " << kMaxRequestLen << std::endl;
      continue;
    }
    // 退出程序
    if (line == "exit") {
      break;
    }
    // 普通命令，发送至服务端，并回显处理结果
    write(connfd, line.c_str(), line.size());
    int readn = read(connfd, buf, kMaxRequestLen);
    if (readn < 0) throw unix_sys_error("read");
    if (readn == 0) PrintExit("server exits abnormally");
    std::cout << std::string(buf, readn) << std::endl;
    // 再次获取输入
    PrintCmd();
  }
  close(connfd);
  return 0;
}
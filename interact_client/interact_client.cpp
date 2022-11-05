#include "interact_client.h"

void InteractClient::Run(const std::string &serv_addr, uint16_t port) {
  int connfd;
  struct sockaddr_in servaddr;

  connfd = socket(AF_INET, SOCK_STREAM, 0);
  if (connfd < 0) throw unix_sys_error("socket");

  // 初始化服务器地址
  bzero(&servaddr, sizeof(servaddr));
  servaddr.sin_family = AF_INET;
  if (inet_pton(AF_INET, serv_addr.c_str(), &servaddr.sin_addr) != 1)
    throw unix_sys_error("inet_pton");
  servaddr.sin_port = htons(port);

  // 连接到服务器
  if (connect(connfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0)
    throw unix_sys_error("connect");

  // 接收用户输入，并处理
  PrintCmd();
  std::string line;
  char buf[df::kMaxRequestLen];
  while (std::getline(std::cin, line)) {
    // 错误处理输入
    if (line.empty()) continue;
    if (line.size() > df::kMaxRequestLen) {
      std::cout << "max line len: " << df::kMaxRequestLen << std::endl;
      continue;
    }

    // 退出程序
    if (line == "exit") break;

    // 普通命令，发送至服务端，并回显处理结果
    write(connfd, line.c_str(), line.size());
    // todo socket中read write粘包等问题
    int readn = read(connfd, buf, df::kMaxRequestLen);
    if (readn < 0) throw unix_sys_error("read");
    if (readn == 0) PrintExit("server exits abnormally");
    std::cout << std::string(buf, readn) << std::endl;

    // 再次获取输入
    PrintCmd();
  }
  close(connfd);
}

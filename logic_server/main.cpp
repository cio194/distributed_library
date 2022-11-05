#include "logic_server.h"
#include <unistd.h>

static void Usage(const char *name) {
  std::cout << "usage: " << name << " workdir port" << std::endl;
  exit(-1);
}

int main(int argc, char **argv) {
  if (argc != 3) Usage(argv[0]);

  // 切换工作目录
  char *workdir = argv[1];
  if (chdir(workdir) != 0) throw unix_sys_error("chdir");

  // 转换interact port
  char *port_s = argv[2];
  char *ptr;
  long port;
  port = strtol(port_s, &ptr, 10);
  if (port == 0) PrintExit("bad port");

  LogicServer server;
  server.Run(port);

  return 0;
}

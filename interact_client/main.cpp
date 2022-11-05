#include "interact_client.h"

static void Usage(const char *name) {
  std::cout << "usage: " << name << " addr port" << std::endl;
  exit(-1);
}

int main(int argc, char **argv) {
  if (argc != 3) Usage(argv[0]);

  // 端口转换为整数
  char *ptr;
  long port;
  port = strtol(argv[2], &ptr, 10);
  if (port == 0) PrintExit("bad port");

  InteractClient client;
  client.Run(argv[1], port);
  return 0;
}
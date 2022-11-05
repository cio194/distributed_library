#include "rpc_client.h"
#include "inseract_server.h"
#include "utils/utils.h"

static void Usage(const char *name) {
  std::cout << "usage: " << name << " listen_port";
  for (int i = 1; i <= df::kShardNum; ++i) {
    std::cout << " rpc_addr" << i << ":port" << i;
  }
  std::cout << std::endl;
  exit(-1);
}

int main(int argc, char **argv) {
  if (argc != 1 + 1 + df::kShardNum) Usage(argv[0]);

  // 转换interact port
  char *port_s = argv[1];
  char *ptr;
  long interact_port;
  interact_port = strtol(port_s, &ptr, 10);
  if (interact_port == 0) PrintExit("bad interact_port");

  // 获取rpc server地址
  std::vector<std::string> rpc_targets;
  for (int i = 2; i < argc; ++i) rpc_targets.emplace_back(argv[i]);

  // run server
  InteractServer server(interact_port, rpc_targets);
  server.Run();
  return 0;
}

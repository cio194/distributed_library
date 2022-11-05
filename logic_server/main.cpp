#include "logic_server.h"

int main() {
  // todo 服务器停止运行时，flush table
  LogicServer server;
  server.Run(df::kLogicPort);
  return 0;
}

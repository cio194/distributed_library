#include "logic_server.h"
#include <thread>

int main() {
  LogicServer server;
  server.Run(df::kLogicPort);

  return 0;
}

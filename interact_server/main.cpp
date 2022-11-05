#include "rpc_client.h"
#include "inseract_server.h"

int main() {
  std::string rpc_target = std::string(df::kLogicAddr) + ":" +
                           std::to_string(df::kLogicPort);
  InteractServer server(df::kInteractPort, rpc_target);
  server.Run();
  return 0;
}

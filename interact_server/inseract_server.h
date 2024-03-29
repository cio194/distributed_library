#ifndef DISTRIBUTED_LIBRARY_INSERACT_SERVER_H
#define DISTRIBUTED_LIBRARY_INSERACT_SERVER_H

#include "utils/my_exceptions.h"
#include "defs/defs.h"
#include "book/book.h"
#include "rpc_client.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>

#include <vector>
#include <cstdint>
#include <cstdio>

class InteractServer {
public:
  InteractServer(uint16_t interact_port, const std::vector<std::string> &rpc_targets);
  void Run();
private:
  std::string ProcessLine(const std::string &line);
  TableClient& Shard(const char *name);
private:
  const uint16_t kInteractPort_;
  std::vector<TableClient> clients_;
};

#endif //DISTRIBUTED_LIBRARY_INSERACT_SERVER_H

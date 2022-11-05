#ifndef DISTRIBUTED_LIBRARY_INTERACT_CLIENT_H
#define DISTRIBUTED_LIBRARY_INTERACT_CLIENT_H

#include "utils/my_exceptions.h"
#include "defs/defs.h"
#include "utils/utils.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>

class InteractClient {
public:
  void Run(const std::string &serv_addr, uint16_t port);

  static void PrintCmd() { std::cout << "library> "; }
};

#endif //DISTRIBUTED_LIBRARY_INTERACT_CLIENT_H

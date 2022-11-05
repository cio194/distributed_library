#include "interact_client.h"

int main() {
  InteractClient client;
  client.Run(df::kInteractAddr, df::kInteractPort);
  return 0;
}
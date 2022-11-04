#ifndef DISTRIBUTED_LIBRARY_DEFS_H
#define DISTRIBUTED_LIBRARY_DEFS_H

#include <cstdint>

constexpr int kMaxRequestLen = 4096;

extern const char *const kInteractAddr;
constexpr uint16_t kInteractPort = 50050;

extern const char *const kLogicAddr;
constexpr uint16_t kLogicPort = 50051;

#endif //DISTRIBUTED_LIBRARY_DEFS_H

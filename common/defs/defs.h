#ifndef DISTRIBUTED_LIBRARY_DEFS_H
#define DISTRIBUTED_LIBRARY_DEFS_H

#include <cstdint>

namespace df {

constexpr int kMaxRequestLen = 4096;

constexpr const char *const kInteractAddr = "127.0.0.1";
constexpr uint16_t kInteractPort = 50050;

constexpr const char *const kLogicAddr = "127.0.0.1";
constexpr uint16_t kLogicPort = 50051;

} // namespace df

#endif //DISTRIBUTED_LIBRARY_DEFS_H

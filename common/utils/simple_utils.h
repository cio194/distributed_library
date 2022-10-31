#ifndef BP_TREE_DISK_SIMPLE_UTILS_H
#define BP_TREE_DISK_SIMPLE_UTILS_H

#include "custom_exceptions.h"
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

inline void PrintExit(const std::string &msg) {
  std::cerr << msg << std::endl;
  exit(-1);
}

namespace simple {

inline void Write(std::fstream &f, void *s, std::streamsize n) {
  if (!f.write(reinterpret_cast<const char *>(s), n))
    throw unix_sys_error("write");
}

inline void Read(std::fstream &f, void *s, std::streamsize n) {
  if (!f.read(reinterpret_cast<char *>(s), n))
    throw unix_sys_error("read");
}

}  // namespace simple


#endif //BP_TREE_DISK_SIMPLE_UTILS_H

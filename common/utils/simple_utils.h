#ifndef BP_TREE_DISK_SIMPLE_UTILS_H
#define BP_TREE_DISK_SIMPLE_UTILS_H

#include "custom_exceptions.h"
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <cstdlib>
#include <iostream>
#include <fstream>

inline void PrintExit(const std::string &msg) {
  std::cerr << msg << std::endl;
  exit(-1);
}

inline void WriteThrow(std::fstream &f, void *s, std::streamsize n) {
  if (!f.write(reinterpret_cast<const char *>(s), n)) throw unix_sys_error("write");
}

inline void ReadThrow(std::fstream &f, void *s, std::streamsize n) {
  if (!f.read(reinterpret_cast<char *>(s), n)) throw unix_sys_error("read");
}

inline bool RegExists(const std::string &path) {
  // 文件不存在
  if (access(path.c_str(), F_OK) != 0) return false;
  // 文件存在但并非reg
  struct stat sb;
  if (lstat(path.c_str(), &sb) != 0) throw unix_sys_error("lstat " + path);
  if (!S_ISREG(sb.st_mode)) PrintExit(path + " not reg");
  // reg文件存在
  return true;
}


#endif //BP_TREE_DISK_SIMPLE_UTILS_H

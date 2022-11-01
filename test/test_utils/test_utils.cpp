#include "test_utils.h"

TmpFiles tmp_files{};

std::string rand_str(const int len) {
  std::string str;
  char c;
  for (int i = 0; i < len; ++i) {
    c = 'a' + rand() % 26;
    str.push_back(c);
  }
  return std::move(str);
}

std::fstream GenTmpFile() {
  std::string filename = rand_str(32) + ".tmp";
  std::fstream f{filename, std::ios_base::in | std::ios_base::out |
                           std::ios_base::trunc};
  if (!f) throw unix_sys_error("create file");
  tmp_files.Push(filename);
  return std::move(f);
}

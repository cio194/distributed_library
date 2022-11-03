#ifndef DISTRIBUTED_LIBRARY_TEST_UTILS_H
#define DISTRIBUTED_LIBRARY_TEST_UTILS_H

#define private public

#include "utils/custom_exceptions.h"
#include "table/book.h"
#include "table/table.h"
#include <vector>
#include <string>
#include <cstdlib>
#include <fstream>
#include <random>
#include <cassert>
#include <set>
#include <climits>
#include <algorithm>

class TmpFiles {
  std::vector<std::string> files_;
public:
  ~TmpFiles() {
    std::string cmd = "rm -rf ";
    for (const auto &file : files_) {
      cmd += file;
      cmd.push_back(' ');
    }
    system(cmd.c_str());
  }

  void Push(const std::string &filename) {
    files_.push_back(filename);
  }
};

extern TmpFiles tmp_files;

std::string rand_str(const int len);

std::fstream GenTmpFile();

std::vector<Book> GenBooks(int count);

bool CheckBookTree(Table &table, const std::vector<Book> &books);

#endif //DISTRIBUTED_LIBRARY_TEST_UTILS_H

#ifndef DISTRIBUTED_LIBRARY_TABLE_H
#define DISTRIBUTED_LIBRARY_TABLE_H

#include "book.h"
#include "b_plus_tree/b_plus_tree.h"
#include "utils/simple_utils.h"
#include <string>

class Table {
public:
  // 创建，或者加载表
  Table();
  // 插入一条数据
  int Insert(const Book &book);
  // 删除一条数据
  int Delete(const std::string &name);
  // 更新一条数据
  int UpdateOne(const Book &book);
  // 查询一条数据
  int SelectOne(const std::string &name, Book &book);

public:
  static const char *const kBaseDir_;
  static const char *const kClusterFile_;
private:
  std::unique_ptr<bplus::BPlusTree> tree_;
};


#endif //DISTRIBUTED_LIBRARY_TABLE_H

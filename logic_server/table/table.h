#ifndef DISTRIBUTED_LIBRARY_TABLE_H
#define DISTRIBUTED_LIBRARY_TABLE_H

#include "book/book.h"
#include "b_plus_tree/b_plus_tree.h"
#include "utils/utils.h"
#include <string>

class Table {
public:
  // 创建，或者加载表
  Table();
  // 插入一条数据
  int Insert(const Book &b);
  // 删除一条数据
  int Delete(const std::string &name);
  // 更新一条数据
  int Update(const Book &b);
  // 查询一条数据
  Book Select(const std::string &name);

  void Flush() { tree_->Flush(); }

public:
  static const char *const kBaseDir_;
  static const char *const kClusterFile_;
private:
  std::unique_ptr<bplus::BPlusTree> tree_;
};


#endif //DISTRIBUTED_LIBRARY_TABLE_H

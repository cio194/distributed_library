#ifndef DISTRIBUTED_LIBRARY_TABLE_H
#define DISTRIBUTED_LIBRARY_TABLE_H

#include "book.h"
#include <string>

class Table {
public:
  // 创建，或者加载表
  Table();
  // 插入一条数据
  int InsertOne(const Book &book);
  // 查询一条数据
  int SelectOne(const std::string &book_name, Book &book);
  // 更新一条数据
  int UpdateOne(const Book &book);
  // 删除一条数据
  int DeleteOne(const std::string& book_name);
private:

};


#endif //DISTRIBUTED_LIBRARY_TABLE_H

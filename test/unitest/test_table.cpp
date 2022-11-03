#include "test_utils.h"
#include "table/table.h"

void TestCreate() {
  Table table;
  tmp_files.Push(Table::kBaseDir_);
}

void TestInsert() {
  Table table;
  tmp_files.Push(Table::kBaseDir_);
  // 生成随机books（name唯一且有序
  int count = 100000;
  auto books = GenBooks(count);
  // 打乱后插入
  auto rbooks = books;
  static std::random_device rd;
  static std::mt19937 gen(rd());
  std::shuffle(rbooks.begin(), rbooks.end(), gen);
  for (const auto &book : rbooks) {
    table.Insert(book);
  }
  // 校验树
  bool ck = CheckBookTree(table, books);
  std::cout << "INSERT " << count << " book ";
  if (!ck) {
    std::cout << "ERROR" << std::endl;
    exit(-1);
  }
  std::cout << "SUCCESS" << std::endl;
}

void TestDelete() {
  // 新建表，并插入数据
  Table table;
  tmp_files.Push(Table::kBaseDir_);
  int count = 10000;
  auto books = GenBooks(count);
  for (const auto &book : books) table.Insert(book);
  // 删除数据，并校验
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis;
  int test_n = 1;
  for (int i = 1; i <= count; ++i) {
    // 每次随机选取索引，进行删除
    int idx = dis(gen) % books.size();
    table.Delete(books[idx].name);
    books.erase(books.begin() + idx);
    // 每100条数据检验一次
    if ((i % 100) != 0) continue;
    bool ck = CheckBookTree(table, books);
    std::cout << "DELETE " << test_n++
              << " with idx " << idx << " ";
    if (!ck) {
      std::cout << "ERROR" << std::endl;
      exit(-1);
    }
    std::cout << "SUCCESS" << std::endl;
  }
}

int main() {
//  TestCreate();
//  TestInsert();
  TestDelete();
}
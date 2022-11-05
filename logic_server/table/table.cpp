#include "table.h"

const char *const Table::kBaseDir_ = "data";
const char *const Table::kClusterFile_ = "book_name.idx";

Table::Table() {
  auto path = std::string(kBaseDir_) + "/" + kClusterFile_;
  if (DirExists(kBaseDir_)) {
    // 加载
    tree_ = std::make_unique<bplus::BPlusTree>(
        path, Book::NameLess, Book::NameEqual);
  } else {
    // 新建
    Mkdir(kBaseDir_);
    tree_ = std::make_unique<bplus::BPlusTree>(
        path, Book::NameLess, Book::NameEqual,
        Book::kNameLen, Book::kBookLen);
  }
}

int Table::Insert(const Book &b) {
  auto entry = Book::ToStorage(b);
  return tree_->Insert(entry.c_str());
}

int Table::Delete(const std::string &name) {
  return tree_->Remove(name.c_str());
}

int Table::Update(const Book &b) {
  if (Delete(b.name) != 0) return -1;
  return Insert(b);
}

Book Table::Select(const std::string &name) {
  std::string str;
  int code = tree_->Search(name.c_str(), str);
  if (code == 0) {
    return Book::FromStorage(str.c_str());
  } else {
    return Book{};
  }
}

#include "book.h"

bool Book::NameLess(const char *name1, const char *name2) {
  int i = 0;
  while (name1[i] != '\0' && name2[i] != '\0') {
    if (name1[i] != name2[i]) break;
    ++i;
  }
  return name1[i] < name2[i];
}

bool Book::NameEqual(const char *name1, const char *name2) {
  int i = 0;
  while (name1[i] != '\0' && name2[i] != '\0') {
    if (name1[i] != name2[i]) break;
    ++i;
  }
  return name1[i] == name2[i];
}

std::string Book::ToStorage(const Book &book) {
  std::string str;
  str.append(book.name, sizeof(book.name));
  str.append(book.author, sizeof(book.author));
  str.append(book.publisher, sizeof(book.publisher));
  str.append(book.publish_date, sizeof(book.publish_date));
  str.append(book.borrower, sizeof(book.borrower));
  str.append(book.borrow_date, sizeof(book.borrow_date));
  return std::move(str);
}

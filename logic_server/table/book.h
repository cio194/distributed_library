#ifndef DISTRIBUTED_LIBRARY_BOOK_H
#define DISTRIBUTED_LIBRARY_BOOK_H

#include <cstdint>
#include <string>
#include <ctime>

struct Book {
  static constexpr int kNameLen = 96;
  static constexpr int kAuthorLen = 64;
  static constexpr int kPublisherLen = 64;
  static constexpr int kBorrowerLen = 64;
  static constexpr int kDateLen = 16;

  char name[kNameLen];
  char author[kAuthorLen];
  char publisher[kPublisherLen];
  char publish_date[kDateLen];
  char borrower[kBorrowerLen];
  char borrow_date[kDateLen];

  static std::string ToStorage(const Book &book);
  static void FromStorage(const char *c_str, Book &book);

  // present：各字段中间配以若干空格
  static std::string ToPresent(const Book &book);
  static void FromPresent(const char *c_str, Book &book);
};

#endif //DISTRIBUTED_LIBRARY_BOOK_H

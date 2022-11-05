#ifndef DISTRIBUTED_LIBRARY_BOOK_H
#define DISTRIBUTED_LIBRARY_BOOK_H

#include "book.pb.h"

#include <cstdint>
#include <string>
#include <ctime>
#include <cstdlib>

struct Book {
  static constexpr int kNameLen = 96;
  static constexpr int kAuthorLen = 64;
  static constexpr int kPublisherLen = 64;
  static constexpr int kBorrowerLen = 64;
  static constexpr int kDateLen = 16;
  static constexpr int kBookLen = kNameLen + kAuthorLen +
                                  kPublisherLen + kDateLen +
                                  kBorrowerLen + kDateLen;
  // 语句格式字符串，与上面常量有耦合关系
  static constexpr const char *const kInsertFormat = "insert %95s %63s %63s %15s %63s %15s";
//  static const std::string kDelete;
//  static const std::string kUpdate;
//  static const std::string kSelect;

  char name[kNameLen] = {0};
  char author[kAuthorLen] = {0};
  char publisher[kPublisherLen] = {0};
  char publish_date[kDateLen] = {0};
  char borrower[kBorrowerLen] = {0};
  char borrow_date[kDateLen] = {0};

  static std::string ToStorage(const Book &b);
  static Book FromStorage(const char *c_str);

  static book::Book ToProto(const Book &b);
  static Book FromProto(const book::Book &proto);

  static bool NameLess(const char *name1, const char *name2);
  static bool NameEqual(const char *name1, const char *name2);
};

#endif //DISTRIBUTED_LIBRARY_BOOK_H

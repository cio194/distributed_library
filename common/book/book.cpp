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

std::string Book::ToStorage(const Book &b) {
  std::string str;
  str.append(b.name, sizeof(b.name));
  str.append(b.author, sizeof(b.author));
  str.append(b.publisher, sizeof(b.publisher));
  str.append(b.publish_date, sizeof(b.publish_date));
  str.append(b.borrower, sizeof(b.borrower));
  str.append(b.borrow_date, sizeof(b.borrow_date));
  return std::move(str);
}

Book Book::FromStorage(const char *c_str) {
  Book b;
  memcpy(b.name, c_str, kNameLen);
  c_str += kNameLen;
  memcpy(b.author, c_str, kAuthorLen);
  c_str += kAuthorLen;
  memcpy(b.publisher, c_str, kPublisherLen);
  c_str += kPublisherLen;
  memcpy(b.publish_date, c_str, kDateLen);
  c_str += kDateLen;
  memcpy(b.borrower, c_str, kBorrowerLen);
  c_str += kBorrowerLen;
  memcpy(b.borrow_date, c_str, kDateLen);
  return b;
}

book::Book Book::ToProto(const Book &b) {
  book::Book proto;
  proto.set_name(b.name);
  proto.set_author(b.author);
  proto.set_publisher(b.publisher);
  proto.set_publish_date(b.publish_date);
  proto.set_borrower(b.borrower);
  proto.set_borrow_date(b.borrow_date);
  return proto;
}

Book Book::FromProto(const book::Book &proto) {
  Book b;
  memcpy(b.name, proto.name().c_str(), proto.name().size() + 1);
  memcpy(b.author, proto.author().c_str(), proto.author().size() + 1);
  memcpy(b.publisher, proto.publisher().c_str(), proto.publisher().size() + 1);
  memcpy(b.publish_date, proto.publish_date().c_str(), proto.publish_date().size() + 1);
  memcpy(b.borrower, proto.borrower().c_str(), proto.borrower().size() + 1);
  memcpy(b.borrow_date, proto.borrow_date().c_str(), proto.borrow_date().size() + 1);
  return b;
}

#include "test_utils.h"

TmpFiles tmp_files{};

std::string rand_str(const int len) {
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis(0, 25);
  std::string str;
  int c;
  for (int i = 0; i < len; ++i) {
    c = 'a' + dis(gen);
    str.push_back(c);
  }
  return std::move(str);
}

std::fstream GenTmpFile() {
  std::string filename = rand_str(32) + ".tmp";
  std::fstream f{filename, std::ios_base::in | std::ios_base::out |
                           std::ios_base::trunc};
  if (!f) throw unix_sys_error("create file");
  tmp_files.Push(filename);
  return std::move(f);
}

// 产生长度在[1, limit)的随机字符串
static std::string RandStr(int limit) {
  assert(limit > 1);
  static std::random_device rd;
  static std::mt19937 gen(rd());
  static std::uniform_int_distribution<> dis;
  int len = 1 + dis(gen) % (limit - 1);
  return rand_str(len);
}

// 给定book name，产生随机book
static Book RandBook(const std::string &name) {
  auto author = RandStr(Book::kAuthorLen);
  auto publisher = RandStr(Book::kPublisherLen);
  auto publish_date = RandStr(Book::kDateLen);
  auto borrower = RandStr(Book::kBorrowerLen);
  auto borrow_date = RandStr(Book::kDateLen);

  Book book;
  memcpy(book.name, name.c_str(), name.size() + 1);
  memcpy(book.author, author.c_str(), author.size() + 1);
  memcpy(book.publisher, publisher.c_str(), publisher.size() + 1);
  memcpy(book.publish_date, publish_date.c_str(), publish_date.size() + 1);
  memcpy(book.borrower, borrower.c_str(), borrower.size() + 1);
  memcpy(book.borrow_date, borrow_date.c_str(), borrow_date.size() + 1);
  return book;
}

std::vector<Book> GenBooks(int count) {
  std::set<std::string> names;
  while (names.size() < count) names.insert(RandStr(Book::kNameLen));

  std::vector<Book> books(count);
  int i = 0;
  for (const auto &name : names) {
    books[i++] = RandBook(name);
  }
  return std::move(books);
}

static bool CheckOrderNum(bplus::BPlusTree *tree, bplus::BPlusNode *t);
static bool CheckKeySorted(bplus::BPlusTree *tree, bplus::BPlusNode *t);
static bool CheckParentAndChildSorted(bplus::BPlusTree *tree,
                                      bplus::BPlusNode *t, bplus::BPlusNode *tp,
                                      const std::string &lk, const std::string &rk);
static bool CheckLeafData(bplus::BPlusTree *tree, const std::vector<Book> &books);
static const std::string &MinName();
static const std::string &MaxName();

bool CheckBookTree(Table &table, const std::vector<Book> &books) {
  table.tree_->Flush();
  auto tree = table.tree_.get();
  auto root = tree->Root();
  if (!CheckOrderNum(tree, tree->Get(root))) {
    std::cout << "CheckOrderNum ";
    return false;
  }
  if (!CheckKeySorted(tree, tree->Get(root))) {
    std::cout << "CheckKeySorted ";
    return false;
  }
  if (!CheckParentAndChildSorted(tree, tree->Get(root), nullptr,
                                 MinName(), MaxName())) {
    std::cout << "CheckParentAndChildSorted ";
    return false;
  }
  if (!CheckLeafData(tree, books)) {
    std::cout << "CheckLeafData ";
    return false;
  }
  return true;
}

bool CheckOrderNum(bplus::BPlusTree *tree, bplus::BPlusNode *t) {
  using namespace bplus;
  // 内部结点
  if (t->type == BP::kInternal) {
    int least = t->pos == tree->Root() ? 1 : tree->kInterLeast_;
    if (t->KNum() + 1 != t->children.size() || t->KNum() < least ||
        t->KNum() > tree->kInterMost_) {
      return false;
    }
    // 递归
    auto children = t->children;
    for (auto child : children) {
      t = tree->Get(child);
      if (!CheckOrderNum(tree, t)) {
        return false;
      }
    }
    return true;
  }
  // 叶结点
  int least = t->pos == tree->Root() ? 0 : tree->kLeafLeast_;
  return least <= t->ENum() && t->ENum() <= tree->kLeafMost_;
}

bool CheckKeySorted(bplus::BPlusTree *tree, bplus::BPlusNode *t) {
  using namespace bplus;
  // 内部结点
  if (t->type == BP::kInternal) {
    int n = t->KNum();
    for (int i = 0; i < n - 1; ++i) {
      if (tree->KeyGE(t->KI(i), t->KI(i + 1))) {
        return false;
      }
    }
    // 递归
    auto children = t->children;
    for (auto child : children) {
      t = tree->Get(child);
      if (!CheckKeySorted(tree, t)) {
        return false;
      }
    }
    return true;
  }
  // 叶结点
  int n = t->ENum();
  for (int i = 0; i < n - 1; ++i) {
    if (tree->KeyGE(t->EI(i), t->EI(i + 1))) {
      return false;
    }
  }
  return true;
}

const std::string &MinName() {
  static const std::string name(Book::kNameLen, '\0');
  return name;
}

const std::string &MaxName() {
  static const std::string name(Book::kNameLen, CHAR_MAX);
  return name;
}

bool CheckParentAndChildSorted(bplus::BPlusTree *tree,
                               bplus::BPlusNode *t, bplus::BPlusNode *tp,
                               const std::string &lk, const std::string &rk) {
  using namespace bplus;
  if (t->type == BP::kInternal) {
    if (tree->KeyLE(t->KI(0), lk.c_str()) ||
        tree->KeyGE(t->KI(t->KNum() - 1), rk.c_str())) {
      return false;
    }
    // 递归
    auto ppos = t->pos;
    auto knum = t->KNum();
    auto children = t->children;
    for (int i = 0; i < children.size(); ++i) {
      tp = tree->Get(ppos);
      t = tree->Get(children[i]);
      if (!CheckParentAndChildSorted(
          tree, t, tp,
          i == 0 ? MinName() : std::string(tp->KI(i - 1), BP::kKeySize),
          i == knum ? MaxName() : std::string(tp->KI(i), BP::kKeySize))) {
        return false;
      }
    }
    return true;
  }
  // 叶结点
  if (tree->KeyLess(t->EI(0), lk.c_str()) ||
      tree->KeyGE(t->EI(t->ENum() - 1), rk.c_str())) {
    return false;
  }
  return true;
}

bool CheckLeafData(bplus::BPlusTree *tree, const std::vector<Book> &books) {
  using namespace bplus;
  auto it = BPlusTree::Iterator(tree);
  int64_t i = 0;
  while (!it.IsEnd()) {
    auto str = *it;
    ++it;
    if (Book::ToStorage(books[i++]) != str) {
      return false;
    }
  }
  return i == books.size();
}

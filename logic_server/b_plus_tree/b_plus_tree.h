#ifndef BP_TREE_DISK_B_PLUS_TREE_H
#define BP_TREE_DISK_B_PLUS_TREE_H

#include "cache.h"
#include "b_plus_struct.h"
#include <string>
#include <memory>
#include <stack>

namespace bplus {

/**
 * TODO
 * 在文件头中添加最左叶结点位置  OK
 * 常量重构 OK
 * 重构 Header  OK
 * 重构 Node  OK
 * 重构 node cache lru cache  OK
 * 修改key，key需要提供长度、比较函数 OK
 * 暂定 添加压缩算法清理碎片
 */
class BPlusTree {
  using Node = BPlusNode;
public:
  explicit BPlusTree(const std::string &path,
                     bool (*key_less)(const char *k1, const char *k2),
                     bool (*key_equal)(const char *k1, const char *k2),
                     int key_size, int entry_size);
  explicit BPlusTree(const std::string &path,
                     bool (*key_less)(const char *k1, const char *k2),
                     bool (*key_equal)(const char *k1, const char *k2));
  void Flush();
  bool Empty();

  int Insert(const char *entry);
  int Remove(const char *key);
  int Search(const char *key, std::string &entry);
  int SearchRange(const char *lk, const char *rk, std::string &entries);

  // 类似迭代器，每次返回1条记录，以提供遍历操作
  class Iterator {
    bp_pos pos_;
    int idx_;
    BPlusTree *tree_;
  public:
    explicit Iterator(BPlusTree *tree);
    Iterator &operator++();
    std::string operator*();

    bool IsEnd() { return pos_ == 0; }
  };

private:
  void CalculateOrder();
  Node *ToLeaf(const char *key);
  bool found_in_leaf_;
  int SearchLeaf(const char *key, Node *t);

  void InsertLeaf(Node *t, int insert_idx, const char *entry);
  void InsertInternal(Node *t, int insert_idx, const char *nkey, bp_pos npos);
  void HandleFull(Node *t);
  bool MoveAble(Node *t);
  void Split(Node *t);
  std::string SplitInternal(Node *t, Node *nt);
  std::string SplitLeaf(Node *t, Node *nt);

  void RemoveEntry(Node *t, int rpos);
  void HandleTooLittle(Node *t);
  void Merge(Node *tp, int ti);
  bool BorrowAble(Node *t);

  void MoveToLeft(Node *tl, Node *tr, Node *tp, int ki);
  void MoveToRight(Node *tl, Node *tr, Node *tp, int ki);
  Node *LeftBro(Node *tp, int ti);
  Node *RightBro(Node *tp, int ti);

private:

  bp_pos Root() { return cache_->header_.root_pos; }

  Node *Get(bp_pos pos) { return cache_->Get(pos); }

  bool (*KeyLess)(const char *k1, const char *k2);
  bool (*KeyEqual)(const char *k1, const char *k2);

  bool KeyGreater(const char *k1, const char *k2) {
    return KeyLess(k2, k1);
  }

  bool KeyLE(const char *k1, const char *k2) {
    return !KeyGreater(k1, k2);
  }

  bool KeyGE(const char *k1, const char *k2) {
    return !KeyLess(k1, k2);
  }

private:
  // 叶结点、内部结点阶数，其他变量用于简化编程
  int kInterOrder_;
  int kInterLeast_;
  int kInterMost_;
  int kLeafOrder_;
  int kLeafLeast_;
  int kLeafMost_;

  std::unique_ptr<Cache> cache_;
  std::vector<bp_pos> pstack_;  // 保存父结点位置，对插入等需回溯的操作提供辅助
  std::vector<int> istack_;  // 保存当前结点在父结点中位置
};

}  // namespace bplus

#endif //BP_TREE_DISK_B_PLUS_TREE_H

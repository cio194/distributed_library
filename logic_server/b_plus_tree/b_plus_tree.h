#ifndef BP_TREE_DISK_B_PLUS_TREE_H
#define BP_TREE_DISK_B_PLUS_TREE_H

#include "node_cache.h"
#include "tree_file_struct.h"
#include <string>
#include <memory>
#include <stack>

namespace simple {

// key应为基本类型，且不应为浮点数
template<typename Key>
class BPlusTree {
  using Node = BPlusNode<Key>;
public:
  explicit BPlusTree(const std::string &tree_file_path, int entry_size);
  int Insert(const std::string &entry);
  int Remove(const Key &key);
  int Search(const Key &key, std::string &entry_found);
  std::vector<std::string> SearchRange(const Key &L, const Key &R);
  void Flush();
  bool IsEmpty();
private:
  Node *DownToLeaf(const Key &key);
  int SearchInLeaf(const Key &key, Node *t, bool &found);
  void InsertLeaf(Node *t, int insert_pos, const std::string &entry);
  void InsertInternal(Node *t, int insert_pos, const Key &nk, b_plus_off nt_pos);
  void RemoveEntry(Node *t, int remove_pos);
  bool CouldBorrow(Node *t);
  void Split(Node *t);
  void MoveToLeft(Node *tl, Node *tr, Node *tp, int ki);
  void MoveToRight(Node *tl, Node *tr, Node *tp, int ki);
private:

  b_plus_off RootPos() { return cache_.get()->header_.root_pos; }

  Node *GetNode(b_plus_off pos) { return cache_.get()->Get(pos); }

  static Key GetKey(const char *entry_c_str) {
    Key k;
    memcpy(&k, entry_c_str, sizeof(k));
    return k;
  }

  int LeafEntryNum(Node *t) { return t->entrys.size() / kEntrySize_; }

private:
  std::unique_ptr<NodeCache<Key>> cache_;
  std::vector<b_plus_off> stack_parent_pos_;  // 保存父结点位置，对插入等需回溯的操作提供辅助
  std::vector<int> stack_index_in_parent_;  // 保存当前结点在父结点中位置
  const int kEntrySize_;

  // 叶结点、内部结点阶数，其他变量用于简化编程
  int kLeafOrderNum_;
  int kLeafEntryLeast_;
  int kLeafEntryMost_;
  int kInterOrderNum_;
  int kInterKeyLeast_;
  int kInterKeyMost_;

public:
  // 最大返回记录数，默认10000
  static int kMaxResultNum;
  static const std::string kResultTooManyMsg;

  // 类似迭代器，每次返回1条记录，以提供遍历操作
  class EntryIterator {
    b_plus_off pos_;
    int cur_;
    BPlusTree<Key> *tree_;
  public:
    explicit EntryIterator(BPlusTree<Key> *tree) : pos_(kHeaderAlign), cur_(0), tree_(tree) {
      if (tree == nullptr) PrintExit("EntryIterator error");
      if (tree->IsEmpty()) pos_ = 0;
    }

    bool IsEnd() {
      return pos_ == 0;
    }

    EntryIterator &operator++() {
      if (IsEnd()) PrintExit("EntryIterator error");
      Node *t = tree_->GetNode(pos_);
      ++cur_;
      if (cur_ >= tree_->LeafEntryNum(t)) {
        cur_ = 0;
        pos_ = t->next;
      }
      return *this;
    }

    std::string operator*() {
      if (IsEnd()) PrintExit("EntryIterator error");
      Node *t = tree_->GetNode(pos_);
      const char *entry = t->entrys.c_str() + tree_->kEntrySize_ * cur_;
      return std::string(entry, tree_->kEntrySize_);
    }

  };
};

template<typename Key>
int BPlusTree<Key>::kMaxResultNum = 10000;

template<typename Key>
const std::string BPlusTree<Key>::kResultTooManyMsg = "too many lines, limit in " + std::to_string(kMaxResultNum);

template<typename Key>
BPlusTree<Key>::BPlusTree(const std::string &tree_file_path, int entry_size)
    : kEntrySize_(entry_size) {
  // 检查记录大小
  if (entry_size > kMaxEntrySize)
    PrintExit("记录大小不应超过" + std::to_string(kMaxEntrySize));
  // 创建结点缓存池
  cache_ = std::make_unique<NodeCache<Key>>(tree_file_path, entry_size);
  Node *node = nullptr;
  kLeafOrderNum_ = (kBlockSize - sizeof(node->type) - sizeof(node->next) - sizeof(typename Node::NumType)) /
                   kEntrySize_ + 1;
  kLeafEntryLeast_ = kLeafOrderNum_ / 2;
  kLeafEntryMost_ = kLeafOrderNum_ - 1;
  kInterOrderNum_ = (kBlockSize - sizeof(node->type) - sizeof(typename Node::NumType) - sizeof(b_plus_off)) /
                    (sizeof(Key) + sizeof(b_plus_off)) + 1;
  kInterKeyLeast_ = kInterOrderNum_ / 2 - 1;
  kInterKeyMost_ = kInterOrderNum_ - 1;
}

template<typename Key>
int BPlusTree<Key>::Insert(const std::string &entry) {
  Key key = GetKey(entry.c_str());
  Node *t = DownToLeaf(key);
  // 叶结点中查找
  bool found;
  int i = SearchInLeaf(key, t, found);
  if (found) return -1;  // 已存在 不允许重复值
  InsertLeaf(t, i, entry);
  return 0;
}

template<typename Key>
typename BPlusTree<Key>::Node *BPlusTree<Key>::DownToLeaf(const Key &key) {
  stack_parent_pos_.clear();
  stack_index_in_parent_.clear();
  Node *t = GetNode(RootPos());
  while (t->type == kInternalNodeType) {
    int i = 0;
    while (i < t->keys.size() && key >= t->keys[i]) ++i;
    stack_parent_pos_.push_back(t->self_pos);
    stack_index_in_parent_.push_back(i);
    t = GetNode(t->children[i]);
  }
  return t;
}

template<typename Key>
void BPlusTree<Key>::InsertLeaf(BPlusTree::Node *t, int insert_pos,
                                const std::string &entry) {
  t->dirty = true;
  t->entrys.insert(insert_pos * kEntrySize_, entry);
  if (LeafEntryNum(t) <= kLeafEntryMost_) return;  // 未满直接返回
  // 根结点满
  if (t->self_pos == RootPos()) {
    Split(t);
    return;
  }
  // 先尝试转移给兄弟
  auto tp_pos = stack_parent_pos_.back();
  stack_parent_pos_.pop_back();
  Node *tp = GetNode(tp_pos);
  int ti = stack_index_in_parent_.back();
  stack_index_in_parent_.pop_back();
  Node *tl = ti == 0 ? nullptr : GetNode(tp->children[ti - 1]);
  if (tl != nullptr && LeafEntryNum(tl) < kLeafEntryMost_) {  // 移入左兄弟
    MoveToLeft(tl, t, tp, ti - 1);
    return;
  }
  Node *tr = ti == tp->keys.size() ? nullptr : GetNode(tp->children[ti + 1]);
  if (tr != nullptr && LeafEntryNum(tr) < kLeafEntryMost_) {  // 移入右兄弟
    MoveToRight(t, tr, tp, ti);
    return;
  }
  // 无法移动，只能分裂
  stack_parent_pos_.push_back(tp_pos);
  stack_index_in_parent_.push_back(ti);
  Split(t);
}

template<typename Key>
int
BPlusTree<Key>::SearchInLeaf(const Key &key, BPlusTree::Node *t, bool &found) {
  static auto leaf_key_greater = [](const Key &key, const char *entry_c_str) {
    return key > GetKey(entry_c_str);
  };
  static auto leaf_key_equal = [](const Key &key, const char *entry_c_str) {
    return key == GetKey(entry_c_str);
  };
  int i = 0, n = LeafEntryNum(t);
  const char *entry = t->entrys.c_str();
  while (i < n && leaf_key_greater(key, entry)) {
    ++i;
    entry += kEntrySize_;
  }
  found = false;
  if (i < n && leaf_key_equal(key, entry)) found = true;
  return i;
}

template<typename Key>
void BPlusTree<Key>::Split(BPlusTree::Node *t) {
  Node *nt = cache_.get()->Alloc();
  Key nk;
  if (t->type == kInternalNodeType) {
    nt->type = kInternalNodeType;
    nk = t->keys[kInterOrderNum_ / 2];
    nt->keys.insert(nt->keys.begin(), t->keys.begin() + kInterOrderNum_ / 2 + 1, t->keys.end());
    nt->children.insert(nt->children.begin(), t->children.begin() + kInterOrderNum_ / 2 + 1, t->children.end());
    t->keys.erase(t->keys.begin() + kInterOrderNum_ / 2, t->keys.end());
    t->children.erase(t->children.begin() + kInterOrderNum_ / 2 + 1, t->children.end());
  } else {
    nt->type = kLeafNodeType;
    nt->entrys.append(t->entrys.begin() + (kLeafOrderNum_ / 2 * kEntrySize_), t->entrys.end());
    t->entrys.erase(t->entrys.begin() + (kLeafOrderNum_ / 2 * kEntrySize_), t->entrys.end());
    nt->next = t->next;
    t->next = nt->self_pos;
    nk = GetKey(nt->entrys.c_str());
  }
  // 根结点分裂
  if (t->self_pos == RootPos()) {
    Node *root = cache_.get()->AllocRoot();
    root->type = kInternalNodeType;
    root->keys.push_back(nk);
    root->children.push_back(t->self_pos);
    root->children.push_back(nt->self_pos);
    return;
  }
  // 向父结点插入
  auto nt_pos = nt->self_pos;
  Node *tp = GetNode(stack_parent_pos_.back());
  stack_parent_pos_.pop_back();
  int ti = stack_index_in_parent_.back();
  stack_index_in_parent_.pop_back();
  InsertInternal(tp, ti, nk, nt_pos);
}

template<typename Key>
void
BPlusTree<Key>::MoveToLeft(BPlusTree::Node *tl, BPlusTree::Node *tr,
                           BPlusTree::Node *tp, int ki) {
  tl->dirty = true;
  tr->dirty = true;
  tp->dirty = true;
  if (tl->type == kInternalNodeType) {
    tl->keys.push_back(tp->keys[ki]);
    tl->children.push_back(tr->children.front());
    tp->keys[ki] = tr->keys.front();
    tr->keys.erase(tr->keys.begin());
    tr->children.erase(tr->children.begin());
  } else {
    tl->entrys.append(tr->entrys.begin(), tr->entrys.begin() + kEntrySize_);
    tr->entrys.erase(tr->entrys.begin(), tr->entrys.begin() + kEntrySize_);
    tp->keys[ki] = GetKey(tr->entrys.c_str());
  }
}

template<typename Key>
void
BPlusTree<Key>::MoveToRight(BPlusTree::Node *tl, BPlusTree::Node *tr,
                            BPlusTree::Node *tp, int ki) {
  tl->dirty = true;
  tr->dirty = true;
  tp->dirty = true;
  if (tl->type == kInternalNodeType) {
    tr->keys.insert(tr->keys.begin(), tp->keys[ki]);
    tr->children.insert(tr->children.begin(), tl->children.back());
    tp->keys[ki] = tl->keys.back();
    tl->keys.pop_back();
    tl->children.pop_back();
  } else {
    auto tl_last_entry_begin = tl->entrys.begin() + tl->entrys.size() - kEntrySize_;
    tr->entrys.insert(tr->entrys.begin(), tl_last_entry_begin, tl->entrys.end());
    tl->entrys.erase(tl_last_entry_begin, tl->entrys.end());
    tp->keys[ki] = GetKey(tr->entrys.c_str());
  }
}

template<typename Key>
void BPlusTree<Key>::InsertInternal(BPlusTree::Node *t, int insert_pos,
                                    const Key &nk,
                                    b_plus_off nt_pos) {
  t->dirty = true;
  t->keys.insert(t->keys.begin() + insert_pos, nk);
  t->children.insert(t->children.begin() + insert_pos + 1, nt_pos);
  if (t->keys.size() <= kInterKeyMost_) return;  // 未满直接返回
  // 根结点满
  if (t->self_pos == RootPos()) {
    Split(t);
    return;
  }
  // 先尝试转移给兄弟
  auto tp_pos = stack_parent_pos_.back();
  stack_parent_pos_.pop_back();
  Node *tp = GetNode(tp_pos);
  int ti = stack_index_in_parent_.back();
  stack_index_in_parent_.pop_back();
  Node *tl = ti == 0 ? nullptr : GetNode(tp->children[ti - 1]);
  if (tl != nullptr && tl->keys.size() < kInterKeyMost_) {  // 移入左兄弟
    MoveToLeft(tl, t, tp, ti - 1);
    return;
  }
  Node *tr = ti == tp->keys.size() ? nullptr : GetNode(tp->children[ti + 1]);
  if (tr != nullptr && tr->keys.size() < kInterKeyMost_) {  // 移入右兄弟
    MoveToRight(t, tr, tp, ti);
    return;
  }
  // 无法移动，只能分裂
  stack_parent_pos_.push_back(tp_pos);
  stack_index_in_parent_.push_back(ti);
  Split(t);
}

template<typename Key>
void BPlusTree<Key>::Flush() {
  cache_.get()->Flush();
}

template<typename Key>
int BPlusTree<Key>::Search(const Key &key, std::string &entry_found) {
  // 定位叶结点
  Node *t = DownToLeaf(key);
  // 叶结点中查找
  bool found;
  int i = SearchInLeaf(key, t, found);
  if (!found) return -1;  // 不存在
  entry_found = std::string(t->entrys.data() + i * kEntrySize_, kEntrySize_);
  return 0;
}

template<typename Key>
std::vector<std::string>
BPlusTree<Key>::SearchRange(const Key &L, const Key &R) {
  if (L > R) return {};
  std::vector<std::string> entrys_found;
  // 定位叶结点
  Node *t = DownToLeaf(L);
  // 叶结点中查找，获得大于等于L的首条记录
  bool found;
  int i = SearchInLeaf(L, t, found);
  // 范围查询
  while (true) {
    // 在当前结点中遍历
    int n = LeafEntryNum(t);
    char *entry = t->entrys.data() + i * kEntrySize_;
    for (; i < n; ++i, entry += kEntrySize_) {
      auto entry_key = *(Key *) entry;
      if (entry_key <= R) {
        entrys_found.push_back(std::string(entry, kEntrySize_));
        if (entrys_found.size() >= kMaxResultNum) {
          std::cout << kResultTooManyMsg << std::endl;
          return std::move(entrys_found);
        }
      } else {
        return std::move(entrys_found);
      }
    }
    // 跳跃至下一个结点
    if (t->next == 0) return std::move(entrys_found);
    t = GetNode(t->next);
    i = 0;
  }
}

template<typename Key>
bool BPlusTree<Key>::IsEmpty() {
  Node *root = GetNode(RootPos());
  return root->type == kLeafNodeType && root->entrys.empty();
}

template<typename Key>
int BPlusTree<Key>::Remove(const Key &key) {
  // 定位叶结点
  Node *t = DownToLeaf(key);
  // 叶结点中查找
  bool found;
  int i = SearchInLeaf(key, t, found);
  if (!found) return -1;  // 不存在
  RemoveEntry(t, i);
}

template<typename Key>
void BPlusTree<Key>::RemoveEntry(BPlusTree::Node *t, int remove_pos) {
  t->dirty = true;
  if (t->type == kInternalNodeType) {
    t->keys.erase(t->keys.begin() + remove_pos);
    b_plus_off need_free = t->children[remove_pos + 1];
    t->children.erase(t->children.begin() + remove_pos + 1);
    cache_.get()->Free(need_free);
    if (t->keys.size() >= kInterKeyLeast_) return;
  } else {
    t->entrys.erase(remove_pos * kEntrySize_, kEntrySize_);
    if (LeafEntryNum(t) >= kLeafEntryLeast_) return;
  }
  // 根结点过空
  if (t->self_pos == RootPos()) {
    // 根结点是内部结点，且无key，才需调整
    if (t->type == kInternalNodeType && t->keys.empty()) {
      cache_.get()->FreeRoot();
    }
    return;
  }
  // 非根结点过空
  Node *tp = GetNode(stack_parent_pos_.back());
  stack_parent_pos_.pop_back();
  int ti = stack_index_in_parent_.back();
  stack_index_in_parent_.pop_back();
  Node *tl = ti == 0 ? nullptr : GetNode(tp->children[ti - 1]);
  if (tl != nullptr && CouldBorrow(tl)) {  // 找左兄弟借数据
    MoveToRight(tl, t, tp, ti - 1);
    return;
  }
  Node *tr = ti == tp->keys.size() ? nullptr : GetNode(tp->children[ti + 1]);
  if (tr != nullptr && CouldBorrow(tr)) {  // 找右兄弟借数据
    MoveToLeft(t, tr, tp, ti);
    return;
  }
  // 无法借数据，只能合并结点
  int merge_i = ti == 0 ? ti : ti - 1;  // 左兄弟存在则合并至左兄弟，否则合并至当前结点
  Node *merge_l = GetNode(tp->children[merge_i]);
  Node *merge_r = GetNode(tp->children[merge_i + 1]);
  merge_l->dirty = true;
  if (t->type == kInternalNodeType) {
    merge_l->keys.push_back(tp->keys[merge_i]);
    merge_l->keys.insert(merge_l->keys.end(), merge_r->keys.begin(), merge_r->keys.end());
    merge_l->children.insert(merge_l->children.end(), merge_r->children.begin(), merge_r->children.end());
  } else {
    merge_l->entrys.append(merge_r->entrys);
    merge_l->next = merge_r->next;
  }
  RemoveEntry(tp, merge_i);
}

template<typename Key>
bool BPlusTree<Key>::CouldBorrow(BPlusTree::Node *t) {
  if (t->type == kInternalNodeType) {
    return t->keys.size() > kInterKeyLeast_;
  } else {
    return LeafEntryNum(t) > kLeafEntryLeast_;
  }
}

}  // namespace simple

#endif //BP_TREE_DISK_B_PLUS_TREE_H

#include "b_plus_tree.h"

namespace bplus {

BPlusTree::BPlusTree(const std::string &path,
                     bool (*key_equal)(const char *, const char *),
                     bool (*key_less)(const char *, const char *),
                     int key_size, int entry_size) : KeyEqual(key_equal),
                                                     KeyLess(key_less) {
  // 参数检查
  if (key_size > BP::kMaxKeySize)
    PrintExit("max key size: " + std::to_string(BP::kMaxKeySize));
  if (entry_size > BP::kMaxEntrySize)
    PrintExit("max entry size: " + std::to_string(BP::kMaxEntrySize));
  BP::kKeySize = key_size;
  BP::kEntrySize = entry_size;
  if (RegExists(path)) PrintExit(path + " already exists");
  // 创建b+树
  cache_ = std::make_unique<Cache>(path, true);
  CalculateOrder();
}

BPlusTree::BPlusTree(const std::string &path,
                     bool (*key_equal)(const char *, const char *),
                     bool (*key_less)(const char *, const char *)) : KeyEqual(key_equal),
                                                                     KeyLess(key_less) {
  // 参数检查
  if (!RegExists(path)) PrintExit(path + " not exists");
  // 加载b+树
  cache_ = std::make_unique<Cache>(path, false);
  CalculateOrder();
}

void BPlusTree::Flush() {
  cache_->Flush();
}

bool BPlusTree::Empty() {
  Node *root = Get(Root());
  return root->type == BP::kLeaf && root->entries.empty();
}

void BPlusTree::CalculateOrder() {
  Node *node = nullptr;
  Node::NumType num = 0;

  int inter_space = BP::kBlockSize - sizeof(node->type) - sizeof(num) - sizeof(bp_pos);
  kInterOrder_ = inter_space / (BP::kKeySize + sizeof(bp_pos)) + 1;
  kInterLeast_ = kInterOrder_ / 2 - 1;
  kInterMost_ = kInterOrder_ - 1;

  int leaf_space = BP::kBlockSize - sizeof(node->type) - sizeof(node->next) - sizeof(num);
  kLeafOrder_ = leaf_space / BP::kEntrySize + 1;
  kLeafLeast_ = kLeafOrder_ / 2;
  kLeafMost_ = kLeafOrder_ - 1;
}

int BPlusTree::Insert(const char *entry) {
  // 至叶结点
  const char *key = entry;
  Node *t = ToLeaf(key);
  // 叶结点中查找
  int i = SearchLeaf(key, t);
  if (i != -1) return -1;  // 已存在 不允许重复值
  InsertLeaf(t, i, entry);
  return 0;
}

BPlusTree::Node *BPlusTree::ToLeaf(const char *key) {
  pstack_.clear();
  istack_.clear();
  Node *t = Get(Root());
  while (t->type == BP::kInternal) {
    int i = 0;
    while (i < t->KNum() && !(KeyLess(key, t->KI(i)))) ++i;
    pstack_.push_back(t->pos);
    istack_.push_back(i);
    t = Get(t->children[i]);
  }
  return t;
}

int BPlusTree::SearchLeaf(const char *key, BPlusTree::Node *t) {
  int i = 0, n = t->ENum();
  const char *entry = t->entries.c_str();
  while (i < n && !KeyLess(key, entry) && !KeyEqual(key, entry)) {
    ++i;
    entry += BP::kEntrySize;
  }
  if (i < n && KeyEqual(key, entry)) return i;
  else return -1;
}

void BPlusTree::InsertLeaf(BPlusTree::Node *t, int insert_idx,
                           const char *entry) {
  t->dirty = true;
  t->EInsert(insert_idx, entry);
  if (t->ENum() <= kLeafMost_) return;  // 未满直接返回
  // 结点溢出，需处理
  HandleFull(t);
}

void BPlusTree::InsertInternal(BPlusTree::Node *t, int insert_idx,
                               const char *nkey, bp_pos npos) {
  t->dirty = true;
  t->KInsert(insert_idx, nkey);
  t->children.insert(t->children.begin() + insert_idx + 1, npos);
  if (t->KNum() <= kInterMost_) return;
  // 结点溢出，需处理
  HandleFull(t);
}

void BPlusTree::HandleFull(BPlusTree::Node *t) {
  // 根结点满
  if (t->pos == Root()) {
    Split(t);
    return;
  }
  // 先尝试转移给兄弟
  Node *tp = Get(pstack_.back());
  int ti = istack_.back();
  Node *tl = ti == 0 ? nullptr : Get(tp->children[ti - 1]);
  if (MoveAble(tl)) {  // 移入左兄弟
    MoveToLeft(tl, t, tp, ti - 1);
    return;
  }
  Node *tr = ti == tp->KNum() ? nullptr : Get(tp->children[ti + 1]);
  if (MoveAble(tr)) {  // 移入右兄弟
    MoveToRight(t, tr, tp, ti);
    return;
  }
  // 无法移动，只能分裂
  Split(t);
}

bool BPlusTree::MoveAble(BPlusTree::Node *t) {
  if (t == nullptr) return false;
  if (t->type == BP::kInternal) {
    return t->KNum() < kInterMost_;
  } else {
    return t->ENum() < kLeafMost_;
  }
}

void BPlusTree::Split(BPlusTree::Node *t) {
  // 向右分裂产生新key、新结点
  Node *nt = cache_->Alloc();
  nt->type = t->type;
  std::string nkey;
  if (t->type == BP::kInternal) {
    nkey = SplitInternal(t, nt);
  } else {
    nkey = SplitLeaf(t, nt);
  }
  // 根结点分裂，需额外处理
  if (t->pos == Root()) {
    Node *root = cache_->AllocRoot();
    root->type = BP::kInternal;
    root->KInsert(0, nkey.c_str());
    root->children.push_back(t->pos);
    root->children.push_back(nt->pos);
    return;
  }
  // 向父结点插入
  auto npos = nt->pos;
  Node *tp = Get(pstack_.back());
  pstack_.pop_back();
  int ti = istack_.back();
  istack_.pop_back();
  InsertInternal(tp, ti, nkey.c_str(), npos);
}

std::string BPlusTree::SplitInternal(Node *t, Node *nt) {
  // 中间key作为新key
  std::string nkey = std::string(t->KI(kInterOrder_ / 2), BP::kKeySize);
  // 后半部分key，除新key外，其他key推入新结点
  nt->KInsert(0, t->KI(kInterOrder_ / 2 + 1),
              t->KNum() - kInterOrder_ / 2 - 1);
  // 原结点擦除key
  t->KErase(kInterOrder_ / 2,
            t->KNum() - kInterOrder_ / 2);

  nt->children.insert(nt->children.begin(),
                      t->children.begin() + kInterOrder_ / 2 + 1,
                      t->children.end());
  t->children.erase(t->children.begin() + kInterOrder_ / 2 + 1,
                    t->children.end());
  return std::move(nkey);
}

std::string BPlusTree::SplitLeaf(BPlusTree::Node *t, BPlusTree::Node *nt) {
  // 分一半entry给新结点即可
  nt->EInsert(0, t->EI(kLeafOrder_ / 2),
              t->ENum() - kLeafOrder_ / 2);
  t->EErase(kLeafOrder_ / 2, t->ENum() - kLeafOrder_ / 2);
  // 修改叶结点链表
  nt->next = t->next;
  t->next = nt->pos;
  return std::string(t->EI(0), BP::kEntrySize);
}

void BPlusTree::MoveToLeft(BPlusTree::Node *tl, BPlusTree::Node *tr, BPlusTree::Node *tp, int ki) {
  tl->dirty = true;
  tr->dirty = true;
  tp->dirty = true;
  if (tl->type == BP::kInternal) {
    // 父结点ki处key，移入左结点末尾
    tl->KInsert(tl->KNum(), tp->KI(ki));
    tl->children.push_back(tr->children.front());
    // 父结点ki处key，用右结点头部key替换
    tp->KReplace(ki, tr->KI(0));
    tr->KErase(0);
    tr->children.erase(tr->children.begin());
  } else {
    // 移动entry即可
    tl->EInsert(tl->ENum(), tr->EI(0));
    tr->EErase(0);
    // 父结点ki处key，用右结点头部key替换
    tp->KReplace(ki, tr->EI(0));
  }
}

void BPlusTree::MoveToRight(BPlusTree::Node *tl, BPlusTree::Node *tr, BPlusTree::Node *tp, int ki) {
  tl->dirty = true;
  tr->dirty = true;
  tp->dirty = true;
  if (tl->type == BP::kInternal) {
    // 父结点ki处key，移入右结点头部
    tr->KInsert(0, tp->KI(ki));
    tr->children.insert(tr->children.begin(), tl->children.back());
    // 父结点ki处key，用左结点尾部key替换
    tp->KReplace(ki, tl->KI(tl->KNum() - 1));
    tl->KErase(tl->KNum() - 1);
    tl->children.pop_back();
  } else {
    // 移动entry即可
    tr->EInsert(0, tl->EI(tl->ENum() - 1));
    tl->EErase(tl->ENum() - 1);
    // 父结点ki处key，用右结点头部key替换
    tp->KReplace(ki, tr->EI(0));
  }
}

int BPlusTree::Remove(const char *key) {
  Node *t = ToLeaf(key);
  // 叶结点中查找
  int i = SearchLeaf(key, t);
  if (i == -1) return -1;  // 不存在
  RemoveEntry(t, i);
}

//void BPlusTree::RemoveEntry(BPlusTree::Node *t, int rpos) {
//  t->dirty = true;
//  if (t->type == BP::kInternal) {
//    t->keys.erase(t->keys.begin() + rpos);
//    bp_pos need_free = t->children[rpos + 1];
//    t->children.erase(t->children.begin() + rpos + 1);
//    cache_.get()->Free(need_free);
//    if (t->keys.size() >= kInterLeast_) return;
//  } else {
//    t->entries.erase(rpos * entry_size_, entry_size_);
//    if (LeafEntryNum(t) >= kLeafLeast_) return;
//  }
//  // 根结点过空
//  if (t->pos == RootPos()) {
//    // 根结点是内部结点，且无key，才需调整
//    if (t->type == BP::kInternal && t->keys.empty()) {
//      cache_.get()->FreeRoot();
//    }
//    return;
//  }
//  // 非根结点过空
//  Node *tp = Get(pstack_.back());
//  pstack_.pop_back();
//  int ti = istack_.back();
//  istack_.pop_back();
//  Node *tl = ti == 0 ? nullptr : Get(tp->children[ti - 1]);
//  if (BorrowAble(tl)) {  // 找左兄弟借数据
//    MoveToRight(tl, t, tp, ti - 1);
//    return;
//  }
//  Node *tr = ti == tp->keys.size() ? nullptr : Get(tp->children[ti + 1]);
//  if (BorrowAble(tr)) {  // 找右兄弟借数据
//    MoveToLeft(t, tr, tp, ti);
//    return;
//  }
//  // 无法借数据，只能合并结点
//  int merge_i = ti == 0 ? ti : ti - 1;  // 左兄弟存在则合并至左兄弟，否则合并至当前结点
//  Node *merge_l = Get(tp->children[merge_i]);
//  Node *merge_r = Get(tp->children[merge_i + 1]);
//  merge_l->dirty = true;
//  if (t->type == BP::kInternal) {
//    merge_l->keys.push_back(tp->keys[merge_i]);
//    merge_l->keys.insert(merge_l->keys.end(), merge_r->keys.begin(), merge_r->keys.end());
//    merge_l->children.insert(merge_l->children.end(), merge_r->children.begin(), merge_r->children.end());
//  } else {
//    merge_l->entries.append(merge_r->entries);
//    merge_l->next = merge_r->next;
//  }
//  RemoveEntry(tp, merge_i);
//}
//
//bool BPlusTree::BorrowAble(BPlusTree::Node *t) {
//  if (t == nullptr) return false;
//  if (t->type == BP::kInternal) {
//    return t->KNum() > kInterLeast_;
//  } else {
//    return t->ENum() > kLeafLeast_;
//  }
//}

}  // namespace bplus
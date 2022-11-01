#ifndef BP_TREE_DISK_LRU_CACHE_H
#define BP_TREE_DISK_LRU_CACHE_H

#include "b_plus_struct.h"
#include <unordered_map>

namespace bplus {

class LRU {
  struct LinkNode {
    LinkNode *prev = nullptr;
    LinkNode *next = nullptr;
    bp_pos pos = 0;
    BPlusNode *bnode = nullptr;

    ~LinkNode() { delete bnode; }
  };

  using BNode = BPlusNode;
  using LNode = LinkNode;

  LNode *head_;
  LNode *tail_;
  std::unordered_map<bp_pos, LNode *> map_;
  const int capacity_;
public:
  explicit LRU(int capacity) : capacity_(capacity), head_(new LNode),
                               tail_(new LNode) {
    head_->next = tail_;
    tail_->prev = head_;
  }

  ~LRU() {
    delete head_;
    delete tail_;
    for (auto &pair : map_) delete pair.second;
  }

  BNode *get(bp_pos pos) {
    if (!map_.count(pos)) return nullptr;
    moveToHead(map_.at(pos));
    return map_.at(pos)->bnode;
  }

  BNode *put(bp_pos pos, BNode *bnode) {
    if (capacity_ <= 0) return nullptr;
    if (map_.count(pos)) PrintExit("lru错误，put冲突");

    auto link_node = new LNode;
    link_node->pos = pos;
    link_node->bnode = bnode;
    addToHead(link_node);
    map_.insert({pos, link_node});  // 多线程下，operator[]插入失败，insert插入成功？

    if (map_.size() <= capacity_) return nullptr;
    // 缓存溢出
    auto tail = removeTail();
    map_.erase(tail->pos);
    auto ret = tail->bnode;
    tail->bnode = nullptr;
    delete tail;
    return ret;
  }

  void Flush(std::fstream &f) {
    for (auto &pair : map_) {
      BNode *bnode = pair.second->bnode;
      if (bnode->dirty) bnode->Write(f);
    }
  }

private:

  void addToHead(LNode *n) {
    head_->next->prev = n;
    n->next = head_->next;
    n->prev = head_;
    head_->next = n;
  }

  void removeNode(LNode *n) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
  }

  void moveToHead(LNode *n) {
    if (n == head_->next) return;
    removeNode(n);
    addToHead(n);
  }

  LNode *removeTail() {
    auto n = tail_->prev;
    removeNode(n);
    return n;
  }
};

}  // namespace bplus

#endif //BP_TREE_DISK_LRU_CACHE_H

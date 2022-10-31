#ifndef BP_TREE_DISK_LRU_CACHE_H
#define BP_TREE_DISK_LRU_CACHE_H

#include "tree_file_struct.h"
#include <unordered_map>

namespace simple {

template<typename BPlusNodeKey>
struct LinkedNode {
  LinkedNode *prev = nullptr;
  LinkedNode *next = nullptr;
  b_plus_off pos = 0;
  BPlusNode <BPlusNodeKey> *b_plus_node = nullptr;

  ~LinkedNode() { if (b_plus_node != nullptr) delete b_plus_node; }
};

template<typename BPlusNodeKey>
class LRUCache {
  using LinkNode = LinkedNode<BPlusNodeKey>;
  using BPNode = BPlusNode<BPlusNodeKey>;
  LinkNode *head_;
  LinkNode *tail_;
  const int capacity_;
public:
  std::unordered_map<b_plus_off, LinkNode *> link_node_map_;
public:
  explicit LRUCache(int capacity) : capacity_(capacity), head_(new LinkNode()),
                                    tail_(new LinkNode()) {
    head_->next = tail_;
    tail_->prev = head_;
  }

  ~LRUCache() {
    delete head_;
    delete tail_;
    for (auto &pair : link_node_map_) delete pair.second;
  }

  BPNode *get(b_plus_off pos) {
    if (!link_node_map_.count(pos)) return nullptr;
    moveToHead(link_node_map_.at(pos));
    return link_node_map_.at(pos)->b_plus_node;
  }

  LinkNode *put(b_plus_off pos, BPNode *b_plus_node) {
    if (capacity_ <= 0) {
      return nullptr;
    }
    if (link_node_map_.count(pos)) PrintExit("lru错误，put冲突");
    auto link_node = new LinkNode;
    link_node->pos = pos;
    link_node->b_plus_node = b_plus_node;
    addToHead(link_node);
    // 匪夷所思，operator[]插入失败，insert插入成功
//    link_node_map_[pos] = link_node;
    link_node_map_.insert({pos, link_node});
    if (link_node_map_.size() <= capacity_) return nullptr;
    // 缓存溢出
    auto tail_link_node = removeTail();
    link_node_map_.erase(tail_link_node->pos);
    return tail_link_node;
  }

  void Flush(std::fstream &file, const int entry_size) {
    for (auto &pair : link_node_map_) {
      auto b_plus_node = pair.second->b_plus_node;
      if (b_plus_node->dirty) b_plus_node->WriteToDisk(file, entry_size);
    }
  }

private:

  void addToHead(LinkNode *n) {
    head_->next->prev = n;
    n->next = head_->next;
    n->prev = head_;
    head_->next = n;
  }

  void removeNode(LinkNode *n) {
    n->prev->next = n->next;
    n->next->prev = n->prev;
  }

  void moveToHead(LinkNode *n) {
    if (n == head_->next) return;
    removeNode(n);
    addToHead(n);
  }

  LinkNode *removeTail() {
    auto n = tail_->prev;
    removeNode(n);
    return n;
  }
};

}

#endif //BP_TREE_DISK_LRU_CACHE_H

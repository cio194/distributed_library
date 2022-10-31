#ifndef BP_TREE_DISK_NODE_CACHE_H
#define BP_TREE_DISK_NODE_CACHE_H

#include "tree_file_struct.h"
#include "utils/simple_utils.h"
#include "utils/custom_exceptions.h"
#include "lru_cache.h"
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <fstream>

namespace simple {

// 作为b+树的 内存<->磁盘 间的纽带
template<typename Key>
class NodeCache {
  using Node = BPlusNode<Key>;
public:
  explicit NodeCache(const std::string &tree_file_path, int entry_size);
  ~NodeCache();
  void Flush();
  Node *AllocRoot();
  Node *Alloc();
  Node *Get(b_plus_off pos);
  void Put(Node *node);
  void FreeRoot();
  void Free(b_plus_off pos);
private:
  bool CheckTreeFileExists();
  void CreateBPlusTree();
  void LoadBPlusTree();
private:
  const std::string tree_file_path_;
  const int kEntrySize_;
  bool write_when_destruct_;

  std::fstream tree_file_;
  LRUCache<Key> lru_cache_;
public:
  BPlusHeader<Key> header_;
};

template<typename Key>
NodeCache<Key>::NodeCache(const std::string &tree_file_path, int entry_size)
    : tree_file_path_(tree_file_path), kEntrySize_(entry_size),
      write_when_destruct_(false), lru_cache_(kCacheNum) {
  // 树文件存在则为加载，否则为新建
  if (CheckTreeFileExists()) {
    LoadBPlusTree();
  } else {
    CreateBPlusTree();
  }
  write_when_destruct_ = true;
}

template<typename Key>
bool NodeCache<Key>::CheckTreeFileExists() {
  if (access(tree_file_path_.c_str(), F_OK) != 0) return false;
  struct stat sb;
  if (lstat(tree_file_path_.c_str(), &sb) != 0)
    throw unix_sys_error("lstat " + tree_file_path_);
  if (!S_ISREG(sb.st_mode)) return false;
  return true;
}

template<typename Key>
void NodeCache<Key>::CreateBPlusTree() {
  // 创建文件
  tree_file_.open(tree_file_path_, std::ios_base::in | std::ios_base::out |
                                   std::ios_base::trunc);
  if (!tree_file_) throw unix_sys_error("open " + tree_file_path_);
  // 初始化b+树文件头，根结点
  header_.InitInMemory(kEntrySize_);
  Node *root = AllocRoot();
  root->type = kLeafNodeType;
  root->next = 0;
}

template<typename Key>
NodeCache<Key>::~NodeCache() {
  if (!write_when_destruct_) return;
  Flush();
}

template<typename Key>
void NodeCache<Key>::Flush() {
  header_.WriteToDisk(tree_file_);
  lru_cache_.Flush(tree_file_, kEntrySize_);
}

template<typename Key>
typename NodeCache<Key>::Node *NodeCache<Key>::AllocRoot() {
  Node *root = Alloc();
  ++header_.height;
  header_.root_pos = root->self_pos;
  return root;
}

template<typename Key>
typename NodeCache<Key>::Node *NodeCache<Key>::Alloc() {
  if (header_.free_node_num > 0) {  // 从空闲链表拿结点
    Node *head_free_node = Get(header_.free_head_pos);
    head_free_node->dirty = true;
    ++header_.node_num;
    --header_.free_node_num;
    header_.free_head_pos = head_free_node->next;
    return head_free_node;
  } else {  // 从文件末尾分配
    Node *node = new Node;
    node->self_pos = header_.file_tail_pos;
    node->dirty = true;
    Put(node);
    ++header_.node_num;
    header_.file_tail_pos += kBlockSize;
    return node;
  }
}

template<typename Key>
void NodeCache<Key>::Put(NodeCache::Node *node) {
  auto expired_link_node = lru_cache_.put(node->self_pos, node);
  if (expired_link_node == nullptr) return;
  auto expired_b_plus_node = expired_link_node->b_plus_node;
  if (!expired_b_plus_node->dirty) {
    delete expired_link_node;
    return;
  }
  expired_b_plus_node->WriteToDisk(tree_file_, kEntrySize_);
  delete expired_link_node;
}

template<typename Key>
typename NodeCache<Key>::Node *NodeCache<Key>::Get(b_plus_off pos) {
  Node *node = lru_cache_.get(pos);
  if (node != nullptr) return node;
  // 缓存未命中
  node = new Node;
  node->ReadFromDisk(tree_file_, pos, kEntrySize_);
  Put(node);
  return node;
}

template<typename Key>
void NodeCache<Key>::LoadBPlusTree() {
  // 创建文件
  tree_file_.open(tree_file_path_);
  if (!tree_file_) throw unix_sys_error("open " + tree_file_path_);
  // 读取header
  header_.ReadFromDisk(tree_file_);
  if (header_.key_size != sizeof(Key)) PrintExit("key size不匹配");
  if (header_.entry_size != kEntrySize_) PrintExit("entry size不匹配");
}

template<typename Key>
void NodeCache<Key>::FreeRoot() {
  auto old_root_pos = header_.root_pos;

  Node *root = Get(header_.root_pos);
  header_.root_pos = root->children[0];
  --header_.height;

  Free(old_root_pos);
}

template<typename Key>
void NodeCache<Key>::Free(b_plus_off pos) {
  Node *node = Get(pos);
  node->type = kFreeNodeType;
  node->keys.clear();
  node->children.clear();
  node->entrys.clear();
  node->next = header_.free_head_pos;
  node->dirty = true;
  header_.free_head_pos = pos;
  ++header_.free_node_num;
  --header_.node_num;
}

}  // namespace simple

#endif //BP_TREE_DISK_NODE_CACHE_H

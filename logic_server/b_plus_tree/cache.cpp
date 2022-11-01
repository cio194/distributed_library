#include "cache.h"

namespace bplus {

Cache::Cache(const std::string &path, bool create) : path_(path),
                                                     write_back_(false),
                                                     lru_(BP::kCacheNum) {
  if (create) {
    CreateTree();
  } else {
    LoadTree();
  }
  write_back_ = true;
}

void Cache::CreateTree() {
  // 创建文件
  tree_file_.open(path_, std::ios_base::in | std::ios_base::out |
                         std::ios_base::trunc);
  if (!tree_file_) throw unix_sys_error("open " + path_);
  // 初始化b+树文件头，根结点
  header_.Init();
  Node *root = AllocRoot();
  root->type = BP::kLeaf;
  root->next = 0;
}

void Cache::LoadTree() {
  // 打开文件
  tree_file_.open(path_);
  if (!tree_file_) throw unix_sys_error("open " + path_);
  // 读取header
  header_.Read(tree_file_);
  BP::kKeySize = header_.key_size;
  BP::kEntrySize = header_.entry_size;
}

Cache::~Cache() {
  if (!write_back_) return;
  Flush();
}

void Cache::Flush() {
  header_.Write(tree_file_);
  lru_.Flush(tree_file_);
}

Cache::Node *Cache::Alloc() {
  if (header_.free_node_num > 0) {
    // 从空闲链表拿结点
    Node *head_free_node = Get(header_.free_head_pos);
    head_free_node->dirty = true;
    ++header_.node_num;
    --header_.free_node_num;
    header_.free_head_pos = head_free_node->next;
    return head_free_node;
  } else {
    // 从文件末尾分配
    Node *node = new Node;
    node->pos = header_.file_tail_pos;
    node->dirty = true;
    Put(node);
    ++header_.node_num;
    header_.file_tail_pos += BP::kBlockSize;
    return node;
  }
}

Cache::Node *Cache::Get(bp_pos pos) {
  Node *node = lru_.get(pos);
  if (node != nullptr) return node;
  // 缓存未命中
  node = new Node;
  node->Read(tree_file_, pos);
  Put(node);
  return node;
}

void Cache::Put(Cache::Node *node) {
  Node *expired = lru_.put(node->pos, node);
  if (expired == nullptr) return;
  // 缓存溢出
  if (expired->dirty) expired->Write(tree_file_);
  delete expired;
}

void Cache::Free(bp_pos pos) {
  Node *node = Get(pos);
  node->type = BP::kFree;
  node->keys.clear();
  node->children.clear();
  node->entries.clear();
  node->next = header_.free_head_pos;
  node->dirty = true;
  header_.free_head_pos = pos;
  ++header_.free_node_num;
  --header_.node_num;
}

Cache::Node *Cache::AllocRoot() {
  Node *root = Alloc();
  ++header_.height;
  header_.root_pos = root->pos;
  return root;
}

void Cache::FreeRoot() {
  auto old = header_.root_pos;
  Node *old_root = Get(header_.root_pos);
  header_.root_pos = old_root->children[0];
  --header_.height;

  Free(old);
}


}  // namespace bplus

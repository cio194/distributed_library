#ifndef BP_TREE_DISK_NODE_CACHE_H
#define BP_TREE_DISK_NODE_CACHE_H

#include "utils/simple_utils.h"
#include "utils/custom_exceptions.h"
#include "b_plus_struct.h"
#include "lru.h"
#include <unistd.h>
#include <sys/stat.h>
#include <string>
#include <fstream>

namespace bplus {

// 结点缓存
class Cache {
  using Node = BPlusNode;
public:
  explicit Cache(const std::string &path, bool create);
  ~Cache();
  void Flush();

  Node *Alloc();
  Node *Get(bp_pos pos);
  void Put(Node *node);
  void Free(bp_pos pos);

  Node *AllocRoot();
  void FreeRoot();

private:
  void CreateTree();
  void LoadTree();
private:
  const std::string path_;
  bool write_back_;
  std::fstream tree_file_;
  LRU lru_;
public:
  BPlusHeader header_;
};

}  // namespace bplus

#endif //BP_TREE_DISK_NODE_CACHE_H

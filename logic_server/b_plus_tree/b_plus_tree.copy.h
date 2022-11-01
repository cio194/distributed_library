#ifndef BP_TREE_DISK_B_PLUS_TREE_H
#define BP_TREE_DISK_B_PLUS_TREE_H

#include "cache.h"
#include "b_plus_struct.h"
#include <string>
#include <memory>
#include <stack>

namespace lalala {

template<typename Key>
std::vector<std::string>
BPlusTree::SearchRange(const Key &L, const Key &R) {
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
    char *entry = t->entries.data() + i * entry_size_;
    for (; i < n; ++i, entry += entry_size_) {
      auto entry_key = *(Key *) entry;
      if (entry_key <= R) {
        entrys_found.push_back(std::string(entry, entry_size_));
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
    t = Get(t->next);
    i = 0;
  }
}


}  // namespace bplus

#endif //BP_TREE_DISK_B_PLUS_TREE_H

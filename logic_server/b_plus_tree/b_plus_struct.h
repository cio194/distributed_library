#ifndef BP_TREE_DISK_TREE_FILE_STRUCT_H
#define BP_TREE_DISK_TREE_FILE_STRUCT_H

#include "utils/custom_exceptions.h"
#include "utils/simple_utils.h"
#include <ios>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

namespace bplus {

using bp_pos = std::streamoff;

// b+树相关定义
struct BP {
  static constexpr int kHeaderAlign = 4096;
  static constexpr int kBlockSize = 16 * 1024;
  static constexpr int kMaxKeySize = 512;
  static constexpr int kMaxEntrySize = 4096;
  static constexpr int kCacheNum = 64;
  static constexpr int kMaxReturn = 10;
  // 结点类型
  static constexpr uint32_t kInternal = 0;
  static constexpr uint32_t kLeaf = 1;
  static constexpr uint32_t kFree = 2;

  static int kKeySize;
  static int kEntrySize;
};

static_assert(BP::kCacheNum > 8, "b+树结点缓存需大于8");

/**
 * b+树文件头
 * key size：键大小，可据此计算内部结点阶数
 * entry size：记录大小，可据此计算叶结点阶数
 * node num：结点数量
 * height：树高
 * root pos：根结点位置
 * left_leaf_pos：最左叶结点位置
 * file_tail_pos：文件末尾位置
 * free_node_num：空闲结点数量
 * free_head_pos：空闲结点链表头位置
 * 最左叶子结点始终为文件头后的数据块，因为操作中其位置始终不变
 */
struct BPlusHeader {
  uint16_t key_size;
  uint32_t entry_size;
  uint64_t node_num;
  uint16_t height;
  bp_pos root_pos;
  bp_pos left_leaf_pos;
  bp_pos file_tail_pos;
  uint64_t free_node_num;
  bp_pos free_head_pos;
public:
  void Init();
  void Write(std::fstream &f);
  void Read(std::fstream &f);
};

/**
 * b+树结点在内存（结点缓存）中的形态
 * pos：结点对应磁盘位置
 * type：结点类型，包括内部结点、叶结点、空闲结点
 * keys：仅内部结点有效，表示结点所有键值
 * children：仅内部结点有效，表示孩子结点的磁盘位置
 *
 * entries：仅叶结点有效，结点记录整块存储在单个字符串中；
 *        每个记录规定开头存放键值，以实现叶结点比较；
 *        叶结点返回的记录也是字符串形式，由调用方进行解析
 *
 * next：叶结点中表示下一叶结点磁盘位置，空闲结点中表示下一空闲结点磁盘位置
 * dirty：脏结点在缓存中被淘汰时需写回磁盘
 *
 * b+树结点磁盘存储：
 *  内部结点：type num(键数量) ks ps
 *  叶结点：type next num(记录数量) es
 *  空结点：type next
 */
struct BPlusNode {
  bp_pos pos;
  uint32_t type;
  std::string keys;
  std::vector<bp_pos> children;
  std::string entries;
  bp_pos next;
  bool dirty;
public:
  using NumType = uint32_t;
  void Write(std::fstream &f);
  void Read(std::fstream &f, bp_pos pos);

  NumType KNum() { return keys.size() / BP::kKeySize; }

  const char *KI(int i) { return keys.c_str() + i * BP::kKeySize; }

  void KInsert(int i, const char *key, NumType count = 1) {
    keys.insert(i * BP::kKeySize, key, BP::kKeySize * count);
  }

  void KErase(int i, NumType count = 1) {
    keys.erase(i * BP::kKeySize, BP::kKeySize * count);
  }

  void KReplace(int i, const char *key) {
    keys.replace(i * BP::kKeySize, BP::kKeySize, key, BP::kKeySize);
  }

  NumType ENum() { return entries.size() / BP::kEntrySize; }

  const char *EI(int i) { return entries.c_str() + i * BP::kEntrySize; }

  void EInsert(int i, const char *entry, NumType count = 1) {
    entries.insert(i * BP::kEntrySize, entry, BP::kEntrySize * count);
  }

  void EErase(int i, NumType count = 1) {
    entries.erase(i * BP::kEntrySize, BP::kEntrySize * count);
  }

};

}  // namespace bplus

#endif //BP_TREE_DISK_TREE_FILE_STRUCT_H

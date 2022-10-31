#ifndef BP_TREE_DISK_TREE_FILE_STRUCT_H
#define BP_TREE_DISK_TREE_FILE_STRUCT_H

#include "custom_exceptions.h"
#include "simple_utils.h"
#include <ios>
#include <string>
#include <vector>
#include <cstdint>
#include <fstream>

namespace simple {

using b_plus_off = std::streamoff;

constexpr int kHeaderAlign = 4096;
constexpr int kBlockSize = 16 * 1024;
constexpr int kMaxEntrySize = 4096;
constexpr int kCacheNum = 64;
constexpr uint32_t kInternalNodeType = 0;
constexpr uint32_t kLeafNodeType = 1;
constexpr uint32_t kFreeNodeType = 2;

static_assert(kCacheNum > 8, "b+树结点缓存需大于8");

/**
 * b+树文件头
 * key size：键大小，可据此计算内部结点阶数
 * entry size：记录大小，可据此计算叶结点阶数
 * node num：结点数量
 * height：树高
 * root pos：根结点位置
 * file_tail_pos：文件末尾位置
 * free_node_num：空闲结点数量
 * free_head_pos：空闲结点链表头位置
 * 最左叶子结点始终为文件头后的数据块，因为操作中其位置始终不变
 */
template<typename Key>
struct BPlusHeader {
  uint16_t key_size;
  uint32_t entry_size;
  uint64_t node_num;
  uint16_t height;
  b_plus_off root_pos;
  b_plus_off file_tail_pos;
  uint64_t free_node_num;
  b_plus_off free_head_pos;
public:
  void InitInMemory(int entry_size);
  void WriteToDisk(std::fstream &tree_file);
  void ReadFromDisk(std::fstream &tree_file);
};

/**
 * b+树结点在内存（结点缓存）中的形态
 * self_pos：结点对应磁盘位置
 * type：结点类型，包括内部结点、叶结点、空闲结点
 * keys：仅内部结点有效，表示结点所有键值
 * children：仅内部结点有效，表示孩子结点的磁盘位置
 *
 * entrys：仅叶结点有效，结点记录整块存储在单个字符串中；
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
template<typename Key>
struct BPlusNode {
  b_plus_off self_pos;
  uint32_t type;
  std::vector<Key> keys;
  std::vector<b_plus_off> children;
  std::string entrys;
  b_plus_off next;
  bool dirty;
public:
  using NumType = uint32_t;
  void WriteToDisk(std::fstream &tree_file, int entry_size);
  void ReadFromDisk(std::fstream &tree_file, b_plus_off node_pos, int entry_size);
};

template<typename Key>
void BPlusNode<Key>::WriteToDisk(std::fstream &tree_file, int entry_size) {
  if (!tree_file.seekp(self_pos)) throw unix_sys_error("seekp writing node");
  // 写入类型
  Write(tree_file, &type, sizeof(type));
  // 按类型写入
  if (type == kInternalNodeType) {  // 内部结点
    uint32_t num = keys.size();
    Write(tree_file, &num, sizeof(num));
    Write(tree_file, keys.data(), keys.size() * sizeof(Key));
    Write(tree_file, children.data(), children.size() * sizeof(b_plus_off));
  } else if (type == kLeafNodeType) {  // 叶结点
    Write(tree_file, &next, sizeof(b_plus_off));
    uint32_t num = entrys.size() / entry_size;
    Write(tree_file, &num, sizeof(num));
    Write(tree_file, entrys.data(), entrys.size());
  } else if (type == kFreeNodeType) {  // 空结点
    Write(tree_file, &next, sizeof(b_plus_off));
  } else {
    throw unix_sys_error("error node type");
  }
  dirty = false;
}

template<typename Key>
void
BPlusNode<Key>::ReadFromDisk(std::fstream &tree_file, b_plus_off node_pos, int entry_size) {
  self_pos = node_pos;
  if (!tree_file.seekg(self_pos)) throw unix_sys_error("seekg reading node");
  // 读入结点类型
  Read(tree_file, &type, sizeof(type));
  // 按类型解析
  if (type == kInternalNodeType) {  // 内部结点
    uint32_t num = 0;
    Read(tree_file, &num, sizeof(num));
    keys.resize(num);
    Read(tree_file, keys.data(), keys.size() * sizeof(Key));
    children.resize(num + 1);
    Read(tree_file, children.data(), children.size() * sizeof(b_plus_off));
  } else if (type == kLeafNodeType) {  // 叶结点
    Read(tree_file, &next, sizeof(b_plus_off));
    uint32_t num = 0;
    Read(tree_file, &num, sizeof(num));
    entrys.resize(num * entry_size);
    Read(tree_file, entrys.data(), entrys.size());
  } else if (type == kFreeNodeType) {  // 空结点
    Read(tree_file, &next, sizeof(b_plus_off));
  } else {
    throw unix_sys_error("error node type");
  }
  dirty = false;
}

template<typename Key>
void BPlusHeader<Key>::InitInMemory(int entry_size) {
  key_size = sizeof(Key);
  this->entry_size = entry_size;
  node_num = 0;
  height = 0;
  root_pos = 0;
  file_tail_pos = kHeaderAlign;
  free_node_num = 0;
  free_head_pos = 0;
}

template<typename Key>
void BPlusHeader<Key>::WriteToDisk(std::fstream &tree_file) {
  if (!tree_file.seekp(0)) throw unix_sys_error("seekp writing header");
  Write(tree_file, &key_size, sizeof(key_size));
  Write(tree_file, &entry_size, sizeof(entry_size));
  Write(tree_file, &node_num, sizeof(node_num));
  Write(tree_file, &height, sizeof(height));
  Write(tree_file, &root_pos, sizeof(root_pos));
  Write(tree_file, &file_tail_pos, sizeof(file_tail_pos));
  Write(tree_file, &free_node_num, sizeof(free_node_num));
  Write(tree_file, &free_head_pos, sizeof(free_head_pos));
}

template<typename Key>
void BPlusHeader<Key>::ReadFromDisk(std::fstream &tree_file) {
  if (!tree_file.seekg(0)) throw unix_sys_error("seekg tree_ file");
  Read(tree_file, &key_size, sizeof(key_size));
  Read(tree_file, &entry_size, sizeof(entry_size));
  Read(tree_file, &node_num, sizeof(node_num));
  Read(tree_file, &height, sizeof(height));
  Read(tree_file, &root_pos, sizeof(root_pos));
  Read(tree_file, &file_tail_pos, sizeof(file_tail_pos));
  Read(tree_file, &free_node_num, sizeof(free_node_num));
  Read(tree_file, &free_head_pos, sizeof(free_head_pos));
}


}  // namespace simple

#endif //BP_TREE_DISK_TREE_FILE_STRUCT_H

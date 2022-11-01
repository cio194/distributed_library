#include "b_plus_struct.h"

namespace bplus {

int BP::kKeySize = 0;
int BP::kEntrySize = 0;

void BPlusHeader::Init() {
  key_size = BP::kKeySize;
  entry_size = BP::kEntrySize;
  node_num = 0;
  height = 0;
  root_pos = 0;
  left_leaf_pos = BP::kHeaderAlign;
  file_tail_pos = BP::kHeaderAlign;
  free_node_num = 0;
  free_head_pos = 0;
}

void BPlusHeader::Write(std::fstream &f) {
  if (!f.seekp(0)) throw unix_sys_error("seekp");
  WriteThrow(f, &key_size, sizeof(key_size));
  WriteThrow(f, &entry_size, sizeof(entry_size));
  WriteThrow(f, &node_num, sizeof(node_num));
  WriteThrow(f, &height, sizeof(height));
  WriteThrow(f, &root_pos, sizeof(root_pos));
  WriteThrow(f, &left_leaf_pos, sizeof(left_leaf_pos));
  WriteThrow(f, &file_tail_pos, sizeof(file_tail_pos));
  WriteThrow(f, &free_node_num, sizeof(free_node_num));
  WriteThrow(f, &free_head_pos, sizeof(free_head_pos));
}

void BPlusHeader::Read(std::fstream &f) {
  if (!f.seekg(0)) throw unix_sys_error("seekg tree_ file");
  ReadThrow(f, &key_size, sizeof(key_size));
  ReadThrow(f, &entry_size, sizeof(entry_size));
  ReadThrow(f, &node_num, sizeof(node_num));
  ReadThrow(f, &height, sizeof(height));
  ReadThrow(f, &root_pos, sizeof(root_pos));
  ReadThrow(f, &left_leaf_pos, sizeof(left_leaf_pos));
  ReadThrow(f, &file_tail_pos, sizeof(file_tail_pos));
  ReadThrow(f, &free_node_num, sizeof(free_node_num));
  ReadThrow(f, &free_head_pos, sizeof(free_head_pos));
}


void BPlusNode::Write(std::fstream &f) {
  if (!f.seekp(pos)) throw unix_sys_error("seekp");

  // 写入类型
  WriteThrow(f, &type, sizeof(type));

  if (type == BP::kInternal) {
    // 内部结点
    auto num = KNum();
    WriteThrow(f, &num, sizeof(num));
    WriteThrow(f, keys.data(), keys.size());
    WriteThrow(f, children.data(), children.size() * sizeof(bp_pos));
  } else if (type == BP::kLeaf) {
    // 叶结点
    WriteThrow(f, &next, sizeof(bp_pos));
    auto num = ENum();
    WriteThrow(f, &num, sizeof(num));
    WriteThrow(f, entries.data(), entries.size());
  } else if (type == BP::kFree) {
    // 空结点
    WriteThrow(f, &next, sizeof(bp_pos));
  } else {
    // error
    throw unix_sys_error("error node-type");
  }
  dirty = false;
}

void BPlusNode::Read(std::fstream &f, bp_pos pos) {
  this->pos = pos;
  if (!f.seekg(pos)) throw unix_sys_error("seekg");

  // 读入结点类型
  ReadThrow(f, &type, sizeof(type));

  if (type == BP::kInternal) {
    // 内部结点
    NumType num;
    ReadThrow(f, &num, sizeof(num));
    keys.resize(num * BP::kKeySize);
    ReadThrow(f, keys.data(), keys.size());
    children.resize(num + 1);
    ReadThrow(f, children.data(), children.size() * sizeof(bp_pos));
  } else if (type == BP::kLeaf) {
    // 叶结点
    ReadThrow(f, &next, sizeof(bp_pos));
    NumType num;
    ReadThrow(f, &num, sizeof(num));
    entries.resize(num * BP::kEntrySize);
    ReadThrow(f, entries.data(), entries.size());
  } else if (type == BP::kFree) {
    // 空结点
    ReadThrow(f, &next, sizeof(bp_pos));
  } else {
    // error
    throw unix_sys_error("error node-type");
  }
  dirty = false;
}

}  // namespace bplus

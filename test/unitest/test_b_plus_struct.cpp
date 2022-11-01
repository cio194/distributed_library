#include "b_plus_tree/b_plus_struct.h"
#include "test_utils.h"
#include <memory>

using namespace bplus;

void TestHeader() {
  auto tmpf = GenTmpFile();
  {
    auto h = std::make_unique<BPlusHeader>();
    h->Init();
    h->Write(tmpf);
  }
  auto h1 = std::make_unique<BPlusHeader>();
  h1->Read(tmpf);
}

void TestInternalNode() {
  auto tmpf = GenTmpFile();
  {
    auto n = std::make_unique<BPlusNode>();
    n->pos = 4096;
    n->type = BP::kInternal;
    n->keys = rand_str(BP::kKeySize * 16);
    n->children.resize(16 + 1);
    n->Write(tmpf);
  }
  auto n1 = std::make_unique<BPlusNode>();
  n1->Read(tmpf, 4096);
}

int main() {
  BP::kKeySize = 64;
  BP::kEntrySize = 64;
//  TestHeader();
//  TestInternalNode();
}
#include <emp-zk/emp-zk.h>

#include "emp-tool/emp-tool.h"

const int threads = 1;
const string circuit_file_location = macro_xstr(EMP_CIRCUIT_PATH) + string("bristol_format/");
int main(int argc, char ** argv) {
  int party, port;
  parse_party_and_port(argv, & party, & port);
  string filename = circuit_file_location + string("sha-256.txt");
  BoolIO < NetIO > * ios[threads];
  for (int i = 0; i < threads; ++i)
    ios[i] = new BoolIO < NetIO > (new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i), party == ALICE);
  setup_zk_bool < BoolIO < NetIO >> (ios, threads, party);

  auto bits_1 = convertEntryToBooleanArray("out.txt", 0);
  auto bits_2 = convertEntryToBooleanArray("out.txt", 1);
  auto bits_3 = convertEntryToBooleanArray("out.txt", 2);
  auto bits_4 = convertEntryToBooleanArray("out.txt", 3);
  auto bits_5 = convertEntryToBooleanArray("out.txt", 4);
  auto bits_6 = convertEntryToBooleanArray("out.txt", 5);
  auto bits_7 = convertEntryToBooleanArray("out.txt", 6);
  auto bits_8 = convertEntryToBooleanArray("out.txt", 7);

  bool * num_1 = new bool[256];
  bool * num_2 = new bool[256];
  bool * num_3 = new bool[256];
  bool * num_4 = new bool[256];
  bool * num_5 = new bool[256];
  bool * num_6 = new bool[256];
  bool * num_7 = new bool[256];
  bool * num_8 = new bool[256];

  memset(num_1, false, 256);
  memset(num_2, false, 256);
  memset(num_3, false, 256);
  memset(num_4, false, 256);
  memset(num_5, false, 256);
  memset(num_6, false, 256);
  memset(num_7, false, 256);
  memset(num_8, false, 256);

  for (size_t i = 0; i < 256; i++) {
    num_1[i] = bits_1[i];
    num_2[i] = bits_2[i];
    num_3[i] = bits_3[i];
    num_4[i] = bits_4[i];
    num_5[i] = bits_5[i];
    num_6[i] = bits_6[i];
    num_7[i] = bits_7[i];
    num_8[i] = bits_8[i];
  }
  bool ** array_leaves = new bool * [8];
  array_leaves[0] = num_1;
  array_leaves[1] = num_2;
  array_leaves[2] = num_3;
  array_leaves[3] = num_4;
  array_leaves[4] = num_5;
  array_leaves[5] = num_6;
  array_leaves[6] = num_7;
  array_leaves[7] = num_8;

  sort_leaves(array_leaves, 8, 256);

  MerkleTree tree(8, 4, array_leaves, filename);
  int * tree_path;
  tree.init_verify_path( & tree_path);

  auto leaf_bits = convertEntryToBooleanArray("out.txt", 0);
  bool * actual_leaf = new bool[256];
  memset(actual_leaf, false, 256);
  for (size_t q = 0; q < 256; q++) {
    actual_leaf[q] = leaf_bits[q];
  }

  tree.prove_in_tree(actual_leaf, tree_path);
  finalize_zk_bool < BoolIO < NetIO >> ();
  for (int i = 0; i < threads; ++i) {
    delete ios[i] -> io;
    delete ios[i];
  }
  return 0;
}
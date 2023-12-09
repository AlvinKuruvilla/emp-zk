
#include <emp-zk/emp-zk.h>
#include <iostream>
#include <emp-tool/emp-tool.h>
using namespace emp;
using namespace std;

int port, party;
const int threads = 1;

const string circuit_file_location = macro_xstr(EMP_CIRCUIT_PATH) + string("bristol_format/");

int main(int argc, char **argv)
{
    int party, port;
    parse_party_and_port(argv, &party, &port);
    string filename = circuit_file_location + string("sha-256.txt");
    BoolIO<NetIO> *ios[threads];
    for (int i = 0; i < threads; ++i)
        ios[i] = new BoolIO<NetIO>(new NetIO(party == ALICE ? nullptr : "127.0.0.1", port + i), party == ALICE);
    setup_zk_bool<BoolIO<NetIO>>(ios, threads, party);

    bool **array_leaves = new bool *[8];
    bool **addr_leaves = new bool *[8];
    dynamic_leaf_array_init(8, array_leaves, addr_leaves);

    sort_leaves(array_leaves, 8, 256);

    MerkleTree tree(8, 4, array_leaves, filename);

    int *tree_path;
    tree.init_verify_path(&tree_path);

    auto leaf_bits = convertEntryToBooleanArray("out.txt", 12);
    bool *actual_leaf = new bool[256];

    memset(actual_leaf, false, 256);
    for (size_t q = 0; q < 256; q++)
    {
        actual_leaf[q] = leaf_bits[q];
    }
    tree.prove_in_tree(actual_leaf, tree_path);

    finalize_zk_bool<BoolIO<NetIO>>();
    for (int i = 0; i < threads; ++i)
    {
        delete ios[i]->io;
        delete ios[i];
    }

    return 0;
}
        
#ifndef EMP_MERKLE_H__
#define EMP_MERKLE_H__

#include <emp-zk/emp-zk.h>
#include <iostream>
#include <emp-tool/emp-tool.h>
using namespace emp;
using namespace std;

class MerkleTree
{
    int num_leaves;
    int levels;
    bool ***tree;

public:
    MerkleTree(int num_leaves, int levels, bool **bit_array, string filename);
    void leaf_print(int level, int leaf);
    void init_verify_path(int **tree_path); // use once
    void get_verify_path(int *tree_path, int index);
    void prove_in_tree(bool *leaf, int *path);
    void prove_not_in_tree(bool *non_leaf);
};

MerkleTree::MerkleTree(int num_leaves, int levels, bool **bit_array, string filename)
{
    this->num_leaves = num_leaves;
    this->levels = levels;

    if ((num_leaves == 0) || ((num_leaves & (num_leaves - 1)) != 0))
    {
        error("number of leaves is not a power of 2\n");
    }

    if (num_leaves != pow(2, levels - 1))
    {
        error("wrong number of levels for leaf cardinality\n");
    }
    tree = new bool **[levels];
    tree[0] = new bool *[num_leaves];
    for (int i = 0; i < levels; i++)
    {
        int nodes = pow(2, levels - (i + 1));
        tree[i] = new bool *[nodes];
        for (int j = 0; j < nodes; j++)
        {
            tree[i][j] = new bool[256];
        }
    }

    for (int leaf = 0; leaf < num_leaves; leaf++)
    {
        tree[0][leaf] = bit_array[leaf];
    }

    for (int lvl = 1; lvl < levels; lvl++)
    {
        int lvl_node_count = pow(2, levels - (lvl + 1));
        int pairing[2]{0, 1};
        for (int nd = 0; nd < lvl_node_count; nd++)
        {
            bool *wit = new bool[512];
            expand_to(tree[lvl - 1][pairing[0]], tree[lvl - 1][pairing[1]], wit, 256, 512);
            bool *output = new bool[256];
            hash_witness(output, wit, filename);
            tree[lvl][nd] = output;
            pairing[0] += 2;
            pairing[1] += 2;
        }
    }
}

void MerkleTree::leaf_print(int level, int leaf)
{
    for (int i = 0; i < 256; i++)
    {
        cout << tree[level][leaf][i];
    }
    cout << endl;
}

void MerkleTree::init_verify_path(int **tree_path)
{
    *tree_path = new int[levels];
}

void MerkleTree::get_verify_path(int *tree_path, int index)
{
    tree_path[0] = index;
    for (int i = 1; i < levels; i++)
    {
        int child_id = tree_path[i - 1];
        int parent_id = child_id / 2;
        if (parent_id >= pow(2, levels - (i + 1)))
        {
            for (int j = i; j < levels; j++)
            {
                tree_path[j] = 9;
            }
            break;
        }
        else
        {
            tree_path[i] = parent_id;
        }
    }
}

void MerkleTree::prove_in_tree(bool *leaf, int *path)
{
    bool leaf_flag = 0;
    for (int i = 0; i < num_leaves; i++)
    {
        int res = compare(leaf, tree[0][i], 256);
        // cout<<"res = "<<res<<endl;
        switch (res)
        {
        case 2:
            this->get_verify_path(path, i);
            leaf_flag = 1;
            break;
        default:
            break;
        }
    }
    if (leaf_flag == 0)
    {
        cout << "Leaf not in tree" << endl;
    }
    else if (path[levels - 1] < 0)
    {
        cout << "leaf does not have path to root" << endl;
    }
    else
    {
        cout << "leaf does have path to root" << endl;
        for (int level_index = 0; level_index < levels - 1; level_index++)
        {
            cout << path[level_index] << "->";
        }
        cout << path[levels - 1];
    }
    cout << endl;
}

void MerkleTree::prove_not_in_tree(bool *non_leaf)
{
    int *left_path = new int[levels];
    int *right_path = new int[levels];
    int proof_flag = 1; // assume leaf is in tree
    int match_flag = 0;

    for (int i = 0; i < num_leaves; i++)
    {
        int res = compare(non_leaf, tree[0][i], 256);
        if (res == 2)
        {
            match_flag = 1;
        }
    }

    if (match_flag > 0)
    {
        cout << "leaf found in tree" << endl;
    }
    else
    {
        int prev = 0;
        for (int i = 0; i < num_leaves; i++)
        {
            int res2 = compare(non_leaf, tree[0][i], 256);
            if (res2 == 1 && prev == 3)
            {
                this->get_verify_path(left_path, i - 1);
                this->get_verify_path(right_path, i);
                if (left_path[0] == (right_path[0] - 1))
                {
                    proof_flag = 0;
                }
            }
            prev = res2;
        }
    }

    if (proof_flag == 0)
    {
        cout << "leaf is proven not to be in tree" << endl;
    }
    else if (proof_flag == 1 && match_flag == 0)
    {
        cout << "leaf value exceeded one of the extremes of the tree leaves" << endl;
    }
}

void dynamic_leaf_array_init(int num_of_leaves, bool **leaf_array, bool **addr_leaf_array)
{
    std::vector<vector<bool>> bits_array(num_of_leaves);
    for (int i = 0; i < num_of_leaves; i++)
    {
        bits_array[i] = convertEntryToBooleanArray("out.txt", i);
    }

    for (size_t lf = 0; lf < num_of_leaves; lf++)
    {
        bool *num = new bool[256];

        for (int bt = 0; bt < 256; bt++)
        {
            num[bt] = bits_array[lf][bt];
        }

        leaf_array[lf] = num;
    }
}

#endif

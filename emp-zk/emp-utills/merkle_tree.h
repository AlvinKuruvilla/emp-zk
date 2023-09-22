#ifndef EMP_MERKLE_H__
#define EMP_MERKLE_H__

#include <iostream>
#include <fstream>
#include <string>
#include <emp-zk/emp-zk.h>
#include "emp-tool/emp-tool.h"

namespace emp
{
    class MerkleTree
    {

    public:
        int num_leaves;
        int levels;
        bool ***tree;
        MerkleTree(int num_leaves, int levels, bool **bit_array, string filename);
        void leaf_print(int level, int leaf);
        void init_verify_path(int **tree_path); // use once
        void get_verify_path(int *tree_path, int index);
        void prove_in_tree(bool *leaf, int *path);
        void prove_not_in_tree(bool *non_leaf);
        void serialize(const std::string &filename);
        MerkleTree* deserialize(const std::string &filename);
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
            std::cout << tree[level][leaf][i];
        }
        std::cout << std::endl;
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
            std::cout << "Leaf not in tree" << std::endl;
            return;
        }
        else if (path[levels - 1] < 0)
        {
            std::cout << "leaf does not have path to root" << std::endl;
            return;
        }
        std::cout << "Leaf found" << std::endl;
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
            std::cout << "leaf found in tree" << std::endl;
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
            std::cout << "leaf is proven not to be in tree" << std::endl;
        }
        else if (proof_flag == 1 && match_flag == 0)
        {
            std::cout << "leaf value exceeded one of the extremes of the tree leaves" << std::endl;
        }
    }
    void MerkleTree::serialize(const std::string &filename) {
        std::ofstream outFile(filename);
        if (!outFile) {
            std::cerr << "Error opening file for writing: " << filename << std::endl;
            return;
        }

        outFile << num_leaves << std::endl;
        outFile << levels << std::endl;

        for (int i = 0; i < levels; i++) {
            int nodes = pow(2, levels - (i + 1));
            for (int j = 0; j < nodes; j++) {
                for(int k = 0; k < 256; k++) {
                outFile << tree[i][j][k];
            }
            outFile << "" << std::endl;
        }
        }
        outFile.close();
    }
MerkleTree* MerkleTree::deserialize(const std::string &filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return nullptr;
    }

    int num_leaves, levels;
    file >> num_leaves;
    file >> levels;
    bool **bit_array;
    bit_array = new bool *[num_leaves];
    for (int i = 0; i < num_leaves; ++i) {
        bit_array[i] = new bool[256];
        std::string line;
        file >> line;
        if (line.length() != 256) {
            std::cerr << "Error: Expected line of length 256, but got " << line.length() << std::endl;
        }
        for (int j = 0; j < 256; ++j) {
            bit_array[i][j] = (line[j] == '1');
        }
    }

    file.close();

    return new MerkleTree(num_leaves, levels, bit_array, filename);
}
}
#endif

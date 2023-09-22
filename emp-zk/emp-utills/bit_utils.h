#include <iostream>
#include <math.h>
#include <time.h>
#include <emp-tool/emp-tool.h>
#include <emp-zk/emp-zk.h>

using namespace std;
namespace emp {

  template <size_t N1, size_t N2>
    bitset<N1 + N2> concat(const bitset < N1 > & b1,
      const bitset<N2> & b2) {
      string s1 = b1.to_string();
      string s2 = b2.to_string();
      return bitset<N1 + N2> (s1 + s2);
    }

  void concat_bits(int n1, int n2, bool * witness) {
    std::bitset <32> bits1(n1);
    std::bitset <32> bits2(n2);
    //cout<<bits1<<endl;
    auto out_bits = concat(bits1, bits2);

    for (size_t i = 0; i < out_bits.size(); i++) {
      witness[i] = out_bits.test(i);
    }
  }

  void hash_witness(bool * output, bool * witness, string filename) {
    vector <Bit> W, O;
    for (int i = 0; i < 512; ++i)
      W.push_back(Bit(witness[i], ALICE));
    O.resize(512);
    for (int i = 0; i < 512; ++i) O[i] = Bit(false, PUBLIC);
    BristolFormat cf(filename.c_str());
    cf.compute((block * ) O.data(), (block * ) W.data(), nullptr);
    for (int i = 0; i < 64; ++i)
      cf.compute(O.data(), O.data(), nullptr);
    for (int i = 0; i < 256; ++i) {
      bool tmp = O[i].reveal < bool > (PUBLIC);
      //if(tmp != output[i])
      //        error("wrong");
      output[i] = tmp;
    }
  }

  void expand_to(bool * source_one, bool * source_two, bool * dest, int init_size, int target_size) { //edit this to take two nodes and make them into one witness
    if (target_size != (init_size * 2)) {
      error("leaves must be same size");
    }
    for (int i = 0; i < init_size; i++) {
      bool temp_bit = source_one[i];
      dest[i] = temp_bit;
    }
    int indx_counter = init_size;
    for (int j = init_size; j < target_size; j++) {
      bool temp_bit_again = source_two[init_size - indx_counter];
      dest[j] = temp_bit_again;
      indx_counter--;
    }

  }

  void bitwise_add(bool * num_one, bool * num_two, bool * res, int new_size) {
    bool carry = 0;

    for (int i = 0; i < new_size; i++) {
      bool answ = num_one[i] ^ num_two[i];
      switch (carry) {
      case 1:
        switch (answ) {
        case 1:
          res[i] = 0;
          carry = 1;
          break;
        default:
          if (num_one[i] && num_two[i] == 1) {
            res[i] = 1;
            carry = 1;
          } else {
            res[i] = 1;
            carry = 0;
          }
          break;
        }
        break;
      default:
        res[i] = answ;
        carry = num_one[i] && num_two[i];
        break;
      }
    }
    if (carry == 1) {
      error("overflow\n");
    }
  }
  int compare(bool * comparing, bool * compare_to, int size) { //use & as inputs for bit arrays
    int sum_1 = 0;
    int sum_2 = 0;
    for (int i = 0; i < size; i++) {
      if (comparing[i] == true) {
        sum_1 += pow(2, i);
      }
      if (compare_to[i] == true) {
        sum_2 += pow(2, i);
      }
    }
    if (sum_1 < sum_2) {
      return 1;
    } //first bool array is less than second input array
    else if (sum_1 == sum_2) {
      return 2;
    } //first is equal to second
    else {
      return 3;
    } //first is greater than second
  }

  void inner_sort_mark_II(bool ** leaves, int size, int bit_size) {
    vector <int> nums;
    for (int i = 0; i < size; i++) {
      int number = 0;
      for (int j = 0; j < bit_size; j++) {
        if (leaves[i][j] == 1) {
          number += pow(2, j);
        }

      }

      nums.push_back(number);

    }
    int total_nums = nums.size();
    std::sort(nums.begin(), nums.end());
    for (int leaves_indx = 0; leaves_indx < total_nums; leaves_indx++) {
      std::bitset <32> bit_from_vec(nums[leaves_indx]);
      for (size_t bt = 0; bt < bit_from_vec.size(); bt++) {
        leaves[leaves_indx][bt] = bit_from_vec.test(bt);
      }

    }
  }

  void sort_leaves(bool ** incoming_leaves, int size, int bit_size) {
    inner_sort_mark_II(incoming_leaves, size, bit_size);
  }

  std::vector <bool> toBooleanArray(unsigned long long num) {
    std::bitset < 256 > b(num);
    std::cout << num << std::endl;
    std::vector < bool > arr(256);
    for (int i = 0; i < 256; ++i) {
      arr[i] = b[i];
    }
    return arr;
  }

  std::vector<std::vector<bool>> readDataset(const std::string & filename) {
    std::vector < std::vector < bool >> results;
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Failed to open file: " << filename << std::endl;
      return results;
    }

    unsigned long long num;
    while (file >> num) {
      results.push_back(toBooleanArray(num));
    }
    file.close();
    return results;
  }

  std::vector<bool> convertEntryToBooleanArray(const std::string & filename, size_t entryIndex) {
    std::ifstream file(filename);
    if (!file.is_open()) {
      std::cerr << "Failed to open file: " << filename << std::endl;
      return std::vector < bool > ();
    }

    unsigned long long num;
    size_t currentIndex = 0;
    while (file >> num) {
      if (currentIndex == entryIndex) {
        return toBooleanArray(num);
      }
      ++currentIndex;
    }

    std::cerr << "Entry index out of bounds" << std::endl;
    return std::vector < bool > ();
  }
}
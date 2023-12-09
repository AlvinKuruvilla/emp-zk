#include <iostream>
#include <math.h>
#include <time.h>
#include <emp-tool/emp-tool.h>
#include <emp-zk/emp-zk.h>
#include "/opt/homebrew/opt/boost/boost/multiprecision/cpp_int.hpp"

using namespace std;
using namespace boost::multiprecision;
namespace emp
{

        template <size_t N1, size_t N2>
        bitset<N1 + N2> concat(const bitset<N1> &b1, const bitset<N2> &b2)
        {
                string s1 = b1.to_string();
                string s2 = b2.to_string();
                return bitset<N1 + N2>(s1 + s2);
        }

        void concat_bits(int n1, int n2, bool *witness)
        {
                std::bitset<32> bits1(n1);
                std::bitset<32> bits2(n2);
                // cout<<bits1<<endl;
                auto out_bits = concat(bits1, bits2);

                for (size_t i = 0; i < out_bits.size(); i++)
                {
                        witness[i] = out_bits.test(i);
                }
        }

        std::uintmax_t base58_decode(std::string address)
        {
                const char base58[] = "123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz";
                std::uintmax_t val = 0;
                for (char c : address)
                {
                        const char *point_to_char = &c;
                        const char *pos = strchr(base58, *point_to_char);
                        if (pos == NULL)
                        {
                                cout << "invalid char found: " << c << endl;
                                exit(-1);
                        }
                        val = val * 58 + (pos - base58);
                }
                return val;
        }

        void hash_witness(bool *output, bool *witness, string filename)
        {
                vector<Bit> W, O;
                for (int i = 0; i < 512; ++i)
                        W.push_back(Bit(witness[i], ALICE));
                O.resize(512);
                for (int i = 0; i < 512; ++i)
                        O[i] = Bit(false, PUBLIC);
                BristolFormat cf(filename.c_str());
                cf.compute((block *)O.data(), (block *)W.data(), nullptr);
                for (int i = 0; i < 64; ++i)
                        cf.compute(O.data(), O.data(), nullptr);
                for (int i = 0; i < 256; ++i)
                {
                        bool tmp = O[i].reveal<bool>(PUBLIC);
                        // if(tmp != output[i])
                        //         error("wrong");
                        output[i] = tmp;
                }
        }

        void expand_to(bool *source_one, bool *source_two, bool *dest, int init_size, int target_size)
        { // edit this to take two nodes and make them into one witness
                if (target_size != (init_size * 2))
                {
                        error("leaves must be same size");
                }
                for (int i = 0; i < init_size; i++)
                {
                        bool temp_bit = source_one[i];
                        dest[i] = temp_bit;
                }
                int indx_counter = init_size;
                for (int j = init_size; j < target_size; j++)
                {
                        bool temp_bit_again = source_two[init_size - indx_counter];
                        dest[j] = temp_bit_again;
                        indx_counter--;
                }
        }

        void bitwise_add(bool *num_one, bool *num_two, bool *res, int new_size)
        {
                bool carry = 0;

                for (int i = 0; i < new_size; i++)
                {
                        bool answ = num_one[i] ^ num_two[i];
                        switch (carry)
                        {
                        case 1:
                                switch (answ)
                                {
                                case 1:
                                        res[i] = 0;
                                        carry = 1;
                                        break;
                                default:
                                        if (num_one[i] && num_two[i] == 1)
                                        {
                                                res[i] = 1;
                                                carry = 1;
                                        }
                                        else
                                        {
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
                if (carry == 1)
                {
                        error("overflow\n");
                }
        }

        void get_plain(bool *res, bool *wit, const char *file)
        {
                setup_plain_prot(false, "");
                BristolFormat cf(file);
                vector<Bit> W, P, O;
                W.resize(cf.n1);
                for (int i = 0; i < cf.n1; ++i)
                        W[i] = Bit(wit[i], PUBLIC);
                O.resize(cf.n1);
                for (int i = 0; i < cf.n1; ++i)
                        O[i] = Bit(false, PUBLIC);
                cf.compute(O.data(), W.data(), P.data());
                for (int i = 0; i < 64; ++i)
                        cf.compute(O.data(), O.data(), P.data());
                for (int i = 0; i < cf.n3; ++i)
                {
                        res[i] = O[i].reveal<bool>(PUBLIC);
                }
                finalize_plain_prot();
        }

        int compare(bool *comparing, bool *compare_to, int size)
        { // use & as inputs for bit arrays
                unsigned long long sum_1 = 0;
                unsigned long long sum_2 = 0;
                for (int i = 0; i < size; i++)
                {
                        if (comparing[i] == true)
                        { // int new_num = pow(2,i); sum_1 = sum_1 + new_num;}
                                sum_1 += pow(2, i);
                        }
                        if (compare_to[i] == true)
                        { // int new_num_2 = pow(2,i); sum_2 = sum_2 + new_num_2;}
                                sum_2 += pow(2, i);
                        }
                }
                // cout<<"sum_1 = "<<sum_1<<endl;
                // cout<<"sum_2 = "<<sum_2<<endl;
                if (sum_1 < sum_2)
                {
                        return 1;
                } // first bool array is less than second input array
                else if (sum_1 == sum_2)
                {
                        return 2;
                } // first is equal to second
                else
                {
                        return 3;
                } // first is greater than second
        }

        void inner_sort_mark_II(bool **leaves, int size, int bit_size)
        {
                // cout<<"innersort"<<endl;
                vector<unsigned long long> nums;
                for (int i = 0; i < size; i++)
                {
                        unsigned long long number = 0;
                        // cout<<"leaf "<<i<<" = "<<endl;
                        for (int j = 0; j < bit_size; j++)
                        {
                                // cout<<leaves[i][j];
                                if (leaves[i][j] == 1)
                                {
                                        number = number + pow(2, j);
                                } // worng numbers, SO CLOSE
                        }

                        cout << "\nnumber " << i << " = " << number << endl;

                        nums.push_back(number);
                }
                int total_nums = nums.size();
                std::sort(nums.begin(), nums.end());
                for (int leaves_indx = 0; leaves_indx < total_nums; leaves_indx++)
                {
                        cout << "nums = " << nums[leaves_indx] << endl;
                        std::bitset<256> bit_from_vec(nums[leaves_indx]);
                        for (size_t bt = 0; bt < bit_from_vec.size(); bt++)
                        {
                                leaves[leaves_indx][bt] = bit_from_vec.test(bt);
                        }
                }
        }

        void sort_leaves(bool **incoming_leaves, int size, int bit_size)
        {
                inner_sort_mark_II(incoming_leaves, size, bit_size);
        }

        std::vector<bool> toBooleanArray(unsigned long long num)
        {
                std::bitset<256> b(num);
                std::cout << num << std::endl;
                std::vector<bool> arr(256);
                for (int i = 0; i < 256; i++)
                {
                        arr[i] = b[i];
                }
                return arr;
        }

        std::vector<bool> toBooleanArrayMarkII(uint256_t entry, int sz)
        {
                std::vector<bool> arr(256);
                for (int a = 0; a < 256; a++)
                {
                        arr[a] = false;
                }
                // uint256_t sixtyFourBitPrime = 28823037651711737;
                // uint256_t sixtyFourBitPrime = 6460752303423619;
                uint256_t sixtyFourBitPrime = 460752303423619;
                uint256_t prime_one = 6323;
                uint256_t prime_two = 181;
                uint256_t new_entry = ((entry * prime_one) + prime_two) % sixtyFourBitPrime;
                // cout<<"new_entry = "<<new_entry<<"\n";

                std::string new_entry_str = new_entry.str();
                unsigned long long sixtyfourBitEntry = std::stoul(new_entry_str);
                cout << "sixtyfourBitEntry = " << sixtyfourBitEntry << endl;

                std::bitset<256> b(sixtyfourBitEntry);
                for (int i = 0; i < 256; i++)
                {
                        arr[i] = b[i];
                }

                return arr;
        }

        std::vector<std::vector<bool>> readDataset(const std::string &filename)
        {
                std::vector<std::vector<bool>> results;
                std::ifstream file(filename);
                if (!file.is_open())
                {
                        std::cerr << "Failed to open file: " << filename << std::endl;
                        return results;
                }

                unsigned long long num;
                while (file >> num)
                {
                        results.push_back(toBooleanArray(num));
                }
                file.close();
                return results;
        }

        std::vector<bool> convertEntryToBooleanArray(const std::string &filename, size_t entryIndex)
        {
                std::ifstream file(filename);
                if (!file.is_open())
                {
                        std::cerr << "Failed to open file: " << filename << std::endl;
                        return std::vector<bool>();
                }
                std::string addr;
                unsigned long long num;
                size_t currentIndex = 0;
                while (file >> addr >> num)
                {
                        uintmax_t decoded_addr = base58_decode(addr);
                        std::string str_decode_addr = std::to_string(decoded_addr);
                        std::string str_num = std::to_string(num);
                        std::string str_indx = std::to_string(currentIndex);
                        std::string full = str_decode_addr + str_num + str_indx;
                        std::stringstream full_as_stream;
                        full_as_stream << full;
                        uint256_t entry;
                        full_as_stream >> entry;
                        int sz = full.length();
                        // cout<<"entry for index "<<currentIndex<<" = "<<entry<<endl;
                        // std::vector<bool> test = toBooleanArayMarkII(entry,sz);
                        if (currentIndex == entryIndex)
                        {
                                return toBooleanArrayMarkII(entry, sz);
                        }
                        ++currentIndex;
                }

                std::cerr << "Entry index out of bounds" << std::endl;
                return std::vector<bool>();
        }
}

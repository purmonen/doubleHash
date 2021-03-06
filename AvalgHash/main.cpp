#include <iostream>
#include <fstream>
#include <stdint.h>
#include <math.h>
#include <vector>


template <typename T>
void printVector(std::vector<T> vector) {
    for (auto t: vector) {
        std::cerr << t << " ";
    }
    std::cerr << std::endl;
}


const int rows = 22;
const int slots = 1 << rows;
uint64_t H[rows];
std::vector<uint64_t> hashTable[slots] = {std::vector<uint64_t>()};
int seeds[slots] = {0};
int bits[slots] = {0};

uint64_t bitsSet(uint64_t x) {
    uint64_t count = 0;
    while (x != 0) {
        count += x & 1;
        x = x >> 1;
    }
    return count;
}

uint64_t randomNum() {
    return (uint64_t(rand()) << 32) + uint64_t(rand());
}

uint64_t hash(uint64_t x) {
    uint64_t result = 0;
    for (uint64_t i = 0; i < rows; i++) {
        result += ((bitsSet(H[i] & x)) % 2) << i;
    }
    return result;
}

uint64_t secondHash(uint64_t x, uint64_t seed, uint64_t bits) {
    srand(int(seed));
    uint64_t result = 0;
    for (uint64_t i = 0; i < bits; i++) {
        result += ((bitsSet(randomNum() & x)) % 2) << i;
    }
    return result;
}

bool hashContains(uint64_t x) {
    auto i = hash(x);
    if (hashTable[i].size() < 3) {
        return find(hashTable[i].begin(), hashTable[i].end(), x) != hashTable[i].end();
    }
    return x == hashTable[i][secondHash(x, seeds[i], hashTable[i].size()) % hashTable[i].size()];
}


int main(int argc, const char * argv[]) {
    std::cout << "Generating hash function" << std::endl;
    for (int i = 0; i < rows; i++) {
        H[i] = randomNum();
    }
    std::cout << "Reading file" << std::endl;
    std::ifstream fileStream("/Users/Sami/Desktop/avalghash");
    uint64_t num;
    std::vector<uint64_t> nums;
    while (fileStream >> std::hex >> num) {
        auto h = hash(num);
        hashTable[h].push_back(num);
        nums.push_back(num);
    }
    std::cout << "Hashing elements" << std::endl;
    int count = 0;
    int empty = 0;
    std::vector<int> usedSeeds[30];
    for (int i = 0; i < slots; i++) {
        uint64_t currentSize = hashTable[i].size();
        if (currentSize > 3) {
            //            std::cout << "Hashing " << i << ", size " << currentSize << std::endl;
        }
        if (currentSize == 0) {
            empty++;
        }
        if (currentSize >= 3) {
            count++;
            auto seed = seeds[i];
            int bits = ceil(log2(currentSize));
            auto remainingSeeds = usedSeeds[bits];
            while (seeds[i] == 0) {
                seed = int(randomNum());
                if (remainingSeeds.size() > 0) {
                    seed = remainingSeeds[remainingSeeds.size()-1];
                    remainingSeeds.pop_back();
                }
                
                std::vector<uint64_t> hashes;
                for (auto num: hashTable[i]) {
                    auto h = secondHash(num, seed, currentSize) % currentSize;
                    if (std::find(hashes.begin(), hashes.end(), h) != hashes.end()) {
                        break;
                    }
                    hashes.push_back(h);
                }
                if (hashes.size() == currentSize) {
                    auto nums = hashTable[i];
                    int j = 0;
                    for (auto num: hashTable[i]) {
                        nums[hashes[j]] = num;
                        j++;
                    }
                    seeds[i] = seed;
                    if (find(usedSeeds[bits].begin(), usedSeeds[bits].end(), seed) == usedSeeds[bits].end()) {
                        usedSeeds[bits].push_back(seed);
                    }
                    //                    bits[i] = bits;
                    hashTable[i] = nums;
                }
            }
        }
    }
    
    int numSeedsUsed = 0;
    for (int i = 0; i < 20; i++) {
        numSeedsUsed += usedSeeds[i].size();
    }
    std::cout << "Collisions >= 3 " << count << std::endl;
    std::cout << "Used inner matrices " << numSeedsUsed << std::endl;
    std::cout << "The matrix" << std::endl;
    for (int i = 0; i < rows; i++) {
        auto x = H[i];
        for (int j = 0; j < 64; j++) {
            std::cout << int(x >> (63-j) & 1) << " ";
        }
        std::cout << std::endl;
    }
    int j = 0;
    for (int i = 0; i < 20; i++) {
        if (usedSeeds[i].size() == 0) {
            continue;
        }
        for (auto seed: usedSeeds[i]) {
            std::cout << "H" << j << ", bits " << i << std::endl;
            srand(seed);
            std::vector<uint64_t> nums;
            for (uint64_t j = 0; j < i; j++) {
                nums.push_back(randomNum());
            }
            for (auto x: nums) {
                for (int j = 0; j < 64; j++) {
                    std::cout << int(x >> (63-j) & 1) << " ";
                }
                std::cout << std::endl;
            }
            j++;
        }
    }
    
//        std::cout << "Looking if we found elements " << std::endl;
//        for (auto num: nums) {
//            if (!hashContains(num)) {
//                std::cout << "Num not found " << num << std::endl;
//            }
//        }
//        std::cout << 1337 << " " << hashContains(1337) << std::endl;
//        std::cout << "Atleast " << count << " hash functions must be made" << std::endl;
//        std::cout << "Empty " << empty << std::endl;
    return 0;
}
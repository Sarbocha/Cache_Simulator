#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>

using namespace std;

// global cache configuration
int num_entries;
int associativity;
int num_sets;
int index_bits;

// a single cache line has a valid bit and a tag
struct CacheLine {
    bool valid;
    unsigned long tag;
};

// the cache is a 2D array: [set index][way]
vector<vector<CacheLine>> cache;

// tracks how long ago each line was used (higher = older)
vector<vector<int>> age;

void init_cache() {
    // calculate number of sets and how many bits we need for the index
    num_sets   = num_entries / associativity;
    index_bits = (int)log2(num_sets);

    // initialize all cache lines as invalid (cache starts empty)
    cache.assign(num_sets, vector<CacheLine>(associativity, {false, 0}));
    age.assign(num_sets, vector<int>(associativity, 0));
}

// simulates a memory access, returns true on hit and false on miss
bool access_cache(unsigned long addr) {
    // extract the set index and tag from the address
    unsigned long index = addr & ((1UL << index_bits) - 1);
    unsigned long tag   = addr >> index_bits;

    // check all ways in the set for a hit
    for (int i = 0; i < associativity; i++) {
        if (cache[index][i].valid && cache[index][i].tag == tag) {
            // hit - reset this line's age and increment everyone else's
            for (int j = 0; j < associativity; j++)
                age[index][j]++;
            age[index][i] = 0;
            return true;
        }
    }

    // miss - first try to find an empty slot before evicting
    for (int i = 0; i < associativity; i++) {
        if (!cache[index][i].valid) {
            cache[index][i] = {true, tag};
            for (int j = 0; j < associativity; j++)
                age[index][j]++;
            age[index][i] = 0;
            return false;
        }
    }

    // no empty slot, so evict the least recently used line
    int lru = 0;
    for (int i = 1; i < associativity; i++) {
        if (age[index][i] > age[index][lru])
            lru = i;
    }
    cache[index][lru] = {true, tag};
    for (int j = 0; j < associativity; j++)
        age[index][j]++;
    age[index][lru] = 0;

    return false;
}

int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: ./cache_sim num_entries associativity input_file" << endl;
        return 1;
    }

    num_entries   = stoi(argv[1]);
    associativity = stoi(argv[2]);
    string input_file = argv[3];

    init_cache();

    // open the input file with memory addresses
    ifstream fin(input_file);
    if (!fin) {
        cout << "Error opening input file" << endl;
        return 1;
    }

    // results go into cache_sim_output
    ofstream fout("cache_sim_output");

    // process each address and write hit/miss to output
    unsigned long addr;
    while (fin >> addr) {
        bool hit = access_cache(addr);
        fout << addr << " : " << (hit ? "HIT" : "MISS") << "\n";
    }

    fin.close();
    fout.close();

    return 0;
}

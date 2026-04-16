#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cmath>
 
using namespace std;
 
int num_entries;
int associativity;
int num_sets;
int index_bits;
 
// each cache line just needs a valid bit and tag
struct CacheLine {
    bool valid;
    unsigned long tag;
};
 
// the cache is a 2D array: [set][way]
// we also keep an "age" counter per line to do LRU
vector<vector<CacheLine>> cache;
vector<vector<int>> age; // higher age = used longer ago
 
void init_cache() {
    num_sets = num_entries / associativity;
 
    // figure out how many bits we need for the index
    index_bits = (int)log2(num_sets);
 
    cache.resize(num_sets, vector<CacheLine>(associativity));
    age.resize(num_sets, vector<int>(associativity, 0));
 
    // start everything as invalid
    for (int i = 0; i < num_sets; i++) {
        for (int j = 0; j < associativity; j++) {
            cache[i][j].valid = false;
            cache[i][j].tag = 0;
        }
    }
}
 
// returns true if hit, false if miss
bool access_cache(unsigned long addr) {
    // pull out the index and tag from the address
    unsigned long index = addr & ((1UL << index_bits) - 1);
    unsigned long tag   = addr >> index_bits;
 
    // check if it's already in the cache (hit)
    for (int i = 0; i < associativity; i++) {
        if (cache[index][i].valid && cache[index][i].tag == tag) {
            // hit - reset this line's age to 0, bump everyone else
            for (int j = 0; j < associativity; j++)
                age[index][j]++;
            age[index][i] = 0;
            return true;
        }
    }
 
    // miss - find an empty slot first
    for (int i = 0; i < associativity; i++) {
        if (!cache[index][i].valid) {
            cache[index][i].valid = true;
            cache[index][i].tag = tag;
            for (int j = 0; j < associativity; j++)
                age[index][j]++;
            age[index][i] = 0;
            return false;
        }
    }
 
    // no empty slot, evict the LRU line
    int lru = 0;
    for (int i = 1; i < associativity; i++) {
        if (age[index][i] > age[index][lru])
            lru = i;
    }
 
    cache[index][lru].tag = tag;
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
 
    ifstream fin(input_file);
    if (!fin) {
        cout << "Error opening input file" << endl;
        return 1;
    }
 
    ofstream fout("cache_sim_output");
 
    unsigned long addr;
    while (fin >> addr) {
        bool hit = access_cache(addr);
        fout << addr << " : " << (hit ? "HIT" : "MISS") << "\n";
    }
 
    fin.close();
    fout.close();
 
    return 0;
}
 

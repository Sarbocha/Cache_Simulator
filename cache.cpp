#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <string>

using namespace std;

// ─── Entry class ─────────────────────────────────────────────────────────────
// represents a single cache line (one slot in the cache)

class Entry {
public:
    Entry() : valid(false), tag(0), ref(0) {}
    ~Entry() {}

    void set_tag(int _tag)   { tag   = _tag;   }
    int  get_tag()           { return tag;      }

    void set_valid(bool _valid) { valid = _valid; }
    bool get_valid()            { return valid;   }

    void set_ref(int _ref)  { ref = _ref;  }
    int  get_ref()          { return ref;  }

    void display(ofstream& outfile) {
        outfile << "valid=" << valid << " tag=" << tag << " ref=" << ref;
    }

private:
    bool valid;
    unsigned tag;
    int ref;   // used for LRU tracking (higher = older)
};

// ─── Cache class ─────────────────────────────────────────────────────────────

class Cache {
public:
    Cache(int _num_entries, int _assoc) {
        num_entries = _num_entries;
        assoc       = _assoc;
        num_sets    = num_entries / assoc;

        // allocate 2D array: entries[set][way]
        entries = new Entry*[num_sets];
        for (int i = 0; i < num_sets; i++)
            entries[i] = new Entry[assoc];
    }

    ~Cache() {
        for (int i = 0; i < num_sets; i++)
            delete[] entries[i];
        delete[] entries;
    }

    // extract the set index from an address
    int get_index(unsigned long addr) {
        int index_bits = (int)log2(num_sets);
        return addr & ((1UL << index_bits) - 1);
    }

    // extract the tag from an address
    int get_tag(unsigned long addr) {
        int index_bits = (int)log2(num_sets);
        return addr >> index_bits;
    }

    // check if address is in cache; write HIT/MISS to output file
    bool hit(ofstream& outfile, unsigned long addr) {
        int index = get_index(addr);
        int tag   = get_tag(addr);

        for (int i = 0; i < assoc; i++) {
            if (entries[index][i].get_valid() && entries[index][i].get_tag() == tag) {
                // hit - reset this line's ref, bump everyone else
                for (int j = 0; j < assoc; j++)
                    entries[index][j].set_ref(entries[index][j].get_ref() + 1);
                entries[index][i].set_ref(0);
                outfile << addr << " : HIT\n";
                return true;
            }
        }

        outfile << addr << " : MISS\n";
        return false;
    }

    // place address into cache (called after a miss)
    void update(ofstream& outfile, unsigned long addr) {
        int index = get_index(addr);
        int tag   = get_tag(addr);

        // look for an empty slot first
        for (int i = 0; i < assoc; i++) {
            if (!entries[index][i].get_valid()) {
                entries[index][i].set_valid(true);
                entries[index][i].set_tag(tag);
                for (int j = 0; j < assoc; j++)
                    entries[index][j].set_ref(entries[index][j].get_ref() + 1);
                entries[index][i].set_ref(0);
                return;
            }
        }

        // no empty slot - evict the LRU line (highest ref value)
        int lru = 0;
        for (int i = 1; i < assoc; i++) {
            if (entries[index][i].get_ref() > entries[index][lru].get_ref())
                lru = i;
        }
        entries[index][lru].set_tag(tag);
        for (int j = 0; j < assoc; j++)
            entries[index][j].set_ref(entries[index][j].get_ref() + 1);
        entries[index][lru].set_ref(0);
    }

    void display(ofstream& outfile) {
        for (int i = 0; i < num_sets; i++) {
            outfile << "Set " << i << ": ";
            for (int j = 0; j < assoc; j++) {
                entries[i][j].display(outfile);
                outfile << " | ";
            }
            outfile << "\n";
        }
    }

private:
    int assoc;
    unsigned num_entries;
    int num_sets;
    Entry **entries;  // 2D array [set][way]
};

// ─── Main ─────────────────────────────────────────────────────────────────────

int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage: " << endl;
        cout << "   ./cache num_entries associativity filename" << endl;
        return 0;
    }

    unsigned entries = atoi(argv[1]);
    unsigned assoc   = atoi(argv[2]);
    string filename  = argv[3];

    // follow the professor's file naming pattern
    string input_filename  = filename;
    string output_filename = "cache_sim_output";

    cout << "Number of entries: " << entries << endl;
    cout << "Associativity: "     << assoc   << endl;
    cout << "Input File Name: "   << filename << endl;

    // open input file
    ifstream input;
    input.open(input_filename);
    if (!input.is_open()) {
        cerr << "Could not open input file " << input_filename << ". Exiting ..." << endl;
        exit(0);
    }

    // open output file
    ofstream output;
    output.open(output_filename);

    // create the cache
    Cache cache(entries, assoc);

    // process each address
    unsigned long addr;
    while (input >> addr) {
        bool is_hit = cache.hit(output, addr);
        if (!is_hit)
            cache.update(output, addr);
    }

    input.close();
    output.close();

    return 0;
}

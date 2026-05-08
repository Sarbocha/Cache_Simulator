#include <iostream>
#include <fstream>
#include <cstdlib>
#include <string>

using namespace std;

// ─────────────────────────────────────────────────────────────
// Entry class: represents one cache line
// ─────────────────────────────────────────────────────────────
class Entry {
public:
    Entry() : valid(false), tag(0), last_used(0) {}

    void set_tag(unsigned t) { tag = t; }
    unsigned get_tag() const { return tag; }

    void set_valid(bool v) { valid = v; }
    bool get_valid() const { return valid; }

    void set_last_used(unsigned long long t) { last_used = t; }
    unsigned long long get_last_used() const { return last_used; }

private:
    bool valid;
    unsigned tag;
    unsigned long long last_used;   // LRU timestamp
};

// ─────────────────────────────────────────────────────────────
// Cache class
// ─────────────────────────────────────────────────────────────
class Cache {
public:
    Cache(int _num_entries, int _assoc)
        : assoc(_assoc),
          num_entries(_num_entries),
          num_sets(_num_entries / _assoc),
          time_counter(0)
    {
        entries = new Entry*[num_sets];
        for (int i = 0; i < num_sets; i++)
            entries[i] = new Entry[assoc];
    }

    ~Cache() {
        for (int i = 0; i < num_sets; i++)
            delete[] entries[i];
        delete[] entries;
    }

    // Compute index bits WITHOUT floating point
    int get_index_bits() const {
        int bits = 0;
        int x = num_sets;
        while (x > 1) {
            x >>= 1;
            bits++;
        }
        return bits;
    }

    int get_index(unsigned long addr) const {
        int index_bits = get_index_bits();
        if (index_bits == 0) return 0;  // fully associative
        unsigned long mask = (1UL << index_bits) - 1;
        return (int)(addr & mask);
    }

    unsigned get_tag(unsigned long addr) const {
        int index_bits = get_index_bits();
        return (unsigned)(addr >> index_bits);
    }

    // Unified access: one timestamp per memory reference
    void access(ofstream& outfile, unsigned long addr) {
        time_counter++;   // ONE logical time step per access

        int index = get_index(addr);
        unsigned tag = get_tag(addr);

        // ─── HIT check ─────────────────────────────────────────
        for (int i = 0; i < assoc; i++) {
            if (entries[index][i].get_valid() &&
                entries[index][i].get_tag() == tag) {

                entries[index][i].set_last_used(time_counter);
                outfile << addr << " : HIT\n";
                return;
            }
        }

        // ─── MISS ──────────────────────────────────────────────
        outfile << addr << " : MISS\n";

        // Try empty slot first
        for (int i = 0; i < assoc; i++) {
            if (!entries[index][i].get_valid()) {
                entries[index][i].set_valid(true);
                entries[index][i].set_tag(tag);
                entries[index][i].set_last_used(time_counter);
                return;
            }
        }

        // No empty slot → evict true LRU (smallest last_used)
        int lru = 0;
        unsigned long long min_time = entries[index][0].get_last_used();

        for (int i = 1; i < assoc; i++) {
            if (entries[index][i].get_last_used() < min_time) {
                min_time = entries[index][i].get_last_used();
                lru = i;
            }
        }

        entries[index][lru].set_tag(tag);
        entries[index][lru].set_valid(true);
        entries[index][lru].set_last_used(time_counter);
    }

private:
    int assoc;
    int num_entries;
    int num_sets;
    Entry **entries;
    unsigned long long time_counter;   // global timestamp
};

// ─────────────────────────────────────────────────────────────
// Main
// ─────────────────────────────────────────────────────────────
int main(int argc, char* argv[]) {
    if (argc < 4) {
        cout << "Usage:\n";
        cout << "   ./cache_sim num_entries associativity filename\n";
        return 0;
    }

    unsigned entries = (unsigned)atoi(argv[1]);
    unsigned assoc   = (unsigned)atoi(argv[2]);
    string filename  = argv[3];

    string input_filename  = filename;
    string output_filename = "cache_sim_output";

    cout << "Number of entries: " << entries << endl;
    cout << "Associativity: "     << assoc   << endl;
    cout << "Input File Name: "   << filename << endl;

    ifstream input(input_filename);
    if (!input.is_open()) {
        cerr << "Could not open input file " << input_filename << endl;
        return 0;
    }

    ofstream output(output_filename);
    if (!output.is_open()) {
        cerr << "Could not open output file " << output_filename << endl;
        return 0;
    }

    Cache cache((int)entries, (int)assoc);

    unsigned long addr;
    while (input >> addr) {
        cache.access(output, addr);
    }

    input.close();
    output.close();

    return 0;
}

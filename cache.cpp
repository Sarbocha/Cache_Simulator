#include<iostream>
#include<fstream>

using namespace std;

class Entry {
public:
  Entry();
  ~Entry();

  void set_tag(int _tag) { tag = _tag; }
  int get_tag() { return tag; }

  void set_valid(bool _valid) { valid = _valid; }
  bool get_valid() { return valid; }

  void set_ref(int _ref) { ref = _ref; }
  int get_ref() { return ref; }

private:  
  bool valid;
  unsigned tag;
  int ref;
};

Entry::Entry() {
    valid = false;
    tag = 0;
    ref = 0;
}

Entry::~Entry() {}

class Cache {
public:
  Cache(int, int);
  ~Cache();

  int get_index(unsigned long addr);
  int get_tag(unsigned long addr);

  bool hit(ofstream& outfile, unsigned long addr);
  void update(unsigned long addr);
  void increment_time(); 

private:
  int assoc;
  unsigned num_entries;
  int num_sets;
  Entry **entries;
  int time;
};

// Constructor
Cache::Cache(int n, int a) {
    num_entries = n;
    assoc = a;
    num_sets = num_entries / assoc;
    time = 0;  

    entries = new Entry*[num_sets];
    for (int i = 0; i < num_sets; i++) {
        entries[i] = new Entry[assoc];
    }
}

// Destructor
Cache::~Cache() {
    for (int i = 0; i < num_sets; i++) {
        delete[] entries[i];
    }
    delete[] entries;
}

// Time increment (called once per access)
void Cache::increment_time() {
    time++;
}

// Index and tag
int Cache::get_index(unsigned long addr) {
    return addr % num_sets;
}

int Cache::get_tag(unsigned long addr) {
    return addr / num_sets;
}

// Check hit
bool Cache::hit(ofstream& outfile, unsigned long addr) {
    int index = get_index(addr);
    int tag = get_tag(addr);

    for (int i = 0; i < assoc; i++) {
        if (entries[index][i].get_valid() &&
            entries[index][i].get_tag() == tag) {

            entries[index][i].set_ref(time);  // update LRU
            outfile << addr << " : HIT" << endl;
            return true;
        }
    }

    outfile << addr << " : MISS" << endl;
    return false;
}

// Update cache on MISS
void Cache::update(unsigned long addr) {
    int index = get_index(addr);
    int tag = get_tag(addr);

    // Step 1: find empty slot
    for (int i = 0; i < assoc; i++) {
        if (!entries[index][i].get_valid()) {
            entries[index][i].set_valid(true);
            entries[index][i].set_tag(tag);
            entries[index][i].set_ref(time);
            return;
        }
    }

    // Step 2: find LRU
    int lru = 0;
    int min_ref = entries[index][0].get_ref();

    for (int i = 1; i < assoc; i++) {
        if (entries[index][i].get_ref() < min_ref) {
            min_ref = entries[index][i].get_ref();
            lru = i;
        }
    }

    // Replace LRU
    entries[index][lru].set_tag(tag);
    entries[index][lru].set_valid(true);
    entries[index][lru].set_ref(time);
}

// Main
int main(int argc, char* argv[]) {
    if (argc != 4) {
        cout << "Usage: ./cache_sim num_entries assoc input_file" << endl;
        return 1;
    }

    int num_entries = stoi(argv[1]);
    int assoc = stoi(argv[2]);
    string filename = argv[3];

    Cache cache(num_entries, assoc);

    ifstream infile(filename);
    if (!infile) {
        cout << "Error: could not open input file." << endl;
        return 1;
    }

    ofstream outfile("cache_sim_output");

    unsigned long addr;

    while (infile >> addr) {
        cache.increment_time();  // ONLY place time increases

        if (!cache.hit(outfile, addr)) {
            cache.update(addr);
        }
    }

    infile.close();
    outfile.close();

    return 0;
}

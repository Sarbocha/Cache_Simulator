# Cache Simulator

A cache simulator that accepts cache configuration parameters and a list of memory addresses,
then simulates cache behavior (hits/misses) using an LRU replacement policy.

## Name
- Sarbocha Pandey

## Build

```bash
make
```

This produces the executable `cache_sim`.

## Run

```bash
./cache_sim <num_entries> <associativity> <input_memory_reference_file>
```

| Argument | Description |
|---|---|
| `num_entries` | Total number of cache entries (must be a power of 2) |
| `associativity` | Number of ways per set (must evenly divide `num_entries`) |
| `input_memory_reference_file` | File containing space-separated memory word addresses |

### Examples

**Direct-mapped (4 entries, 1-way):**
```bash
./cache_sim 4 1 input0
```

**2-way set-associative (4 entries, 2 sets of 2):**
```bash
./cache_sim 4 2 input0
```

**Fully associative (4 entries, 4-way):**
```bash
./cache_sim 4 4 input0
```

## Output

Results are written to `cache_sim_output` in the format:
```
<address> : HIT
<address> : MISS
...
```

## Design Notes

- **Block size**: 1 word (as specified)
- **Addresses**: Treated as word addresses
- **Index bits**: log2(num_sets), where num_sets = num_entries / associativity
- **Tag**: upper bits of the address after the index field
- **Replacement policy**: LRU (Least Recently Used) using a doubly-linked list + hash map for O(1) hit/evict
- **Initial state**: All entries invalid (cold cache)

### Address Breakdown

```
| <--- tag ---> | <--- index (log2(num_sets) bits) ---> |
```

For a fully-associative cache (num_sets = 1), index_bits = 0, so the entire address is the tag.

## Clean

```bash
make clean
```

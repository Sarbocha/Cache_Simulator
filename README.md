# Cache Simulator

A direct-mapped and set-associative cache simulator written in C++.

## Author
Sarbocha Pandey

## Description

This program simulates the behavior of a cache given a configuration and a sequence of memory addresses. For each memory reference, the simulator reports whether it results in a cache hit or miss.

## Build Instructions

### Windows (MinGW)
```powershell
g++ -o cache_sim cache.cpp -static
```

### Linux / macOS
```bash
g++ -o cache_sim cache.cpp
```

## Usage

```
./cache_sim num_entries associativity input_file
```

### Arguments
- `num_entries` — total number of cache entries (must be a power of 2)
- `associativity` — associativity of the cache (must evenly divide `num_entries`)
- `input_file` — path to a file containing space-separated memory addresses

## Input Format

The input file should contain memory addresses separated by spaces. For example:

```
1 3 5 1 3 1
```

## Output

The simulator writes results to a file called `cache_sim_output` in the current directory. Each line corresponds to one memory reference:

```
[ADDR] : [HIT or MISS]
```

### Example

Given the input above and the command:
```
./cache_sim 4 2 input.txt
```

The output file will contain:
```
1 : MISS
3 : MISS
5 : MISS
1 : MISS
3 : MISS
1 : HIT
```

To view the output on Windows:
```powershell
type cache_sim_output
```

On Linux / macOS:
```bash
cat cache_sim_output
```

## Implementation Details

- Cache is initially empty (all entries invalid)
- Each cache block holds one word
- Memory references are word addresses
- LRU (Least Recently Used) replacement policy is used for eviction

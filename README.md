Multilevel LRU cache simulation (L1 and L2) in C++

CacheLine Structure: 
- Contains the tag, valid, dirty bit, and the last accessed time for LRU.

Cache Class:
- Defines the structure of a cache (size, associativity, blocks, etc.).
- Implements methods for accessing the cache, handling hits and misses, and updating the LRU for replacement.
- Handles the replacement policy (LRU).

CacheSimulator Class:
- Simulates a multi-level cache system with L1 I-cache, L1 D-cache, and L2 cache.
- Reads a trace file and processes each address accordingly, checking first L1 and then L2 cache.

Main Program:
- Configures the cache with specified size, block size, and associativity, and simulates the trace file.

Split Cache: I-cache and D-cache
Cache Sizes: 1024 to 16384 bytes
Block Sizes: 16 bytes or 64 bytes
Associativities: Direct Mapped, 2-way, 4-way, 8-way
Replacement Policies: LRU (Least Recently Used)
Multi-level Cache: L1 (I-cache, D-cache), L2 (Unified cache)
Multi-level Cache Policy: inclusive, exclusive

Prints hit and miss statistics (added counters to track these values)

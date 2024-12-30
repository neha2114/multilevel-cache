#include <iostream>
#include <vector>
#include <list>
#include <unordered_map>
#include <fstream>
#include <sstream>
#include <cmath>

using namespace std;

// Cache Line Structure
struct CacheLine {
    unsigned long long tag;
    bool valid;
    bool dirty;
    int lastUsed;  // For LRU Policy
};

// Cache Class
class Cache {
public:
    int size;            // Cache size in bytes
    int blockSize;       // Block size in bytes
    int associativity;   // Cache associativity (Direct-mapped, 2-way, etc.)
    int numBlocks;       // Number of blocks in the cache
    int numSets;         // Number of sets (for set-associative caches)
    vector<vector<CacheLine>> cache; // Cache storage
    int blockOffsetBits;
    int indexBits;
    int tagBits;
    int accessCounter;    // For LRU

    // Statistics counters
    int hits;
    int misses;
    int writeBacks;

    // Constructor
    Cache(int s, int b, int a) : size(s), blockSize(b), associativity(a), accessCounter(0), hits(0), misses(0), writeBacks(0) {
        numBlocks = size / blockSize;
        numSets = (associativity == 1) ? numBlocks : numBlocks / associativity;
        cache.resize(numSets, vector<CacheLine>(associativity, {0, false, false, 0}));

        blockOffsetBits = log2(blockSize);
        indexBits = log2(numSets);
        tagBits = 64 - blockOffsetBits - indexBits;
    }

    // Access the cache for a given address
    bool accessCache(unsigned long long address, bool isWrite, bool isDataCache, bool isL1);
    void updateLRU(int set, int way);
    unsigned long long getTag(unsigned long long address);
    unsigned long long getIndex(unsigned long long address);
    unsigned long long getBlockOffset(unsigned long long address);
    bool checkCacheHit(int set, unsigned long long tag);
    void writeBack(int set, int way);

    // Print statistics
    void printStats() {
        cout << "Hits: " << hits << endl;
        cout << "Misses: " << misses << endl;
        cout << "Write-backs: " << writeBacks << endl;
    }
};

// Cache access logic
bool Cache::accessCache(unsigned long long address, bool isWrite, bool isDataCache, bool isL1) {
    unsigned long long tag = getTag(address);
    unsigned long long index = getIndex(address);
    unsigned long long blockOffset = getBlockOffset(address);
    int set = index;

    // Search for the tag in the cache set
    for (int way = 0; way < associativity; ++way) {
        if (cache[set][way].valid && cache[set][way].tag == tag) {
            // Cache hit
            hits++;
            if (isWrite) {
                cache[set][way].dirty = true;
            }
            updateLRU(set, way);
            return true;
        }
    }

    // Cache miss
    misses++;
    if (isDataCache && isWrite && !isL1) {
        // Write-back policy: write back to L2 cache
        // (Implementation can be added for L2 cache)
    }

    // Cache replacement (LRU)
    int replaceWay = 0;
    for (int way = 1; way < associativity; ++way) {
        if (!cache[set][way].valid || cache[set][way].lastUsed < cache[set][replaceWay].lastUsed) {
            replaceWay = way;
        }
    }

    // If the line is dirty, write it back to the next level
    if (cache[set][replaceWay].valid && cache[set][replaceWay].dirty) {
        writeBack(set, replaceWay);
    }

    // Replace cache line
    cache[set][replaceWay].tag = tag;
    cache[set][replaceWay].valid = true;
    cache[set][replaceWay].dirty = isWrite;
    updateLRU(set, replaceWay);

    return false;
}

void Cache::updateLRU(int set, int way) {
    cache[set][way].lastUsed = accessCounter++;
}

unsigned long long Cache::getTag(unsigned long long address) {
    return address >> (blockOffsetBits + indexBits);
}

unsigned long long Cache::getIndex(unsigned long long address) {
    return (address >> blockOffsetBits) & ((1 << indexBits) - 1);
}

unsigned long long Cache::getBlockOffset(unsigned long long address) {
    return address & ((1 << blockOffsetBits) - 1);
}

bool Cache::checkCacheHit(int set, unsigned long long tag) {
    for (int way = 0; way < associativity; ++way) {
        if (cache[set][way].valid && cache[set][way].tag == tag) {
            return true;
        }
    }
    return false;
}

void Cache::writeBack(int set, int way) {
    // Placeholder function for writing back to next cache level (L2)
    writeBacks++;
}

// Cache Simulator
class CacheSimulator {
public:
    Cache* L1_ICache;
    Cache* L1_DCache;
    Cache* L2_Cache;

    CacheSimulator(int cacheSize, int blockSize, int associativity);
    void simulate(const string& traceFile);
    void printStats();
};

// Cache Simulator constructor
CacheSimulator::CacheSimulator(int cacheSize, int blockSize, int associativity) {
    L1_ICache = new Cache(cacheSize, blockSize, associativity); // I-cache
    L1_DCache = new Cache(cacheSize, blockSize, associativity); // D-cache
    L2_Cache = new Cache(16384, blockSize, 8); // L2 Cache
}

// Simulate cache operations based on trace file
void CacheSimulator::simulate(const string& traceFile) {
    ifstream file(traceFile);
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string operation;
        string addressStr;

        ss >> operation >> addressStr;
        unsigned long long address = stoull(addressStr, nullptr, 16);

        bool isWrite = (operation == "write");
        bool isFetch = (operation == "fetch");
        bool isDataCache = !isFetch;

        if (isFetch) {
            // Access I-cache
            L1_ICache->accessCache(address, isWrite, isDataCache, true);
        } else {
            // Access D-cache
            L1_DCache->accessCache(address, isWrite, isDataCache, true);
        }

        // If miss in L1, check L2
        if (!L1_ICache->accessCache(address, isWrite, isDataCache, true)) {
            L2_Cache->accessCache(address, isWrite, isDataCache, false);
        }
    }

    file.close();
}

void CacheSimulator::printStats() {
    cout << "L1 I-Cache:" << endl;
    L1_ICache->printStats();
    cout << endl;

    cout << "L1 D-Cache:" << endl;
    L1_DCache->printStats();
    cout << endl;

    cout << "L2 Cache:" << endl;
    L2_Cache->printStats();
}

// Main function
int main() {
    CacheSimulator simulator(1024, 16, 2); // Example: 1024 bytes cache, 16-byte blocks, 2-way associativity
    simulator.simulate("traces.txt");

    // Print the statistics for each cache
    simulator.printStats();

    return 0;
}

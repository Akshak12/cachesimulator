#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
using namespace std;

/*
 * CACHE MEMORY SIMULATOR (LRU Policy)
 * -----------------------------------
 * This program simulates how a CPU cache works using
 * the Least Recently Used (LRU) replacement policy.
 * 
 * Key Concepts:
 * - Each memory address is represented as an integer.
 * - The cache has limited capacity.
 * - When cache is full and a new address must be loaded,
 *   the least recently used address is removed.
 */

class LRUCache {
    int capacity;  // Maximum size of the cache

    // Doubly linked list to maintain usage order:
    // Front = most recently used, Back = least recently used
    list<int> cacheList;

    // Map to quickly find an address in the cache
    unordered_map<int, list<int>::iterator> cacheMap;

    int hits, misses; // For statistics

public:
    // Constructor
    LRUCache(int size) {
        capacity = size;
        hits = misses = 0;
    }

    // Function to access a memory address
    void access(int address) {
        // CASE 1: Address is already in cache → HIT
        if (cacheMap.find(address) != cacheMap.end()) {
            hits++;
            // Move it to the front (most recently used)
            cacheList.erase(cacheMap[address]);
            cacheList.push_front(address);
            cacheMap[address] = cacheList.begin();
            cout << "Accessing " << address << " -> HIT [OK]" << endl;

        } 
        // CASE 2: Address not in cache → MISS
        else {
            misses++;
            cout << "Accessing " << address << " -> MISS [X]" << endl;

            // If cache is full, remove least recently used (back)
            if ((int)cacheList.size() == capacity) {
                int leastUsed = cacheList.back();
                cacheList.pop_back();
                cacheMap.erase(leastUsed);
                cout << "Removed LRU address: " << leastUsed << endl;
            }

            // Insert new address at the front
            cacheList.push_front(address);
            cacheMap[address] = cacheList.begin();
        }
    }

    // Print current cache content
    void displayCache() {
        cout << "Current Cache State: [ ";
        for (int addr : cacheList)
            cout << addr << " ";
        cout << "]" << endl;
    }

    // Print final statistics
    void printStats() {
        cout << "\n===== CACHE STATISTICS =====" << endl;
        cout << "Total Hits: " << hits << endl;
        cout << "Total Misses: " << misses << endl;
        int total = hits + misses;
        if (total > 0)
            cout << "Hit Ratio: " << (float)hits / total << endl;
        cout << "============================" << endl;
    }
};

int main() {
    int cacheSize;
    cout << "Enter Cache Size: ";
    cin >> cacheSize;

    LRUCache cache(cacheSize);

    int n;
    cout << "Enter number of memory accesses: ";
    cin >> n;

    cout << "Enter memory addresses (integers): ";
    for (int i = 0; i < n; i++) {
        int addr;
        cin >> addr;
        cache.access(addr);
        cache.displayCache();
    }

    cache.printStats();

    return 0;
}

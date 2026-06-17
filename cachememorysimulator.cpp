#include <iostream>
#include <vector>
#include <unordered_map>
#include <list>
#include <thread>
#include <mutex>
#include <chrono>
#include <random>
#include <functional>
using namespace std;

/*
 * CACHE MEMORY SIMULATOR (LRU Policy)
 * -----------------------------------
 * Simulates how a CPU cache works using the Least Recently
 * Used (LRU) replacement policy.
 *
 * Mode 1: original single-threaded interactive mode (manual input).
 * Mode 2: multi-threaded benchmark mode. Multiple threads concurrently
 *         issue memory accesses against ONE shared cache instance,
 *         modeling multiple CPU cores/threads sharing a cache.
 *
 * Why a mutex is required:
 *   cacheList and cacheMap are mutated on every access (erase + push_front,
 *   map updates, possible eviction). If two threads call access() at the
 *   same time with no synchronization, both can interleave mid-mutation,
 *   corrupting the linked list / map (undefined behavior, possible crash)
 *   and silently losing hit/miss counts. The mutex makes each access()
 *   call atomic relative to every other thread.
 */

class LRUCache {
    int capacity;
    list<int> cacheList;
    unordered_map<int, list<int>::iterator> cacheMap;
    int hits, misses;
    mutex cacheMutex; // protects cacheList, cacheMap, hits, misses

public:
    LRUCache(int size) {
        capacity = size;
        hits = misses = 0;
    }

    // verbose defaults to true so existing single-threaded call sites
    // (cache.access(addr)) behave exactly as before.
    void access(int address, bool verbose = true) {
        lock_guard<mutex> lock(cacheMutex); // critical section starts

        if (cacheMap.find(address) != cacheMap.end()) {
            hits++;
            cacheList.erase(cacheMap[address]);
            cacheList.push_front(address);
            cacheMap[address] = cacheList.begin();
            if (verbose)
                cout << "Accessing " << address << " -> HIT [OK]" << endl;
        } else {
            misses++;
            if (verbose)
                cout << "Accessing " << address << " -> MISS [X]" << endl;

            if ((int)cacheList.size() == capacity) {
                int leastUsed = cacheList.back();
                cacheList.pop_back();
                cacheMap.erase(leastUsed);
                if (verbose)
                    cout << "Removed LRU address: " << leastUsed << endl;
            }

            cacheList.push_front(address);
            cacheMap[address] = cacheList.begin();
        }
    } // lock released automatically here (RAII)

    void displayCache() {
        lock_guard<mutex> lock(cacheMutex);
        cout << "Current Cache State: [ ";
        for (int addr : cacheList)
            cout << addr << " ";
        cout << "]" << endl;
    }

    void printStats() {
        lock_guard<mutex> lock(cacheMutex);
        cout << "\n===== CACHE STATISTICS =====" << endl;
        cout << "Total Hits: " << hits << endl;
        cout << "Total Misses: " << misses << endl;
        int total = hits + misses;
        if (total > 0)
            cout << "Hit Ratio: " << (float)hits / total << endl;
        cout << "Total Accesses Recorded: " << total << endl;
        cout << "============================" << endl;
    }

    // exposed so the benchmark can verify the correctness invariant
    int totalAccessesRecorded() {
        lock_guard<mutex> lock(cacheMutex);
        return hits + misses;
    }
};

mutex coutMutex; // std::cout itself isn't synchronized across threads,
                  // so even this status line needs its own lock to avoid
                  // garbled/interleaved output when threads finish close together.

// One worker = one simulated thread/core issuing its own access pattern
// against the SHARED cache. Each thread gets a distinct RNG seed so
// patterns differ but are reproducible.
void worker(LRUCache &cache, int threadId, int numAccesses, int addressRange) {
    mt19937 rng(threadId + 1);
    uniform_int_distribution<int> dist(0, addressRange - 1);

    for (int i = 0; i < numAccesses; i++) {
        int addr = dist(rng);
        cache.access(addr, false); // verbose=false: avoid console clutter/contention during the run
    }
    lock_guard<mutex> lk(coutMutex);
    cout << "Thread " << threadId << " finished " << numAccesses << " accesses." << endl;
}

void runMultiThreadedBenchmark() {
    int cacheSize, numThreads, accessesPerThread, addressRange;

    cout << "\n--- Multi-threaded Cache Benchmark ---\n";
    cout << "Enter Cache Size: ";
    cin >> cacheSize;
    cout << "Enter number of threads: ";
    cin >> numThreads;
    cout << "Enter accesses per thread: ";
    cin >> accessesPerThread;
    cout << "Enter address range (e.g., 100): ";
    cin >> addressRange;

    LRUCache cache(cacheSize);
    vector<thread> threads;

    auto start = chrono::high_resolution_clock::now();

    for (int i = 0; i < numThreads; i++)
        threads.emplace_back(worker, ref(cache), i, accessesPerThread, addressRange);

    for (auto &t : threads)
        t.join();

    auto end = chrono::high_resolution_clock::now();
    double elapsedMs = chrono::duration<double, milli>(end - start).count();

    cache.printStats();

    int expected = numThreads * accessesPerThread;
    int actual = cache.totalAccessesRecorded();
    cout << "Correctness check -> expected accesses: " << expected
         << " | recorded accesses: " << actual
         << (expected == actual ? "  [PASS: no lost updates]" : "  [FAIL: race condition detected]")
         << endl;

    cout << "Total time for " << numThreads << " threads: " << elapsedMs << " ms" << endl;
}

int main() {
    int mode;
    cout << "Select mode:\n";
    cout << "1. Single-threaded interactive mode (manual input)\n";
    cout << "2. Multi-threaded benchmark mode (concurrent access simulation)\n";
    cout << "Choice: ";
    cin >> mode;

    if (mode == 1) {
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
    } else {
        runMultiThreadedBenchmark();
    }

    return 0;
}

**Cache Memory Simulator (LRU)**

This is a C++ program that simulates how a CPU cache works using the LRU (Least Recently Used) replacement policy. I built the basic single-threaded version first, then added a second mode that lets multiple threads access the cache at the same time, so I could practice and demonstrate handling concurrent access safely.

**What it does**

The cache is built with a doubly linked list to track which addresses were used most recently, plus a hash map so checking whether an address is already cached is O(1) instead of scanning the whole list. When the cache is full and a new address comes in, whatever is at the back of the list gets evicted.

**There are two modes:**


**Interactive mode:** You type in a cache size and a list of memory addresses, and the program processes them one at a time, printing whether each one was a hit or a miss and what the cache looks like afterward.
**Benchmark mode:** Instead of you typing addresses, the program spins up several threads, and each thread generates and fires off its own sequence of random memory accesses, all hitting the same shared cache at the same time. At the end it prints combined hit/miss stats and how long the whole run took.



In interactive mode you'll be asked for a cache size, how many accesses you want to make, and then the addresses themselves. In benchmark mode you're asked for cache size, number of threads, accesses per thread, and the address range to randomly pick from.


**Why concurrency needed extra work**

Once multiple threads are reading and writing the same cache at the same time, things can go wrong fast. The internal linked list and hash map both get modified on every access, moving an address to the front and possibly evicting the back element. If two threads did that at the exact same moment with no protection, you'd end up with a corrupted list, a crash, or accesses that silently never got counted, because both threads could read the old state before either one finished writing.

To fix that, I added a mutex to the cache class, and every method that touches the shared list or map locks it first using lock_guard, which automatically releases the lock when the function ends, even if something goes wrong partway through. That way only one thread can actually be modifying the cache's internals at any given moment, even though many threads can be calling into it.

I didn't want to just assume this worked, so the benchmark mode checks it directly: it knows exactly how many accesses it told the threads to make in total, and compares that to how many hits plus misses actually got recorded. If the mutex weren't there, this number would sometimes come up short, because accesses would get silently lost to the race condition. In testing it always matched exactly.

One more small thing: even printing to the console isn't automatically safe across threads, so there's a second, separate lock just for the "thread finished" status messages, otherwise they can print on top of each other and look garbled.



#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <thread.h>
#include <kernel.h>
#include <debug-macros.h>

#define NR_CPUS 4

/* ---------------- fake klib and am ---------------- */

int cpu_current() {
    pthread_t pid = pthread_self();
    for (int i = 0; i < n_; ++i) {
        if (threads_[i].thread == pid) {
            return threads_[i].id;
        }
    }
}

int cpu_count() {
    return n_;
}

inline void putch(char ch) {
    putchar(ch);
}

inline int atomic_xchg(volatile int *addr, int newval) {
    int result;
    asm volatile ("lock xchg %0, %1":
        "+m"(*addr), "=a"(result) : "1"(newval) : "memory");
    return result;
}

/* --------------------------------------------- */

static inline size_t power_bound(size_t x) {
    size_t ret = 1;
    while (ret < x) ret <<= 1;
    return ret;
}

static void entry(int id) {
    while (n_ < NR_CPUS); //wait until all threads were created

    size_t random_size[10];
    void *ptr[10];
    //size_t max_size = 64 * 1024; //1 B to 64 KiB per request
    size_t max_size = 1 * 1024; //1 B to 64 KiB per request
    for (int i = 0; i < LENGTH(random_size); ++i) {
        random_size[i] = rand() % max_size + 1;
        ptr[i] = pmm->alloc(random_size[i]);
        //assert(ptr[i]);
        //assert(((uintptr_t)ptr[i] & (power_bound(random_size[i]) - 1)) == 0);
    }
}

static void alloc_test() {
    for (int i = 0; i < NR_CPUS; ++i) {
        create(entry);
    }
}

int main() {
    srand(time(0));
    pmm->init();
    alloc_test();
}
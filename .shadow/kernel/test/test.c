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
    return NR_CPUS;
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

    size_t block_size[1];
    void *ptr[1];
    //size_t max_size = 64 * 1024; //1 B to 64 KiB per request
    size_t min_size = 128;

    for (int i = 0; i < LENGTH(block_size); ++i) {
        //if (rand() & 1) {
        //block_size[i] = rand() % max_size + 1;
        //} else {
        block_size[i] = rand() % min_size + 1;
        //}
        ptr[i] = pmm->alloc(block_size[i]);

        // check if we get an available addr
        assert(ptr[i]);
        // check if we get a legal addr
        assert(((uintptr_t)ptr[i] & (power_bound(block_size[i]) - 1)) == 0);
    }

    unsigned char my_identifier = id;
    for (int i = 0; i < LENGTH(block_size); ++i) {
        // brush my range with my id
        memset(ptr[i], my_identifier, block_size[i]);
    }

    // check if anyone invades my range for 10 times
    for (int k = 0; k < 10; ++k)
        for (int i = 0; i < LENGTH(block_size); ++i) {
            for (size_t offset = 0; offset < block_size[i]; ++offset) {
                unsigned char byte_here = *(unsigned char *)(ptr[i] + offset);
                assert(byte_here == id);
            }
        }

    for (int i = 0; i < LENGTH(block_size); ++i) pmm->free(ptr[i]);
}

static void alloc_test() {
    for (int i = 0; i < NR_CPUS; ++i) {
        create(entry);
    }
}

static void simple_test() {
    n_ = 1;
    entry(1);
}

int main() {
    srand(time(0));
    pmm->init();
    // alloc_test();
    simple_test();
}
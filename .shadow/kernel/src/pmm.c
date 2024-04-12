#include <common.h>
#include <spinlock.h>
#include <debug-macros.h>

spinlock_t big_kernel_lock;

#ifdef TEST

struct heap_t {
    void *start, *end;
} heap;

#endif

static inline size_t power_bound(size_t x) {
    size_t ret = 1;
    while (ret < x) ret <<= 1;
    return ret;
}

static void *kalloc(size_t size) {
    // TODO
    // You can add more .c files to the repo.
    return NULL;
}

static void *kalloc_buddy(size_t size) {
    // TODO
    // You can add more .c files to the repo.
    return NULL;
}


static void *kalloc_stupid(size_t size) {

    spinlock_lock(&big_kernel_lock);

    size_t bound = power_bound(size);
    void *next_available = (void*)(
        (((uintptr_t)heap.start - 1) & (~(bound - 1)))
        + bound);
    assert(((uintptr_t)next_available & (bound - 1)) == 0);
    if (next_available >= heap.end) {
        spinlock_unlock(&big_kernel_lock);
        return NULL;
    }
    else {
        heap.start = next_available + size;
        LOG_RANGE(size, next_available);
        spinlock_unlock(&big_kernel_lock);
        return next_available;
    }

    return NULL;
}


static void kfree(void *ptr) {
    // TODO
    // You can add more .c files to the repo.
}

#ifndef TEST

static void pmm_init() {
    uintptr_t pmsize = (
        (uintptr_t)heap.end
        - (uintptr_t)heap.start
    );

    printf(
        "Got %d MiB heap: [%p, %p)\n",
        pmsize >> 20, heap.start, heap.end
    );

    spinlock_init(&big_kernel_lock);
}

#else

#define TEST_HEAP_SIZE 16 * 1024 * 1024 //16MiB

static void pmm_init() {
    char *ptr = malloc(TEST_HEAP_SIZE);
    heap.start = ptr;
    heap.end = ptr + TEST_HEAP_SIZE;
    
    uintptr_t pmsize = (
        (uintptr_t)heap.end
        - (uintptr_t)heap.start
    );

    printf(
        "Got %ld MiB heap: [%p, %p)\n",
        pmsize >> 20, heap.start, heap.end
    );

    spinlock_init(&big_kernel_lock);
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    //.alloc = kalloc,
    //.alloc = kalloc_stupid,
    .alloc = kalloc_buddy,
    .free  = kfree,
};

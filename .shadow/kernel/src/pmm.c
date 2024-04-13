#include <common.h>
#include <spinlock.h>
#include <chunklist.h>
#include <debug-macros.h>

spinlock_t big_kernel_lock;

#ifdef TEST

struct heap_t {
    void *start, *end;
} heap;

#endif

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
    void *next_available = align_to_bound(heap.start, bound);
    assert(((uintptr_t)next_available & (bound - 1)) == 0);
    if (next_available >= heap.end) {
        spinlock_unlock(&big_kernel_lock);
        return NULL;
    }
    else {
        heap.start = next_available + size;
        // LOG_RANGE(size, next_available);
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

    size_t bound = power_bound(pmsize);
    while (align_to_bound(heap.start, bound) >= heap.end) bound >>= 1;
    for (void *x = align_to_bound(heap.start, bound); x + bound >= heap.end; bound >>= 1);
    // TODO: make more use of heap
    heap.start = align_to_bound(heap.start, bound);
    heap.end = heap.start + bound;

    ECHO_VAR(bound, %ld);
    ECHO_VAR(heap.start, %p);
    ECHO_VAR(heap.end, %p);
    ASSERT_EQUAL(heap.end - heap.start, bound);
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    //.alloc = kalloc,
    .alloc = kalloc_stupid,
    //.alloc = kalloc_buddy,
    .free  = kfree,
};

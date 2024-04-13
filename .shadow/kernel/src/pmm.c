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

static void setup_heap_structure() {
    size_t prefix;
    void *bound;

    log_nr_page = 0;
    nr_page = 1;
    while (1) {
        prefix = 
        (nr_page) * sizeof(chunk_t) +
        (log_nr_page + 1) * sizeof(chunklist_t);
        bound = align_to_bound(heap.start + prefix, nr_page << LOG_PAGE_SIZE);
        if (bound + nr_page * PAGE_SIZE < heap.end) {
            ++log_nr_page;
            nr_page <<= 1;
        } else break;
    }
    --log_nr_page;
    nr_page >>= 1;

    chunks = heap.start;
    chunklist = heap.start + (nr_page) * sizeof(chunk_t);
    assert(chunklist == heap.start + prefix);
    // TODO: make more use of heap

    printf("we make heap to this structure:\n\n");
    printf("Manage %ld pages\n", nr_page);
    printf("[%p, %p) to store ds\n", heap.start, heap.start + prefix);
    printf("[%p, %p) to allocate\n", bound, bound + nr_page * PAGE_SIZE);
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

    /* ---------- */

    setup_heap_structure();
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    //.alloc = kalloc,
    .alloc = kalloc_stupid,
    //.alloc = kalloc_buddy,
    .free  = kfree,
};

#include <assert.h>
#include <common.h>
#include <slab.h>
#include <buddy.h>
#include <chunklist.h>

#ifdef TEST

struct heap_t {
    void *start, *end;
} heap;

#endif

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

static void *kalloc(size_t size) {
    if (size > REJECT_THRESHOLD) return NULL;

    size = power_bound(size);
    if (size >= PAGE_SIZE / 2) {
        return buddy_alloc(size);
    }
}

static void kfree(void *ptr) {
    if ((((uintptr_t)ptr) & (PAGE_SIZE - 1)) == 0) {
        // aligned to page, it must be allocate by buddy
        buddy_free(ptr);
        return;
    }
}

static void setup_heap_layout() {
    // TODO: make more use of heap
    size_t prefix;
    void *bound;

    log_nr_page = 0;
    nr_page = 1;
    while (1) {
        prefix = 
        (nr_page) * sizeof(chunk_t) +
        (log_nr_page + 1) * sizeof(chunklist_t) +
        (cpu_count() * (SLAB_LEVEL_MAXIMAL - SLAB_LEVEL_MINIMAL + 1)) * sizeof(slab_t);
        bound = align_to_bound(heap.start + prefix, nr_page << LOG_PAGE_SIZE);
        if (bound + nr_page * PAGE_SIZE < heap.end) {
            ++log_nr_page;
            nr_page <<= 1;
        } else break;
    }
    --log_nr_page;
    nr_page >>= 1;

    chunks = heap.start;
    chunklist = (void *)chunks + nr_page * sizeof(chunk_t);
    slabs = (void *)chunklist + (log_nr_page + 1) * sizeof(chunklist_t);
    mem = align_to_bound(chunklist + (log_nr_page + 1) * sizeof(chunklist_t),
                         nr_page << LOG_PAGE_SIZE);

    printf("\nwe make heap to this structure:\n\n");
    printf("Manage %ld pages\n", nr_page);
    printf("[%p, %p) to store chunks\n", chunks, chunks + nr_page);
    printf("[%p, %p) to store chunklist\n", chunklist, chunklist + (log_nr_page + 1));
    printf("[%p, %p) to store slabs\n", slabs, slabs + (cpu_count() * (SLAB_LEVEL)));
    printf("[%p, %p) to allocate\n\n", mem, mem + nr_page * PAGE_SIZE);
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

    /* ---------- */

    setup_heap_layout();
    chunk_init();
    buddy_init();
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};

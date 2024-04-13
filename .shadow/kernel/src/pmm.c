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
    if (size > REJECT_THRESHOLD) return NULL;

    size = (size - 1) / PAGE_SIZE + 1;
    spinlock_lock(&big_kernel_lock);

    int expected_level = level_bound(size);
    if (!chunklist[expected_level].head) {
        int level = expected_level;
        ++level;
        while (level <= log_nr_page && !chunklist[level].head) ++level;
        if (level > log_nr_page) {
            //no more chunk to split
            spinlock_unlock(&big_kernel_lock);
            return NULL;
        }
        assert(chunklist[level].head);
        chunk_t *bigger_chunk = chunklist[level].head;
        size_t bigger_id = get_chunk_id(bigger_chunk);
        chunk_remove(bigger_id);
        while (level > expected_level) {
            size_t bigger_size = bigger_chunk->size;
            size_t r_id = bigger_id + (bigger_size / 2);
            chunks[r_id].size = bigger_size / 2;
            bigger_chunk->size = bigger_size / 2;

            chunk_insert(level - 1, r_id);
            
            --level;
        }
        chunk_insert(level, bigger_id);
    }
    chunk_t *chunk = chunklist[expected_level].head;
    size_t chunk_id = get_chunk_id(chunk);
    chunk_remove(chunk_id);
    chunk->status = CHUNK_USING;
    spinlock_unlock(&big_kernel_lock);
    LOG_RANGE(size * PAGE_SIZE, mem + (chunk_id * PAGE_SIZE));
    return mem + (chunk_id * PAGE_SIZE);
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
        spinlock_unlock(&big_kernel_lock);
        return next_available;
    }

    return NULL;
}


static void kfree(void *ptr) {
    // TODO
    // You can add more .c files to the repo.
}

static void kfree_buddy(void *ptr) {
    spinlock_lock(&big_kernel_lock);

    size_t chunk_id = (ptr - mem) / PAGE_SIZE;

    chunks[chunk_id].status = CHUNK_FREE;
    size_t buddy_id = get_buddy_id(chunk_id);
    int level = level_bound(chunks[chunk_id].size);

    while (level < log_nr_page &&
           chunks[buddy_id].size == chunks[chunk_id].size &&
           chunks[buddy_id].status == CHUNK_FREE) {
           //not be splited and free

        chunk_remove(buddy_id);
        chunk_remove(chunk_id);

        if (buddy_id < chunk_id) chunk_id = buddy_id;

        chunks[chunk_id].size *= 2;
        chunk_insert(level + 1, chunk_id);
        buddy_id = get_buddy_id(chunk_id);
        ++level;
    }

    spinlock_unlock(&big_kernel_lock);
}

static void setup_heap_structure() {
    // TODO: make more use of heap
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
    chunklist = heap.start + nr_page * sizeof(chunk_t);
    mem = align_to_bound(chunklist + (log_nr_page + 1) * sizeof(chunklist_t),
                         nr_page << LOG_PAGE_SIZE);

    printf("\nwe make heap to this structure:\n\n");
    printf("Manage %ld pages\n", nr_page);
    printf("[%p, %p) to store chunks\n", chunks, chunks + nr_page);
    printf("[%p, %p) to store chunklist\n", chunklist, chunklist + (log_nr_page + 1));
    printf("[%p, %p) to allocate\n", mem, mem + nr_page * PAGE_SIZE);
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
    chunk_init();
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    //.alloc = kalloc,
    //.alloc = kalloc_stupid,
    .alloc = kalloc_buddy,
    //.free  = kfree,
    .free  = kfree_buddy,
};

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
    /*spinlock_lock(&big_kernel_lock);
    printf("[kalloc] cpu%d wants %ld bytes\n", cpu_current(), size);
    spinlock_unlock(&big_kernel_lock);*/
    if (size > REJECT_THRESHOLD) return NULL;

    size = power_bound(size);
    if (size >= PAGE_SIZE / 2) { //slow path
        return buddy_alloc(size);
    } else {
        return slab_allocate(size); //fast path
    }
}

static void kfree(void *ptr) {
    /*spinlock_lock(&big_kernel_lock);
    printf("[kfree] cpu%d free %p\n", cpu_current(), ptr);
    spinlock_unlock(&big_kernel_lock);*/
    if ((((uintptr_t)ptr) & (PAGE_SIZE - 1)) == 0) {
        // aligned to page, it must be allocate by buddy
        buddy_free(ptr);
    } else {
        slab_free(ptr);
    }
}

static void setup_heap_layout() {
    // TODO: make more use of heap
    log_nr_page      = 12; //16MiB, 4096 pages
    void *mem_end;
    while (1) {
    mem_end          = (void *)((uintptr_t)heap.end & (~((1 << log_nr_page) * PAGE_SIZE - 1)));
    nr_page          = (mem_end - heap.start -
                       (log_nr_page + 1) * sizeof(chunklist_t) -
                       (cpu_count() * (SLAB_LEVEL)) * sizeof(slab_t))
                       / ((1 << log_nr_page) * (sizeof(chunk_t) + PAGE_SIZE)) * (1 << log_nr_page);
    if (nr_page > 0) break;
    else --log_nr_page;
    }
    chunklist        = heap.start;
    slabs            = (void *)chunklist + (log_nr_page + 1) * sizeof(chunklist_t);
    chunks           = (void *)slabs + (cpu_count() * (SLAB_LEVEL)) * sizeof(slab_t);
    mem              = mem_end - PAGE_SIZE * nr_page;
    for (int i = 0; i <= log_nr_page; ++i)
        chunklist[i].head = NULL;
    for (size_t i = 0; i < nr_page; ++i)
        chunks[i] = (chunk_t){
            .status = CHUNK_FREE,
            .size = 0,
            .next = NULL,
            .prev = NULL
        };
    for (int i = 0; i < nr_page / (1 << log_nr_page); ++i) {
        chunks[i * (1 << log_nr_page)].size = (1 << log_nr_page);
        chunk_insert(log_nr_page, i * (1 << log_nr_page));
    }
    int temp_log = log_nr_page - 1;
    while (1) {
        while ((1 << temp_log) * sizeof(chunk_t) > mem - ((void*)chunks + nr_page * sizeof(chunk_t)) ||
               (1 << temp_log) * PAGE_SIZE > heap.end - (mem + nr_page * PAGE_SIZE))
               --temp_log;

        if (temp_log < 0) break;
        else {
            
            printf("ahead %d\n", temp_log);
            printf("chunks extend [%p, %p)\n", ((void*)chunks + nr_page * sizeof(chunk_t)),
                                               ((void*)chunks + (nr_page + (1 << temp_log)) * sizeof(chunk_t))                    );
            printf("mem extend [%p, %p)\n", (mem + nr_page * PAGE_SIZE),
                                            (mem + (nr_page + (1 << temp_log)) * PAGE_SIZE));
            nr_page += (1 << temp_log);
        }
    }
    /*size_t prefix;
    void *bound;

    log_nr_page = 0;
    nr_page = 1;
    while (1) {
        prefix = 
        (nr_page) * sizeof(chunk_t) +
        (log_nr_page + 1) * sizeof(chunklist_t) +
        (cpu_count() * (SLAB_LEVEL)) * sizeof(slab_t);
        bound  = align_to_bound(heap.start + prefix, nr_page << LOG_PAGE_SIZE);
        if (bound + nr_page * PAGE_SIZE < heap.end) {
            ++log_nr_page;
            nr_page <<= 1;
        } else break;
    }
    --log_nr_page;
    nr_page >>= 1;

    chunks    = heap.start;
    chunklist = (void *)chunks + nr_page * sizeof(chunk_t);
    slabs     = (void *)chunklist + (log_nr_page + 1) * sizeof(chunklist_t);
    mem       = align_to_bound(chunklist + (log_nr_page + 1) * sizeof(chunklist_t),
                               nr_page << LOG_PAGE_SIZE);*/

    printf("\nwe make heap to this structure:\n\n");
    printf("Manage %ld pages\n", nr_page);
    printf("[%p, %p) to store chunklist\n", chunklist, chunklist + (log_nr_page + 1));
    printf("[%p, %p) to store slabs\n", slabs, slabs + (cpu_count() * (SLAB_LEVEL)) * sizeof(slab_t));
    printf("[%p, %p) to store chunks\n", chunks, chunks + nr_page);
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

#define TEST_HEAP_SIZE 32 * 1024 * 1024 //32MiB

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
    //chunk_init();
    buddy_init();
    slab_init();
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};

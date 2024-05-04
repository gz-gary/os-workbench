#include <common.h>
#include <slab.h>
#include <buddy.h>
#include <chunklist.h>

#ifdef TEST

struct heap_t {
    void *start, *end;
} heap;

#endif

static void *kalloc(size_t size) {
    if (size > REJECT_THRESHOLD) return NULL;

    size = power_bound(size);
    if (size >= PAGE_SIZE / 2) { //slow path
        return buddy_alloc(size);
    } else { //fast path
        return slab_allocate(size);
    }
}

static void kfree(void *ptr) {
    if ((((uintptr_t)ptr) & (PAGE_SIZE - 1)) == 0) {
        // aligned to page, it must be allocate by buddy
        buddy_free(ptr);
    } else {
        slab_free(ptr);
    }
}

static void setup_heap_layout() {
    // TODO: make more use of heap
    log_nr_page      = 12; //16MiB, 4096 pages, reject allocation of higher level
    void *mem_end;
    while (1) {
        mem_end = (void *)((uintptr_t)heap.end & (~((1 << log_nr_page) * PAGE_SIZE - 1)));
        nr_page = (mem_end - heap.start -
                  (log_nr_page + 1) * sizeof(chunklist_t) -
                  (cpu_count() * (SLAB_LEVEL)) * sizeof(slab_t))
                  / ((1 << log_nr_page) * (sizeof(chunk_t) + PAGE_SIZE)) * (1 << log_nr_page);
        if (nr_page > 0) break;
        else --log_nr_page;
    }
    chunklist = heap.start;
    slabs     = (void *)chunklist + (log_nr_page + 1) * sizeof(chunklist_t);
    chunks    = (void *)slabs + (cpu_count() * (SLAB_LEVEL)) * sizeof(slab_t);
    mem       = mem_end - PAGE_SIZE * nr_page;
    /*printf("\nwe make heap to this structure:\n\n");
    printf("Manage %d pages\n", nr_page);
    printf("[%p, %p) to store chunklist\n", chunklist, chunklist + (log_nr_page + 1));
    printf("[%p, %p) to store slabs\n", slabs, slabs + (cpu_count() * (SLAB_LEVEL)) * sizeof(slab_t));
    printf("[%p, %p) to store chunks\n", chunks, chunks + nr_page);
    printf("[%p, %p) to allocate\n\n", mem, mem + nr_page * PAGE_SIZE);*/
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
    /*int temp_log = log_nr_page - 1;
    while (1) {
        while (temp_log >= 0 &&
               ((1 << temp_log) * sizeof(chunk_t) > mem - ((void*)chunks + nr_page * sizeof(chunk_t)) ||
               (1 << temp_log) * PAGE_SIZE > heap.end - (mem + nr_page * PAGE_SIZE)))
               --temp_log;

        if (temp_log < 0) break;
        else {
            
            printf("ahead %d %d\n", temp_log, 1 << temp_log);
            printf("chunks extend [%p, %p)\n", ((void*)chunks + nr_page * sizeof(chunk_t)),
                                               ((void*)chunks + (nr_page + (1 << temp_log)) * sizeof(chunk_t)));
            printf("mem extend [%p, %p)\n", (mem + nr_page * PAGE_SIZE),
                                            (mem + (nr_page + (1 << temp_log)) * PAGE_SIZE));
            for (size_t i = nr_page; i < nr_page + (1 << temp_log); ++i)
                chunks[i] = (chunk_t){
                    .status = CHUNK_FREE,
                    .size = 0,
                    .next = NULL,
                    .prev = NULL
                };
            chunks[nr_page].size = (1 << temp_log);
            chunk_insert(temp_log, nr_page);
            nr_page += (1 << temp_log);
        }
    }*/
    // buddy_dump();
    //printf("mem we use %d MiB\n", (nr_page * PAGE_SIZE) >> 20);
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

    setup_heap_layout();
    buddy_init();
    slab_init();
}

#else

#define TEST_HEAP_SIZE 16 * 1024 * 1024 //32MiB

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
    buddy_init();
    slab_init();
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};

#include <common.h>
#include <spinlock.h>

spinlock_t big_kernel_lock;

#ifdef TEST

struct heap_t {
    void *start, *end;
} heap;

#endif

static void *kalloc(size_t size) {
    // TODO
    // You can add more .c files to the repo.
    spinlock_lock(&big_kernel_lock);
    printf("There are %d cpus now\n", cpu_count());
    printf("cpu[%d] wants a block of %d bytes\n\n", cpu_current(), size);
    spinlock_unlock(&big_kernel_lock);

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
        "Got %d MiB heap: [%p, %p)\n",
        pmsize >> 20, heap.start, heap.end
    );

    spinlock_init(&big_kernel_lock);
}

#endif

MODULE_DEF(pmm) = {
    .init  = pmm_init,
    .alloc = kalloc,
    .free  = kfree,
};

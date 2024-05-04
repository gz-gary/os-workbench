#include <common.h>
#include <buddy.h>
#include <chunklist.h>

spinlock_t big_kernel_lock;

void *buddy_alloc(size_t size) {
    size = (size - 1) / PAGE_SIZE + 1;
    spinlock_lock(&big_kernel_lock);

    int expected_level = level_bound(size);
    if (!chunklist[expected_level].head) {
        int level = expected_level + 1;
        while (level <= log_nr_page && !chunklist[level].head) ++level;
        if (level > log_nr_page) { //no more chunk to split
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
            assert(get_buddy_id(r_id) == bigger_id);
            assert(get_buddy_id(bigger_id) == r_id);

            chunk_insert(level - 1, r_id);
            --level;
        }
        chunk_insert(level, bigger_id);
    }
    assert(chunklist[expected_level].head);
    chunk_t *chunk = chunklist[expected_level].head;
    size_t chunk_id = get_chunk_id(chunk);
    chunk_remove(chunk_id);

    chunk->status = CHUNK_USING;

    // LOG_RANGE(chunks[chunk_id].size * PAGE_SIZE, mem + (chunk_id * PAGE_SIZE));

    spinlock_unlock(&big_kernel_lock);
    return mem + (chunk_id * PAGE_SIZE);
}

void buddy_free(void *ptr) {
    spinlock_lock(&big_kernel_lock);

    size_t chunk_id = (ptr - mem) / PAGE_SIZE;

    chunks[chunk_id].status = CHUNK_FREE;
    size_t buddy_id = get_buddy_id(chunk_id);
    int level = level_bound(chunks[chunk_id].size);

    while (level < log_nr_page &&
           buddy_id < nr_page &&
           chunks[buddy_id].size == chunks[chunk_id].size &&
           chunks[buddy_id].status == CHUNK_FREE) {
           //not out of range && not be splited && free

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

void buddy_init() {
    spinlock_init(&big_kernel_lock);
}

void buddy_dump() {
}
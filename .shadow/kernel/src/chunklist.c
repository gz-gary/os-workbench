// free space list
#include <chunklist.h>
#include <util-funcs.h>

#define NULL 0

chunk_t *chunks;
chunklist_t *chunklist;
size_t nr_page;
int log_nr_page;
void *mem;

void chunk_insert(int level, size_t chunk_id) {
    chunk_t *chunk = &chunks[chunk_id];
    chunk->next = chunklist[level].head;
    if (chunklist[level].head)
        chunklist[level].head->prev = chunk;
    chunklist[level].head = chunk;
}

void chunk_remove(size_t chunk_id) {
    chunk_t *chunk = &chunks[chunk_id];
    int level = level_bound(chunk->size);
    if (chunk->next)
        chunk->next->prev = chunk->prev;
    if (chunk->prev)
        chunk->prev->next = chunk->next;
    if (chunklist[level].head == chunk)
        chunklist[level].head = chunk->next;
    chunk->next = NULL;
    chunk->prev = NULL;
}

void chunk_init() {
    for (size_t i = 0; i < nr_page; ++i)
        chunks[i] = (chunk_t){
            .status = CHUNK_FREE,
            .size = 0, //undefined except chunk[0]
            .next = NULL,
            .prev = NULL
        };
    chunks[0].size = nr_page;
    for (int i = 0; i <= log_nr_page; ++i)
        chunklist[i].head = NULL;
    chunk_insert(log_nr_page, 0);
}

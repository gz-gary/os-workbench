// free space list
#include <assert.h>
#include <common.h>
#include <chunklist.h>

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
    assert(!chunk->next || (chunk->next->prev == chunk));
    assert(!chunk->prev || (chunk->prev->next == chunk));
}

void chunk_remove(size_t chunk_id) {
    chunk_t *chunk = &chunks[chunk_id];
    assert(!chunk->next || (chunk->next->prev == chunk));
    assert(!chunk->prev || (chunk->prev->next == chunk));
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

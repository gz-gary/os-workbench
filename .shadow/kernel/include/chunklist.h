#include <sys/types.h>

#define CHUNK_USING 0
#define CHUNK_FREE 1
#ifndef PAGE_SIZE
#define PAGE_SIZE (4 * 1024)
#endif
#define LOG_PAGE_SIZE (12)
#define REJECT_THRESHOLD (16 * 1024 * 1024)
// #define MAX_NR_PAGE REJECT_THRESHOLD / PAGE_SIZE

typedef struct chunk_t chunk_t;
typedef struct chunklist_t chunklist_t;

struct chunk_t {
    int status;
    size_t size;
    chunk_t *next, *prev;
};

struct chunklist_t {
    chunk_t *head;
};

extern chunk_t *chunks;
extern chunklist_t *chunklist;
extern size_t nr_page;
extern int log_nr_page;
extern void *mem;
extern void chunk_insert(int level, size_t chunk_id);
extern void chunk_remove(size_t chunk_id);

static inline size_t get_chunk_id(chunk_t *chunk) {
    return chunk - chunks;
}

static inline size_t get_buddy_id(size_t chunk_id) {
    return chunk_id ^ (chunks[chunk_id].size);
}
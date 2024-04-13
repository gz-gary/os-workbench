#include <sys/types.h>

#define PAGE_SIZE 4 * 1024
#define REJECT_THRESHOLD 16 * 1024 * 1024

typedef struct chunk_t chunk_t;

struct chunk_t {
    size_t size;
    chunk_t *next, *prev;
};

extern chunk_t *chunks;
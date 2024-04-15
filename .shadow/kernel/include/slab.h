#include <sys/types.h>

typedef struct slab_hdr_t slab_hdr_t;
typedef struct piece_t piece_t;
typedef struct slab_t slab_t;

struct slab_hdr_t {
    void *mem;
    size_t size;
};

struct piece_t {
    struct piece_t *next;
};

struct slab_t {
    struct piece_t *head;
};

//from 8KiB to 1024KiB
//level from 3 to 10
//use 0 to 7 to represent
#define SLAB_LEVEL_MAXIMAL 10
#define SLAB_LEVEL_MINIMAL 3

extern slab_t **slabs;

extern void *slab_allocate();
extern void slab_free(void *ptr);

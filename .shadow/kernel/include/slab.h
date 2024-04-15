#include <sys/types.h>

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

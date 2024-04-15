#include <assert.h>
#include <common.h>
#include <slab.h>
#include <buddy.h>

slab_t *(*slabs);

static inline void *get_page_start(void *ptr) {
    return (void *)(((uintptr_t)ptr) & (~(PAGE_SIZE - 1)));
}

static void fetch_slab(slab_t *slab, size_t size) {
    // end - x * size == start + prefix + x * sizeof(piece_t)

    slab_hdr_t *hdr = buddy_alloc(PAGE_SIZE);
    int nr_pieces   = (PAGE_SIZE - sizeof(slab_hdr_t)) / (size + sizeof(piece_t));
    piece_t *piece  = (void *)hdr + sizeof(slab_hdr_t);
    hdr->size       = size;
    hdr->mem        = (void *)hdr + PAGE_SIZE - nr_pieces * size;

    for (int i = 0; i < nr_pieces; ++i) {
        piece[i].next = slab->head;
        slab->head = &piece[i];
    }
}

void *slab_allocate(size_t size) {
    if (size < (1 << SLAB_LEVEL_MINIMAL))
        size = (1 << SLAB_LEVEL_MINIMAL);

    int level    = level_bound(size);
    int cpu      = cpu_current();
    slab_t *slab = &slabs[cpu][level - SLAB_LEVEL_MINIMAL];

    if (!slab->head)
        fetch_slab(slab, size);

    piece_t    *piece  = slab->head;
    slab_hdr_t *hdr    = get_page_start(piece);
    int        idx     = ((void *)piece - ((void *)hdr + sizeof(slab_hdr_t))) / sizeof(piece_t);
    void       *ret    = hdr->mem + idx * hdr->size;

    slab->head = piece->next;
    piece->next = NULL;

    return ret;
}

void slab_free(void *ptr) {
    slab_hdr_t *hdr   = get_page_start(ptr);
    size_t     size   = hdr->size;
    int        level  = level_bound(size);
    int        cpu    = cpu_current();
    slab_t     *slab  = &slabs[cpu][level - SLAB_LEVEL_MINIMAL];
    piece_t    *piece = (void *)hdr + sizeof(slab_hdr_t);
    int        idx    = (ptr - hdr->mem) / size;

    piece[idx].next = slab->head;
    slab->head = &piece[idx];
}

void slab_init() {
    int cpu_cnt = cpu_count();
    for (int i = 0; i < cpu_cnt; ++i)
        for (int j = 0; j < SLAB_LEVEL; ++j) {
            printf("%d %d %p\n", i, j, &slabs[i][j]);
            //slabs[i][j].head = NULL;
        }
}
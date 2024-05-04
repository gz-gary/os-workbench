#include <common.h>
#include <slab.h>
#include <buddy.h>

void *slabs;

static inline slab_t *locate_slab(int cpu, int level) {
    return slabs + (cpu * SLAB_LEVEL + level) * sizeof(slab_t);
}

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

    spinlock_lock(&slab->lock);
    for (int i = 0; i < nr_pieces; ++i) {
        piece[i].next = slab->head;
        slab->head = &piece[i];
    }
    spinlock_unlock(&slab->lock);

    slab_dump();
}

void *slab_allocate(size_t size) {
    if (size < (1 << SLAB_LEVEL_MINIMAL))
        size = (1 << SLAB_LEVEL_MINIMAL);

    int level    = level_bound(size);
    int cpu      = cpu_current();
    slab_t *slab = locate_slab(cpu, level - SLAB_LEVEL_MINIMAL);

    if (!slab->head)
        fetch_slab(slab, size);

    piece_t    *piece  = slab->head;
    slab_hdr_t *hdr    = get_page_start(piece);
    int        idx     = ((void *)piece - ((void *)hdr + sizeof(slab_hdr_t))) / sizeof(piece_t);
    void       *ret    = hdr->mem + idx * hdr->size;

    spinlock_lock(&slab->lock);
    slab->head = piece->next;
    piece->next = NULL;
    spinlock_unlock(&slab->lock);

    return ret;
}

void slab_free(void *ptr) {
    slab_hdr_t *hdr   = get_page_start(ptr);
    size_t     size   = hdr->size;
    int        level  = level_bound(size);
    int        cpu    = cpu_current();
    slab_t     *slab  = locate_slab(cpu, level - SLAB_LEVEL_MINIMAL);
    piece_t    *piece = (void *)hdr + sizeof(slab_hdr_t);
    int        idx    = (ptr - hdr->mem) / size;

    spinlock_lock(&slab->lock);
    piece[idx].next = slab->head;
    slab->head = &piece[idx];
    spinlock_unlock(&slab->lock);
}

void slab_init() {
    int cpu_cnt = cpu_count();
    for (int i = 0; i < cpu_cnt; ++i)
        for (int j = 0; j < SLAB_LEVEL; ++j) {
            slab_t *slab = locate_slab(i, j);
            slab->head = NULL;
            spinlock_init(&slab->lock);
        }
}

void slab_dump() {
    for (int j = 0; j < SLAB_LEVEL; ++j) {
        printf("slab level %d size %d\n", j, (1 << (j + SLAB_LEVEL_MINIMAL)));
        slab_t *slab = locate_slab(0, j);
        for (piece_t *piece = slab->head; piece; piece = piece->next) {
            slab_hdr_t *hdr = get_page_start(piece);
            int idx = ((void *)piece - ((void *)hdr + sizeof(slab_hdr_t))) / sizeof(piece_t);
            printf("[%p, %p) ", hdr->mem + idx * hdr->size, hdr->mem + (idx + 1) * hdr->size);
        }
        printf("\n");
    }
}
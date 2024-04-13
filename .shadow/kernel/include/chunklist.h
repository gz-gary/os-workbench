#include <sys/types.h>

#define PAGE_SIZE 4 * 1024
#define LOG_PAGE_SIZE 12
#define REJECT_THRESHOLD 16 * 1024 * 1024
// #define MAX_NR_PAGE REJECT_THRESHOLD / PAGE_SIZE

typedef struct chunk_t chunk_t;
typedef struct chunklist_t chunklist_t;

struct chunk_t {
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
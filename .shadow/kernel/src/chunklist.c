// free space list
#include <chunklist.h>

chunk_t *chunks;
chunklist_t *chunklist;
size_t nr_page;
int log_nr_page;
void *mem;
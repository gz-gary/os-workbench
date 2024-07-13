#include <sys/types.h>
#include <spinlock.h>

extern struct spinlock big_kernel_lock;

extern void *buddy_alloc(size_t size);
extern void buddy_free(void *ptr);
extern void buddy_init();

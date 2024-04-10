#include <common.h>
#include <spinlock.h>

int shared_counter;
spinlock_t big_kernel_lock;

static void os_init() {
    pmm->init();
    spinlock_init(&big_kernel_lock);
    shared_counter = 0;
}

static void os_run() {
    for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }
    while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};

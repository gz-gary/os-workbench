//#include <assert.h>
#include "am.h"
#include <spinlock.h>
#include <common.h>

int shared_counter;
spinlock_t big_kernel_lock;

static void os_init() {
    pmm->init();
}

static void os_run() {
    //for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        //putch(*s == '*' ? '0' + cpu_current() : *s);
    //}
    shared_counter = 0;
    spinlock_init(&big_kernel_lock);
    while (shared_counter <= 300) {
        spinlock_lock(&big_kernel_lock);
        if (big_kernel_lock.owner != cpu_current()) {
            printf("no\n");
        }
        assert(big_kernel_lock.owner == cpu_current());
        //++shared_counter;
        //printf("cpu%d add counter to -> %d\n", cpu_current(), shared_counter);
        spinlock_unlock(&big_kernel_lock);
    }
    while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};

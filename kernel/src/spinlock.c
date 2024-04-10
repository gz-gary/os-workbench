// spin lock
#include <am.h>
#include <spinlock.h>

#define NOT_FREE 0
#define FREE 1

void spinlock_init(spinlock_t *spinlock) {
    spinlock->flag = FREE;
}

void spinlock_lock(spinlock_t *spinlock) {
    while (atomic_xchg(&spinlock->flag, NOT_FREE) == NOT_FREE)
        ; //spin wait
    spinlock->owner = cpu_current();
}

void spinlock_unlock(spinlock_t *spinlock) {
    spinlock->flag = FREE;
}
// spin lock
#include <common.h>
#include <am.h>
#include <spinlock.h>

#define NOT_FREE 0
#define FREE 1

void spinlock_init(struct spinlock *spinlock) {
    spinlock->flag = FREE;
    spinlock->owner = -1;
}

void spinlock_lock(struct spinlock *spinlock) {
    while (atomic_xchg(&spinlock->flag, NOT_FREE) == NOT_FREE)
        ; //spin wait
    spinlock->owner = cpu_current();
}

void spinlock_unlock(struct spinlock *spinlock) {
    spinlock->flag = FREE;
}

bool holding(struct spinlock *spinlock) {
    return (
        spinlock->flag == NOT_FREE
        && spinlock->owner == cpu_current()
    );
}

#ifndef SPINLOCK_H
#define SPINLOCK_H

struct spinlock {
    int flag;
    int owner;
};

void spinlock_init(spinlock_t *spinlock);

void spinlock_lock(spinlock_t *spinlock);

void spinlock_unlock(spinlock_t *spinlock);

#endif

#ifndef SPINLOCK_H
#define SPINLOCK_H

#define SPINLOCK_NOT_FREE 0
#define SPINLOCK_FREE 1

#include <am.h>

struct spinlock {
    int flag;
    int owner;
    const char *name;
};

void spinlock_init(struct spinlock *spinlock, const char *name);

void spinlock_lock(struct spinlock *spinlock);

void spinlock_unlock(struct spinlock *spinlock);

bool holding(struct spinlock *spinlock);

#endif

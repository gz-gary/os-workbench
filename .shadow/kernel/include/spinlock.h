#ifndef SPINLOCK_H
#define SPINLOCK_H

struct spinlock {
    int flag;
    int owner;
};

void spinlock_init(struct spinlock *spinlock);

void spinlock_lock(struct spinlock *spinlock);

void spinlock_unlock(struct spinlock *spinlock);

#endif

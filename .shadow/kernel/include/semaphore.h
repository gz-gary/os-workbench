#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <queue.h>
#include <spinlock.h>

struct semaphore {
    queue_t queue;
    struct spinlock lock;
    int value;
    const char *name;
};

void semaphore_init(struct semaphore *sem, const char *name, int value);

void semaphore_wait(struct semaphore *sem);

void semaphore_signal(struct semaphore *sem);

#endif

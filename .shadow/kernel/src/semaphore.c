#include <semaphore.h>

void semaphore_init(struct semaphore *sem, const char *name, int value) {
    spinlock_init(&sem->lock, name);
    sem->name = name;
    sem->value = value;
}

void semaphore_wait(struct semaphore *sem) {
    spinlock_lock(&sem->lock);
    while (sem->value < 1) {
        spinlock_unlock(&sem->lock);
        spinlock_lock(&sem->lock);
    }
    --sem->value;
    spinlock_unlock(&sem->lock);
}

void semaphore_signal(struct semaphore *sem) {
    spinlock_lock(&sem->lock);
    ++sem->value;
    spinlock_unlock(&sem->lock);
}


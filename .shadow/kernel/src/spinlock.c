// spin lock
#include <common.h>
#include <os.h>

extern struct cpu_info_t cpu_info[CPUS_LIMIT];

static void push_off(void) {
    int old = ienabled();
    iset(false);

    struct cpu_info_t *c = &cpu_info[cpu_current()];
    if (c->noff == 0) {
        c->intena = old;
    }
    c->noff += 1;
}

static void pop_off(void) {
    struct cpu_info_t *c = &cpu_info[cpu_current()];

    // Never enable interrupt when holding a lock.
    if (ienabled()) {
        panic("pop_off - interruptible");
    }
    
    if (c->noff < 1) {
        panic("pop_off");
    }

    c->noff -= 1;
    if (c->noff == 0 && c->intena) {
        iset(true);
    }
}

void spinlock_init(struct spinlock *spinlock, const char *name) {
    spinlock->flag = SPINLOCK_FREE;
    spinlock->owner = INVALID_CPU;
    spinlock->name = name;
}

void spinlock_lock(struct spinlock *spinlock) {
    push_off();
    if (holding(spinlock)) {
        panic("Lock again after acquired!");
    }

    while (atomic_xchg(&spinlock->flag, SPINLOCK_NOT_FREE) == SPINLOCK_NOT_FREE)
        ; //spin wait
    spinlock->owner = cpu_current();
}

void spinlock_unlock(struct spinlock *spinlock) {
    if (!holding(spinlock)) {
        panic("Unlock before acquired!");
    }

    spinlock->owner = INVALID_CPU;
    atomic_xchg(&spinlock->flag, SPINLOCK_FREE);
    pop_off();
}

bool holding(struct spinlock *spinlock) {
    return (
        spinlock->flag == SPINLOCK_NOT_FREE
        && spinlock->owner == cpu_current()
    );
}

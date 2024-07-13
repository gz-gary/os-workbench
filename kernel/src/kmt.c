#include <os.h>

int cnt_tasks = 0;
task_t *tasks[TASKS_LIMIT];
task_t *current[CPUS_LIMIT];
spinlock_t lock_tasks_list;

struct cpu_info_t {
    int noff;
    int intena;
} cpu_info[CPUS_LIMIT];

static void push_off(void) {
    int old = ienabled();
    struct cpu_info_t *c = &cpu_info[cpu_current()];

    iset(false);
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

static void kmt_init() {
    kmt->spin_init(&lock_tasks_list, "tasks list");
    for (int i = 0; i < CPUS_LIMIT; ++i) {
        cpu_info[i] = (struct cpu_info_t) {
            .noff = 0,
            .intena = true
        };
        current[i] = NULL;
    }
    for (int i = 0; i < TASKS_LIMIT; ++i) {
        tasks[i] = NULL;
    }
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
    task->status = TASK_RUNABLE;
    kmt->spin_lock(&lock_tasks_list);

    kmt->spin_unlock(&lock_tasks_list);
    return 0;
}

static void kmt_teardown(task_t *task) {
}

static void kmt_spin_init(spinlock_t *lk, const char *name) {
    spinlock_init(lk);
    lk->name = name;
}

static void kmt_spin_lock(spinlock_t *lk) {
    push_off();

    // This is a deadlock.
    if (holding(lk)) {
        panic("deadlock");
    }

    spinlock_lock(lk);
}

static void kmt_spin_unlock(spinlock_t *lk) {
    if (!holding(lk)) {
        panic("release a lock not acquired");
    }

    spinlock_unlock(lk);

    pop_off();
}

static void kmt_sem_init(sem_t *sem, const char *name, int value) {

}
static void kmt_sem_wait(sem_t *sem) {

}
static void kmt_sem_signal(sem_t *sem) {

}

static int kmt_get_next_task(int tid) {
    kmt->spin_lock(&lock_tasks_list);
    int nxt_tid = tid < cnt_tasks - 1 ? tid + 1 : 0;
    while (1) {
        assert(nxt_tid != tid);
        if (tasks[nxt_tid]
            && tasks[nxt_tid]->status == TASK_RUNABLE) {
            break;
        }
        ++nxt_tid;
        if (nxt_tid == cnt_tasks) nxt_tid = 0;
    }
    kmt->spin_unlock(&lock_tasks_list);
    return nxt_tid;
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = kmt_create,
    .teardown = kmt_teardown,
    .spin_init = kmt_spin_init,
    .spin_lock = kmt_spin_lock,
    .spin_unlock = kmt_spin_unlock,
    .sem_init = kmt_sem_init,
    .sem_wait = kmt_sem_wait,
    .sem_signal = kmt_sem_signal,
};

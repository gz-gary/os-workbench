#include <common.h>
#include <os.h>

int cnt_tasks;
task_t *tasks[TASKS_LIMIT];
task_t *current[CPUS_LIMIT];
spinlock_t lock_tasks_list;

struct cpu_info_t cpu_info[CPUS_LIMIT];

static Context *kmt_context_save(Event ev, Context *context) {
    int c = cpu_current();
    current[c]->context = *context;
    current[c]->status = TASK_RUNABLE;
    // putch('0');
    return NULL;
}

static int kmt_get_next_task(int tid) {
    kmt->spin_lock(&lock_tasks_list);
    int nxt_tid = tid < cnt_tasks - 1 ? tid + 1 : 0;
    while (1) {
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

static Context *kmt_schedule(Event ev, Context *context) {
    // putch('1');
    int c = cpu_current();
    int next_tid = kmt_get_next_task(current[c]->tid);
    // printf("\nswitch %d to %d\n", current[c]->tid, next_tid);
    current[c] = tasks[next_tid];
    current[c]->status = TASK_RUNNING;
    return &(current[c]->context);
}

static void kmt_init() {
    os->on_irq(INT32_MIN, EVENT_NULL, kmt_context_save);
    os->on_irq(INT32_MAX, EVENT_NULL, kmt_schedule);

    kmt->spin_init(&lock_tasks_list, "tasks list");
    for (int i = 0; i < CPUS_LIMIT; ++i) {
        cpu_info[i] = (struct cpu_info_t) {
            .noff = 0,
            .intena = true
        };
        current[i] = NULL;
    }
    cnt_tasks = 0;
    for (int i = 0; i < TASKS_LIMIT; ++i) {
        tasks[i] = NULL;
    }
}

static int kmt_create(task_t *task, const char *name, void (*entry)(void *arg), void *arg) {
    task->status = TASK_RUNABLE;
    task->name = name;
    task->entry = entry;

    kmt->spin_lock(&lock_tasks_list);

    task->context = *kcontext(
        (Area){ .start = task->kstack, .end = task + 1 }, entry, arg
    );
    int first_unused_tid = -1;
    for (int i = 0; i < cnt_tasks; ++i) if (tasks[i] == NULL) {
        first_unused_tid = i;
        break;
    }
    if (first_unused_tid == -1) {
        first_unused_tid = cnt_tasks++;
    }
    printf("%d %d %p\n", cnt_tasks, first_unused_tid, task);
    tasks[first_unused_tid] = task;
    task->tid = first_unused_tid;

    kmt->spin_unlock(&lock_tasks_list);
    return 0;
}

static void kmt_teardown(task_t *task) {
    kmt->spin_lock(&lock_tasks_list);

    tasks[task->tid] = NULL;
    if (task->tid == cnt_tasks - 1) --cnt_tasks;
    pmm->free(task);

    kmt->spin_unlock(&lock_tasks_list);
}

MODULE_DEF(kmt) = {
    .init = kmt_init,
    .create = kmt_create,
    .teardown = kmt_teardown,
    .spin_init = spinlock_init,
    .spin_lock = spinlock_lock,
    .spin_unlock = spinlock_unlock,
    .sem_init = semaphore_init,
    .sem_wait = semaphore_wait,
    .sem_signal = semaphore_signal,
};

#include <common.h>
#include <spinlock.h>

#ifndef TEST

#define NR_CPUS 8
#define BUF 512
#define TOTAL_ALLOC 120

typedef struct workload_t workload_t;
typedef struct workload_queue_t workload_queue_t;

struct workload_t {
    enum {
        WORK_ALLOC,
        WORK_FREE
    } type;
    union {
        void *ptr;
        size_t size;
    };
};

struct workload_queue_t {
    spinlock_t lock;
    int head, tail, size;
    workload_t queue[BUF];
};

workload_queue_t consumer_queue[NR_CPUS];

void queue_init(workload_queue_t *q) {
    spinlock_init(&q->lock);
    q->head = 0;
    q->tail = 0;
    q->size = 0;
}

void queue_push(workload_queue_t *q, workload_t workload) {
    q->queue[q->tail] = workload;
    q->tail = (q->tail + 1) % BUF;
    ++q->size;
}

workload_t queue_pop(workload_queue_t *q) {
    workload_t ret = q->queue[q->head];
    q->head = (q->head + 1) % BUF;
    --q->size;
    return ret;
}

static void producer() {
    size_t size;
    size_t sum_size = 0;
    for (int _ = 0; _ < TOTAL_ALLOC; ++_) {
        float rnd = (float)rand() / (float)RAND_MAX;
        if (rnd < 0.7) {
            rnd = (float)rand() / (float)RAND_MAX;
            if (rnd < 0.9) {
                size = rand() % 128 + 1;
            } else {
                size = rand() % (4096 - 129) + 129;
            }
        } else if (rnd < 0.98) {
            size = (rand() % 16 + 1) * 4096;
        } else {
            size = (rand() % 4 + 1) * 1024 * 1024;
        }
        int cpuid = rand() % cpu_count();
        workload_t workload = (workload_t) {
            .type = WORK_ALLOC,
            .size = size
        };
        //printf("cpu %d to allocate %d bytes\n", cpuid, size);
        sum_size += size;
        spinlock_lock(&consumer_queue[cpuid].lock);
        queue_push(&consumer_queue[cpuid], workload);
        spinlock_unlock(&consumer_queue[cpuid].lock);
    }
    //printf("sum_size: %d\n", sum_size);
}

#endif 

spinlock_t stdout_log;

static void os_init() {
    pmm->init();
    #ifndef TEST
    for (int i = 0; i < cpu_count(); ++i) {
        queue_init(&consumer_queue[i]);
    }
    producer();
    spinlock_init(&stdout_log);
    #endif
}

static void os_run() {
    /*for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }*/
    #ifndef TEST
    int cpuid = cpu_current();
    workload_t workload;
    int work_to_do = 0;
    while (1) {
        work_to_do = 0;
        spinlock_lock(&consumer_queue[cpuid].lock);
        if (consumer_queue[cpuid].size > 0) {
            workload = queue_pop(&consumer_queue[cpuid]);
            work_to_do = 1;
        }
        spinlock_unlock(&consumer_queue[cpuid].lock);
        if (work_to_do) {
            if (workload.type == WORK_ALLOC) {
                void *ptr = pmm->alloc(workload.size);
                assert(ptr);
                assert((((uintptr_t)ptr) & (power_bound(workload.size) - 1)) == 0);
                /*spinlock_lock(&stdout_log);
                printf("kalloc %p %p\n", ptr, ptr + workload.size);
                spinlock_unlock(&stdout_log);*/
                int another_cpuid = rand() % cpu_count();
                workload = (workload_t) {
                    .type = WORK_FREE,
                    .ptr = ptr
                };
                spinlock_lock(&consumer_queue[another_cpuid].lock);
                queue_push(&consumer_queue[another_cpuid], workload);
                spinlock_unlock(&consumer_queue[another_cpuid].lock);
            } else {
                pmm->free(workload.ptr);
                /*spinlock_lock(&stdout_log);
                printf("kfree %p\n", workload.ptr);
                spinlock_unlock(&stdout_log);*/
            }
        }
    }
    #endif
    while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};

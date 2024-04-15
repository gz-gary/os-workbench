#include <common.h>
#include <spinlock.h>

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
        printf("cpu %d to allocate %d bytes\n", cpu_current(), size);
        sum_size += size;
        spinlock_lock(&consumer_queue[cpuid].lock);
        queue_push(&consumer_queue[cpuid], workload);
        spinlock_unlock(&consumer_queue[cpuid].lock);
    }
    printf("sum_size: %ld\n", sum_size);
}

static void os_init() {
    pmm->init();
    for (int i = 0; i < cpu_count(); ++i) {
        queue_init(&consumer_queue[i]);
    }
    producer();
}

static void os_run() {
    /*for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }*/
    while (1) ;
}

MODULE_DEF(os) = {
    .init = os_init,
    .run  = os_run,
};

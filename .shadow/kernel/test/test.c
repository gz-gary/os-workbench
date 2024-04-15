#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <thread.h>
#include <kernel.h>
#include <spinlock.h>
#include <debug-macros.h>

#define NR_CPUS 4
#define BUF 512
#define TOTAL_ALLOC 80

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

/* ---------------- fake klib and am ---------------- */

int cpu_current() {
    pthread_t pid = pthread_self();
    for (int i = 0; i < n_; ++i) {
        if (threads_[i].thread == pid) {
            return threads_[i].id;
        }
    }
}

inline int cpu_count() {
    return NR_CPUS;
}

inline void putch(char ch) {
    putchar(ch);
}

inline int atomic_xchg(volatile int *addr, int newval) {
    int result;
    asm volatile ("lock xchg %0, %1":
        "+m"(*addr), "=a"(result) : "1"(newval) : "memory");
    return result;
}

/* --------------------------------------------- */

static inline size_t power_bound(size_t x) {
    size_t ret = 1;
    while (ret < x) ret <<= 1;
    return ret;
}

static void entry(int id) {
    while (n_ < NR_CPUS); //wait until all threads were created

    size_t block_size[8];
    void *ptr[8];
    //size_t max_size = 64 * 1024; //1 B to 64 KiB per request
    size_t min_size = 128;

    for (int i = 0; i < LENGTH(block_size); ++i) {
        //if (rand() & 1) {
        //block_size[i] = rand() % max_size + 1;
        //} else {
        block_size[i] = rand() % min_size + 1;
        //}
        ptr[i] = pmm->alloc(block_size[i]);

        // check if we get an available addr
        assert(ptr[i]);
        // check if we get a legal addr
        assert(((uintptr_t)ptr[i] & (power_bound(block_size[i]) - 1)) == 0);
    }

    unsigned char my_identifier = id;
    for (int i = 0; i < LENGTH(block_size); ++i) {
        // brush my range with my id
        memset(ptr[i], my_identifier, block_size[i]);
    }

    // check if anyone invades my range for 10 times
    for (int k = 0; k < 10; ++k)
        for (int i = 0; i < LENGTH(block_size); ++i) {
            for (size_t offset = 0; offset < block_size[i]; ++offset) {
                unsigned char byte_here = *(unsigned char *)(ptr[i] + offset);
                assert(byte_here == id);
            }
        }

    for (int i = 0; i < LENGTH(block_size); ++i) pmm->free(ptr[i]);
}

struct lock_counter {
    int cnt;
    spinlock_t lock;
} total_free;

struct lock_log {
    FILE *fp;
    spinlock_t lock;
} log_file;

size_t sum_size;

static void workload_producer() {
    size_t size;
    int _ = 0;
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
        int cpuid = rand() % NR_CPUS;
        workload_t workload = (workload_t) {
            .type = WORK_ALLOC,
            .size = size
        };
        sum_size += size;
        spinlock_lock(&consumer_queue[cpuid].lock);
        queue_push(&consumer_queue[cpuid], workload);
        spinlock_unlock(&consumer_queue[cpuid].lock);
    }
    printf("sum_size: %ld\n", sum_size);
}

static void workload_consumer(int id) {
    while (n_ < NR_CPUS); //wait until all threads were created
    int cpuid = id - 1;
    workload_t workload;
    int work_to_do;
    while (1) {
        spinlock_lock(&total_free.lock);
        if (total_free.cnt == TOTAL_ALLOC) {
            spinlock_lock(&log_file.lock);
            if (log_file.fp) {
                fclose(log_file.fp);
                log_file.fp = NULL;
            }
            spinlock_unlock(&log_file.lock);
            spinlock_unlock(&total_free.lock);
            break;
        }
        spinlock_unlock(&total_free.lock);
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
                spinlock_lock(&log_file.lock);
                fprintf(log_file.fp, "kalloc %p %p\n", ptr, ptr + workload.size);
                spinlock_unlock(&log_file.lock);
                int another_cpuid = rand() % NR_CPUS;
                workload = (workload_t) {
                    .type = WORK_FREE,
                    .ptr = ptr
                };
                spinlock_lock(&consumer_queue[another_cpuid].lock);
                queue_push(&consumer_queue[another_cpuid], workload);
                spinlock_unlock(&consumer_queue[another_cpuid].lock);
            } else {
                pmm->free(workload.ptr);
                spinlock_lock(&log_file.lock);
                fprintf(log_file.fp, "kfree %p\n", workload.ptr);
                spinlock_unlock(&log_file.lock);
                spinlock_lock(&total_free.lock);
                ++total_free.cnt;
                spinlock_unlock(&total_free.lock);
            }
        }
    }
}

static void alloc_test() {
    for (int i = 0; i < NR_CPUS; ++i) {
        queue_init(&consumer_queue[i]);
    }
    spinlock_init(&total_free.lock);
    log_file.fp = fopen("test/log.txt", "w");
    spinlock_init(&log_file.lock);
    total_free.cnt = 0;
    for (int i = 0; i < NR_CPUS; ++i) {
        create(workload_consumer);
    }
    create(workload_producer);
}

static void simple_test() {
    n_ = 1;
    entry(1);
}

int main() {
    srand(time(0));
    pmm->init();
    alloc_test();
    // simple_test();
}
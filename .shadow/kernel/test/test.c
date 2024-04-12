#include <thread.h>
#include <kernel.h>

#define NR_CPUS 4

struct heap_t heap;

int cpu_current() {
    pthread_t pid = pthread_self();
    for (int i = 0; i < n_; ++i) {
        if (threads_[i].thread == pid) {
            return threads_[i].id;
        }
    }
}

int cpu_count() {
    return n_;
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

static void entry(int id) {
    while (n_ < NR_CPUS); //wait until all threads were created
    pmm->alloc(128);
}

int main() {
    pmm->init();
    for (int i = 0; i < 4; ++i) {
        create(entry);
    }
}
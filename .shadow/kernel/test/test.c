#include <thread.h>
#include <kernel.h>

#define NR_CPUS 4

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
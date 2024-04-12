#include <stdio.h>
#include <stdint.h>

struct heap_t {
    void *start, *end;
};
extern struct heap_t heap;

extern void putch(char ch);

extern int cpu_current();
extern int cpu_count();

extern int atomic_xchg(volatile int *addr, int newval);
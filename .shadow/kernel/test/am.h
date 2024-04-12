#include <stdio.h>
#include <stdint.h>

struct heap_t {
    uintptr_t start, end;
};

extern inline void putch(char ch);

extern int cpu_current();
extern int cpu_count();

extern inline int atomic_xchg(volatile int *addr, int newval);

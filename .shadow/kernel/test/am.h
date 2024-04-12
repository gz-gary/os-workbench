#pragma once
#include <stdio.h>

inline void putch(char ch) {
    putchar(ch);
}

extern int cpu_current();
extern int cpu_count();

inline int atomic_xchg(volatile int *addr, int newval) {
    int result;
    asm volatile ("lock xchg %0, %1":
        "+m"(*addr), "=a"(result) : "1"(newval) : "memory");
    return result;
}

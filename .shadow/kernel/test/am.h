#pragma once
#include <stdio.h>
#include <stdint.h>

extern void putch(char ch);

extern int cpu_current();
extern int cpu_count();

extern int atomic_xchg(volatile int *addr, int newval);
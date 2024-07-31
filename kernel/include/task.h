#ifndef TASK_H
#define TASK_H

#include <am.h>
#include <klib.h>
#include <klib-macros.h>

#define TASKS_LIMIT 128
#define KSTACK_LIMIT 8 * 1024 // 8 KiB

struct task {
    const char     *name;
    void           (*entry)(void*);
    Context        context;
    int            tid;
    enum {
        TASK_RUNABLE,
        TASK_RUNNING,
        TASK_BLOCKED,
    }              status;
    uint8_t        kstack[KSTACK_LIMIT];
};

#endif

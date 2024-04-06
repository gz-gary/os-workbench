#include "co.h"
#include <stdlib.h>
#include <setjmp.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <time.h>
#define STACK_SIZE (128 * 1024) // - 64 KiB per coroutine

enum co_status {
    CO_NEW = 1,     // - created but not awake
    CO_RUNNING,     // - currently running
    CO_WAITING,     // - waiting to continue
    CO_DEAD,        // - jobs finished
};

struct co {
    char *name;
    void (*func)(void *);
    void *arg;

    enum co_status status;
    struct co *    who_wake_me_up;
    jmp_buf        context;
    #if __x86_64__
    __attribute__((aligned(16)))       // x86_64 stack pointer alignment
    #endif
    uint8_t        stack[STACK_SIZE];
};

#define NEW(co)       (((co)->status) == CO_NEW)
#define RUNNING(co)   (((co)->status) == CO_RUNNING)
#define WAITING(co)   (((co)->status) == CO_WAITING)
#define DEAD(co)      (((co)->status) == CO_DEAD)

typedef struct list_node {
    struct list_node   *next, *prev;
    struct co          *item;
} list_node;

static list_node *ls_front, *ls_back;

static void insert_front(struct co *item) {
    list_node *new_node = malloc(sizeof(list_node));
    *new_node = (list_node) {
        .item = item,
        .next = ls_front,
        .prev = NULL,
    };
    if (ls_front) ls_front->prev = new_node;
    ls_front = new_node;
}
#define for_in_list(iter) for (list_node *iter = ls_front; iter; iter = (iter)->next)

static void remove_item(const struct co *item) {
    for_in_list(ln) if (ln->item == item) {
        if (ln == ls_front) ls_front = ls_front->next;
        else ln->prev->next = ln->next;
        free(ln);
        break;
    }
}

static struct co *current;

static struct co *co_available() {
    // - randomly choose a coroutine of CO_NEW or CO_WAITING
    int cnt = 0;
    for_in_list(ln) if (WAITING(ln->item) || NEW(ln->item)) ++cnt;
    assert(cnt > 0);
    int r = rand() % cnt;
    struct co *ret = NULL;
    for_in_list(ln) if (WAITING(ln->item) || NEW(ln->item)) {
        if (r == 0) { ret = ln->item; break; } 
        else --r;
    }
    return ret;
}

__attribute__((constructor))
static void co_init() {
    srand(time(0));
    // - create a coroutine representing for main control-flow
    current = malloc(sizeof(struct co));
    *current = (struct co) {
        .name = "gz's light-weighted libco",     // - give it an arbitrary name
        .func = NULL,
        .arg = NULL,
        .status = CO_RUNNING,
    };
    insert_front(current);
}

__attribute__((always_inline))
static inline void stack_switch_call(void *sp, void *entry, uintptr_t arg) {
    asm volatile (
#if __x86_64__
        // - Stack structure :
        // sp-0x8   (nothing)     high      - to avoid strange seg-fault of alignment problems
        // sp-0x10  (caller rsp)    |       - pushed by first instr. movq
        // |---------------|        |       - rsp points to here before call, set by instr. leaq
        // sp-0x18  (rip of call)   |       - pushed by instr. call
        // sp-0x20  (caller rbp)   low      - pushed by callee
        // - reg. %rdi is used to store the single argument
        " \
        movq %%rsp, -0x10(%0); \
        leaq -0x10(%0), %%rsp; \
        movq %2, %%rdi; \
        call *%1 \
        " : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
#else
        // - Stack structure :
        // sp-0x4   (caller esp)  high      - pushed by first instr. movl
        // sp-0x8   (arg)           |       - pushed by second instr. movl
        // |---------------|        |       - esp points to here before call, set by instr. leal
        // sp-0xc   (eip of call)   |       - pushed by instr. call
        // sp-0x10  (caller ebp)   low      - pushed by callee
        " \
        movl %%esp, -0x4(%0); \
        leal -0x8(%0), %%esp; \
        movl %2, -0x8(%0); \
        call *%1 \
        " : : "b"((uintptr_t)sp), "d"(entry), "a"(arg) : "memory"
#endif
    );
}

__attribute__((always_inline))
static inline void stack_restore(void *sp) {
    asm volatile (
#if __x86_64__
        "movq -0x10(%0), %%rsp"
        : : "b"((uintptr_t)sp) : "memory"
#else
        "movl -0x4(%0), %%esp"
        : : "b"((uintptr_t)sp) : "memory"
#endif
    );
}

struct co *co_start(const char *name, void (*func)(void *), void *arg) {
    struct co *new_co = malloc(sizeof(struct co));
    *new_co = (struct co) {
        .name = malloc(strlen(name) + 1),          // +1 for the last '\0'
        .func = func,
        .arg = arg,
        .status = CO_NEW,
    };
    strcpy(new_co->name, name);
    insert_front(new_co);
    return new_co;
}

void co_wait(struct co *co) {
    // - keep running until co is dead
    while (!DEAD(co)) {
        co_yield();
    }
    // - recycle its resources (with assurement that co is only waited once)
    remove_item(co);
    free(co->name);
    free(co);
}

void co_yield() {
    int val = setjmp(current->context);
    if (val == 0) {
        // - Control flow gets here when someone called co_yield()
        // - All we need to do are:

        // - 1. let current coroutine wait
        assert(RUNNING(current));
        current->status = CO_WAITING;
        // - 2. randomly choose a new coroutine to go on
        struct co* waker = current;
        current = co_available();

        assert(current);
        // - 3. wake it up or continue
        if (NEW(current)) {
            ((volatile struct co *)current)->who_wake_me_up = waker;
            // wake it up and decide which to run after its death
            ((volatile struct co *)current)->status = CO_RUNNING;
            void *ptr = (current->stack + STACK_SIZE);
            stack_switch_call(current->stack + STACK_SIZE, current->func, (uintptr_t)current->arg);
            if (!DEAD(current->who_wake_me_up)) {
                stack_restore(ptr);
            }
            // - we can't write the code below because of strange behavior of gcc
            // stack_restore(current->stack + STACK_SIZE);

            // - when a coroutine died, it returns to co_yield() of the one who wakes it up
            ((volatile struct co *)current)->status = CO_DEAD;
            // - after its death, we choose another waiting coroutine to continue
            for_in_list(ln) if (WAITING(ln->item)) {
                current = ln->item;
                ((volatile struct co *)current)->status = CO_RUNNING;
                // - switch to its context and run it (go back its co_yield())
                longjmp(current->context, 0);
            }
        } else if (WAITING(current)) {
            ((volatile struct co *)current)->status = CO_RUNNING;
            // - switch to its context and run it (go back its co_yield())
            longjmp(current->context, 0);
        }
    } else {
        // - current coroutine was chosen to continue
        assert(current->status == CO_RUNNING);
        // - do nothing and return
        return;
    }
}
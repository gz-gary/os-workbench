#include <common.h>
#include <os.h>

typedef struct handler_alt_t {
    int seq;
    int event;
    handler_t handler;
} handler_alt_t;

handler_alt_t handlers[HANDLERS_LIMIT];
int cnt_handlers;
extern task_t *current[CPUS_LIMIT];

static void os_sort_handlers() {
    for (int i = 0; i < cnt_handlers; ++i) {
        for (int j = 0; j < cnt_handlers - 1; ++j) {
            if (handlers[j].seq > handlers[j + 1].seq) {
                handler_alt_t tmp = handlers[j];
                handlers[j] = handlers[j + 1];
                handlers[j + 1] = tmp;
            }
        }
    }
}

static void os_init_handlers() {
    cnt_handlers = 0;
    for (int i = 0; i < HANDLERS_LIMIT; ++i) {
        handlers[i] = (handler_alt_t) {
            .handler = NULL, .event = EVENT_NULL, .seq = INT32_MAX
        };
    }
}

sem_t empty, fill;
#define P kmt->sem_wait
#define V kmt->sem_signal

static inline task_t *task_alloc() {
    return pmm->alloc(sizeof(task_t));
}

void T_produce(void *arg) { while (1) { P(&empty); putch('('); V(&fill); } }
void T_consume(void *arg) { while (1) { P(&fill); putch(')'); V(&empty); } }

static void run_test1() {
    int N = 5;
    int NPROD = 1;
    int NCONS = 1;
    kmt->sem_init(&empty, "empty", N);
    kmt->sem_init(&fill, "fill", 0);
    for (int i = 0; i < NPROD; ++i) {
        kmt->create(task_alloc(), "producer", T_produce, NULL);
    }
    for (int i = 0; i < NCONS; ++i) {
        kmt->create(task_alloc(), "consumer", T_consume, NULL);
    }
}

static void os_init() {
    os_init_handlers();

    pmm->init();
    kmt->init();
    for (int i = 0; i < CPUS_LIMIT; ++i) {
        task_t *t = task_alloc();
        kmt->create(t, "idle", NULL, NULL);
        t->status = TASK_RUNNING;
        current[i] = t;
    }

    run_test1();

}

static void os_run() {
    for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }
    iset(true);
    while (1) ;
}

static Context* os_trap(Event ev, Context *context) {
    Context *new_context = NULL;
    for (int i = 0; i < cnt_handlers; ++i) {
        if (handlers[i].event == EVENT_NULL
            || handlers[i].event == ev.event) {
            Context *c = handlers[i].handler(ev, context);
            panic_on(c && new_context, "Multiple context returned");
            if (c) new_context = c;
        }
    }
    panic_on(!new_context, "No context retunred");
    assert(!memcmp(new_context, context, sizeof(Context)));
    // assert(*new_context == *context);
    return new_context;
    // return context;
}

static void os_on_irq(int seq, int event, handler_t handler) {
    handlers[cnt_handlers] = (handler_alt_t) {
        .seq = seq, .event = event, .handler = handler
    };
    ++cnt_handlers;
    os_sort_handlers();
}

MODULE_DEF(os) = {
    .init    = os_init,
    .run     = os_run,
    .trap    = os_trap,
    .on_irq  = os_on_irq,
};

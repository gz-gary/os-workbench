#include <common.h>
#include <os.h>
#include <devices.h>

typedef struct handler_alt_t {
    int seq;
    int event;
    handler_t handler;
} handler_alt_t;

handler_alt_t handlers[HANDLERS_LIMIT];
int cnt_handlers;
extern task_t *current[CPUS_LIMIT];
spinlock_t lock_trap;

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

static inline task_t *task_alloc() {
    task_t *t = pmm->alloc(sizeof(task_t));
    t->status = TASK_RUNABLE;
    return t;
}

#ifdef LOCAL_TEST

sem_t empty, fill;
#define P kmt->sem_wait
#define V kmt->sem_signal

void T_produce(void *arg) {
    while (1) {
        putch('C');
        // yield();
        // P(&empty); /*putch('(');*/ V(&fill);
    }
}
void T_consume(void *arg) {
    while (1) {
        // yield();
        // P(&fill); /*putch(')');*/ V(&empty);
    }
}

static void run_test1() {
    int N = 5;
    int NPROD = 1;
    // int NCONS = 1;
    kmt->sem_init(&empty, "empty", N);
    kmt->sem_init(&fill, "fill", 0);
    for (int i = 0; i < NPROD; ++i) {
        kmt->create(task_alloc(), "producer", T_produce, NULL);
    }
    /*for (int i = 0; i < NCONS; ++i) {
        kmt->create(task_alloc(), "consumer", T_consume, NULL);
    }*/
}

static void tty_reader(void *arg) {
    device_t *tty = dev->lookup(arg);
    char cmd[128], resp[128], ps[16];
    snprintf(ps, 16, "(%s) $ ", arg);
    while (1) {
        tty->ops->write(tty, 0, ps, strlen(ps));
        int nread = tty->ops->read(tty, 0, cmd, sizeof(cmd) - 1);
        cmd[nread] = '\0';
        sprintf(resp, "tty reader task: got %d character(s).\n", strlen(cmd));
        tty->ops->write(tty, 0, resp, strlen(resp));
    }
}

static void run_test2() {
    return;
    dev->init();
    kmt->create(task_alloc(), "tty_reader", tty_reader, "tty1");
    kmt->create(task_alloc(), "tty_reader", tty_reader, "tty2");
}

#endif

static void os_run() {
#ifdef LOCAL_TEST
    /*for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }*/
#endif
    // while (1) { yield(); }
    while (1) { putch('A' + cpu_current()); }
}

static void os_init() {
    os_init_handlers();

    pmm->init();
    kmt->init();
    kmt->spin_init(&lock_trap, "trap lock");
#ifdef LOCAL_TEST
    printf("CPU count = %d\n", cpu_count());
#endif
    for (int i = 0; i < cpu_count(); ++i) {
        task_t *t = task_alloc();
        kmt->create(t, "idle", os_run, NULL);
        t->status = TASK_RUNNING;
        current[i] = t;
    }

#ifdef LOCAL_TEST
    run_test1();
    run_test2();
#endif

}

static Context* os_trap(Event ev, Context *context) {
    kmt->spin_lock(&lock_trap);

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

    kmt->spin_unlock(&lock_trap);

    return new_context;
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

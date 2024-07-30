#include <common.h>
#include <os.h>

typedef struct handler_alt_t {
    int seq;
    int event;
    handler_t handler;
} handler_alt_t;

handler_alt_t handlers[HANDLERS_LIMIT];
int cnt_handlers;

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

static void os_init() {
    os_init_handlers();

    pmm->init();
    kmt->init();

}

static void os_run() {
    for (const char *s = "Hello World from CPU #*\n"; *s; s++) {
        putch(*s == '*' ? '0' + cpu_current() : *s);
    }
    iset(true);
    while (1) ;
}

static Context* os_trap(Event ev, Context *context) {
    /*
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
    return new_context;*/
    assert(ev.event == EVENT_IRQ_TIMER);
    putch('i');
    return context;
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

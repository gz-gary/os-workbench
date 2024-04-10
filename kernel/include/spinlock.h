
typedef struct spinlock_t {
    int flag;
    int owner;
} spinlock_t;

void spinlock_init(spinlock_t *spinlock);

void spinlock_lock(spinlock_t *spinlock);

void spinlock_unlock(spinlock_t *spinlock);
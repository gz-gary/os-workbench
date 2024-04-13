#include <stdint.h>
#include <sys/types.h>

inline size_t power_bound(size_t x) {
    size_t ret = 1;
    while (ret < x) ret <<= 1;
    return ret;
}

inline void *align_to_bound(void *ptr, size_t bound) {
    void *next_available = (void *)(
        (((uintptr_t)ptr - 1) & (~(bound - 1)))
        + bound);
    return next_available;
}

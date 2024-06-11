#define ASSERT_EQUAL(a, b) assert((a) == (b))
#define ASSERT_LEQ(a, b) assert((a) <= (b))
#define ASSERT_GEQ(a, b) assert((a) >= (b))
#define ECHO_VAR(var, type) printf(#var " = " #type "\n", var)
#define ECHO_ARR(arr, index, type) printf(#arr "[%d] = " #type "\n", index, arr[index])
#define LOG_RANGE(size, ptr) printf("cpu[%d] wants %ld bytes\nallocator gives out [%p, %p)\n\n", cpu_current(), size, ptr, ptr + size);
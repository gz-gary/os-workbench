
#include <sys/types.h>
typedef struct ls_node ls_node;

struct ls_node {
    size_t len;
    ls_node *next;
};
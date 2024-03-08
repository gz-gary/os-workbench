#include <stdlib.h>

typedef struct ListNode ListNode;
typedef struct Process Process;

struct ListNode {
    ListNode *prev;
    Process *item;
};

ListNode * insert_item(ListNode *list_tail, Process *new_item) {
    ListNode *new_node = (ListNode*)malloc(sizeof(ListNode));
    new_node->item = new_item;
    new_node->prev = list_tail;
    return new_node;
}

struct Process {
    pid_t pid;
    pid_t ppid;

    char name[128];

    // Process *parent;
    ListNode *son_list_tail;
};

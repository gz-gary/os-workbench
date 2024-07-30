#include <os.h>
#include <queue.h>

void queue_init(queue_t *queue) {
    queue->size = 0;
    queue->head = NULL;
    queue->tail = NULL;
}

void queue_push(queue_t *queue, int value) {
    queue_node_t *new_node = pmm->alloc(sizeof(queue_node_t));
    new_node->value = value;
    new_node->next = NULL;

    if (queue->head == NULL) {
        queue->head = new_node;
        queue->tail = new_node;
    } else {
        queue->tail->next = new_node;
    }
    ++queue->size;
}

int queue_pop(queue_t *queue) {
    queue_node_t *head = queue->head;
    queue->head = head->next;
    --queue->size;
    int ret = head->value;
    pmm->free(head);
    return ret;
}

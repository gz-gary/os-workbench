#ifndef QUEUE_H
#define QUEUE_H

typedef struct queue queue_t;
typedef struct queue_node queue_node_t;

struct queue_node {
    int value;
    struct queue_node *next;
};

struct queue {
    int size;
    struct queue_node *head, *tail;
};

void queue_init(queue_t *queue);
void queue_push(queue_t *queue, int value);
int queue_pop(queue_t *queue);

#endif

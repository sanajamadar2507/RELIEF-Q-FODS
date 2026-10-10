#ifndef QUEUE_H
#define QUEUE_H

#include "reliefq_types.h"

typedef struct QNode {
    ReliefRequest data;
    struct QNode* next;
} QNode;

typedef struct {
    QNode* front;
    QNode* rear;
    int    count;
} Queue;

void           q_init(Queue* q);
int            q_enqueue(Queue* q, const ReliefRequest* req);
int            q_dequeue(Queue* q, ReliefRequest* out_req);
ReliefRequest* q_peek(const Queue* q);
int            q_is_empty(const Queue* q);
int            q_size(const Queue* q);
void           q_clear(Queue* q);
int            q_to_array(const Queue* q, ReliefRequest* dest, int max_dest);

#endif /* QUEUE_H */

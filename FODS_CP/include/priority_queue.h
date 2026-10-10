#ifndef PRIORITY_QUEUE_H
#define PRIORITY_QUEUE_H

#include "reliefq_types.h"
#include "queue.h"

typedef struct {
    Queue levels[4]; /* 0: Critical, 1: High, 2: Medium, 3: Low */
} PriorityQueue;

void           pq_init(PriorityQueue* pq);
int            pq_insert(PriorityQueue* pq, const ReliefRequest* req);
int            pq_dequeue(PriorityQueue* pq, ReliefRequest* out_req);
ReliefRequest* pq_peek(const PriorityQueue* pq);
int            pq_is_empty(const PriorityQueue* pq);
int            pq_size(const PriorityQueue* pq);
void           pq_clear(PriorityQueue* pq);

#endif /* PRIORITY_QUEUE_H */

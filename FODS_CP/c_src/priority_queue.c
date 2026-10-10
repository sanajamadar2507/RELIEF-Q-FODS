#include "priority_queue.h"

void pq_init(PriorityQueue* pq) {
    if (!pq) return;
    for (int i = 0; i < 4; i++) {
        q_init(&pq->levels[i]);
    }
}

int pq_insert(PriorityQueue* pq, const ReliefRequest* req) {
    if (!pq || !req) return 0;
    int idx = (int)req->priority;
    if (idx < 0 || idx > 3) {
        idx = 3; /* default to Low if invalid */
    }
    return q_enqueue(&pq->levels[idx], req);
}

int pq_dequeue(PriorityQueue* pq, ReliefRequest* out_req) {
    if (!pq) return 0;
    for (int i = 0; i < 4; i++) {
        if (!q_is_empty(&pq->levels[i])) {
            return q_dequeue(&pq->levels[i], out_req);
        }
    }
    return 0;
}

ReliefRequest* pq_peek(const PriorityQueue* pq) {
    if (!pq) return 0;
    for (int i = 0; i < 4; i++) {
        if (!q_is_empty(&pq->levels[i])) {
            return q_peek(&pq->levels[i]);
        }
    }
    return 0;
}

int pq_is_empty(const PriorityQueue* pq) {
    if (!pq) return 1;
    for (int i = 0; i < 4; i++) {
        if (!q_is_empty(&pq->levels[i])) {
            return 0;
        }
    }
    return 1;
}

int pq_size(const PriorityQueue* pq) {
    if (!pq) return 0;
    int total = 0;
    for (int i = 0; i < 4; i++) {
        total += q_size(&pq->levels[i]);
    }
    return total;
}

void pq_clear(PriorityQueue* pq) {
    if (!pq) return;
    for (int i = 0; i < 4; i++) {
        q_clear(&pq->levels[i]);
    }
}

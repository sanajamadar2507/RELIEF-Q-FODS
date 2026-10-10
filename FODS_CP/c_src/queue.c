#include "queue.h"
#include "memory_pool.h"

void q_init(Queue* q) {
    if (!q) return;
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

int q_enqueue(Queue* q, const ReliefRequest* req) {
    if (!q || !req) return 0;

    QNode* node = (QNode*)rq_malloc(sizeof(QNode));
    if (!node) return 0;

    node->data = *req;
    node->next = 0;

    if (q->rear == 0) {
        q->front = q->rear = node;
    } else {
        q->rear->next = node;
        q->rear = node;
    }
    q->count++;
    return 1;
}

int q_dequeue(Queue* q, ReliefRequest* out_req) {
    if (!q || q->front == 0) return 0;

    QNode* node = q->front;
    if (out_req) {
        *out_req = node->data;
    }

    q->front = q->front->next;
    if (q->front == 0) {
        q->rear = 0;
    }
    q->count--;
    rq_free(node);
    return 1;
}

ReliefRequest* q_peek(const Queue* q) {
    if (!q || q->front == 0) return 0;
    return &q->front->data;
}

int q_is_empty(const Queue* q) {
    return (!q || q->count == 0);
}

int q_size(const Queue* q) {
    return q ? q->count : 0;
}

void q_clear(Queue* q) {
    if (!q) return;
    QNode* curr = q->front;
    while (curr != 0) {
        QNode* next = curr->next;
        rq_free(curr);
        curr = next;
    }
    q->front = 0;
    q->rear = 0;
    q->count = 0;
}

int q_to_array(const Queue* q, ReliefRequest* dest, int max_dest) {
    if (!q || !dest || max_dest <= 0) return 0;
    int count = 0;
    QNode* curr = q->front;
    while (curr != 0 && count < max_dest) {
        dest[count++] = curr->data;
        curr = curr->next;
    }
    return count;
}

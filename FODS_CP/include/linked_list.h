#ifndef LINKED_LIST_H
#define LINKED_LIST_H

#include "reliefq_types.h"

typedef struct LLNode {
    ReliefRequest data;
    struct LLNode* next;
} LLNode;

typedef struct {
    LLNode* head;
    int size;
} LinkedList;

void           ll_init(LinkedList* list);
int            ll_insert(LinkedList* list, const ReliefRequest* req);
ReliefRequest* ll_search(LinkedList* list, const char* requestID);
int            ll_delete(LinkedList* list, const char* requestID);
int            ll_size(const LinkedList* list);
void           ll_clear(LinkedList* list);
int            ll_to_array(const LinkedList* list, ReliefRequest* dest, int max_dest);

#endif /* LINKED_LIST_H */

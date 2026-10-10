#include "linked_list.h"
#include "string_utils.h"
#include "memory_pool.h"

void ll_init(LinkedList* list) {
    if (!list) return;
    list->head = 0;
    list->size = 0;
}

int ll_insert(LinkedList* list, const ReliefRequest* req) {
    if (!list || !req) return 0;

    LLNode* node = (LLNode*)rq_malloc(sizeof(LLNode));
    if (!node) return 0;

    node->data = *req;
    node->next = 0;

    if (!list->head) {
        list->head = node;
    } else {
        LLNode* temp = list->head;
        while (temp->next != 0) {
            temp = temp->next;
        }
        temp->next = node;
    }
    list->size++;
    return 1;
}

ReliefRequest* ll_search(LinkedList* list, const char* requestID) {
    if (!list || !requestID) return 0;
    LLNode* temp = list->head;
    while (temp != 0) {
        if (rq_strcasecmp(temp->data.requestID, requestID) == 0) {
            return &temp->data;
        }
        temp = temp->next;
    }
    return 0;
}

int ll_delete(LinkedList* list, const char* requestID) {
    if (!list || !list->head || !requestID) return 0;

    if (rq_strcasecmp(list->head->data.requestID, requestID) == 0) {
        LLNode* to_free = list->head;
        list->head = list->head->next;
        rq_free(to_free);
        list->size--;
        return 1;
    }

    LLNode* prev = list->head;
    LLNode* temp = list->head->next;
    while (temp != 0) {
        if (rq_strcasecmp(temp->data.requestID, requestID) == 0) {
            prev->next = temp->next;
            rq_free(temp);
            list->size--;
            return 1;
        }
        prev = temp;
        temp = temp->next;
    }
    return 0;
}

int ll_size(const LinkedList* list) {
    return list ? list->size : 0;
}

void ll_clear(LinkedList* list) {
    if (!list) return;
    LLNode* curr = list->head;
    while (curr != 0) {
        LLNode* next = curr->next;
        rq_free(curr);
        curr = next;
    }
    list->head = 0;
    list->size = 0;
}

int ll_to_array(const LinkedList* list, ReliefRequest* dest, int max_dest) {
    if (!list || !dest || max_dest <= 0) return 0;
    int count = 0;
    LLNode* curr = list->head;
    while (curr != 0 && count < max_dest) {
        dest[count++] = curr->data;
        curr = curr->next;
    }
    return count;
}

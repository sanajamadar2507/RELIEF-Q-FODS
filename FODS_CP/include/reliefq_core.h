#ifndef RELIEFQ_CORE_H
#define RELIEFQ_CORE_H

#include "reliefq_types.h"
#include "resource_array.h"
#include "linked_list.h"
#include "queue.h"
#include "priority_queue.h"
#include "stack.h"

typedef struct {
    ResourceArray    resourceArray;
    Queue            waitingQueue;
    PriorityQueue    priorityQueue;
    LinkedList       completedList;
    Stack            historyStack;
    ReliefRequest    allRequests[MAX_REQUESTS];
    int              allRequestsCount;
    ReliefRequest    waitingForResources[MAX_REQUESTS];
    int              waitingForResourcesCount;
    EventLog         eventLog[MAX_LOG_EVENTS];
    int              eventLogCount;
    int              allocCounter;
    int              arrivalCounter;
    AllocationResult lastAllocationResult;
} ReliefQSystem;

/* Global or instance operations */
void           rq_init_system(ReliefQSystem* sys);
void           rq_load_sample_data(ReliefQSystem* sys);
int            rq_submit_request(ReliefQSystem* sys, const ReliefRequest* req, int silent);
int            rq_classify_next(ReliefQSystem* sys, ReliefRequest* out_req, int silent);
int            rq_classify_all(ReliefQSystem* sys, int silent);
int            rq_allocate_request(ReliefQSystem* sys, ReliefRequest* req, AllocationResult* out_res);
int            rq_allocate_next(ReliefQSystem* sys, AllocationResult* out_res);
int            rq_retry_allocation_by_id(ReliefQSystem* sys, const char* requestID, AllocationResult* out_res);
int            rq_undo_last(ReliefQSystem* sys, AllocationRecord* out_rec);
int            rq_add_resource(ReliefQSystem* sys, const char* name, int quantity);
void           rq_log_event(ReliefQSystem* sys, const char* text);
ReliefRequest* rq_find_request(ReliefQSystem* sys, const char* requestID);

#endif /* RELIEFQ_CORE_H */

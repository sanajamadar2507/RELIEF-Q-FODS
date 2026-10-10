#include "reliefq_core.h"
#include "string_utils.h"
#include "memory_pool.h"

void rq_log_event(ReliefQSystem* sys, const char* text) {
    if (!sys || !text) return;

    /* Maintain up to MAX_LOG_EVENTS in ring / shift */
    if (sys->eventLogCount >= MAX_LOG_EVENTS) {
        for (int i = 0; i < MAX_LOG_EVENTS - 1; i++) {
            sys->eventLog[i] = sys->eventLog[i + 1];
        }
        sys->eventLogCount = MAX_LOG_EVENTS - 1;
    }

    int idx = sys->eventLogCount++;
    sys->eventLog[idx].time[0] = '\0'; /* formatted by client or default */
    rq_strncpy(sys->eventLog[idx].text, text, sizeof(sys->eventLog[idx].text));
}

void rq_init_system(ReliefQSystem* sys) {
    if (!sys) return;

    rq_mem_init();
    res_init(&sys->resourceArray);
    q_init(&sys->waitingQueue);
    pq_init(&sys->priorityQueue);
    ll_init(&sys->completedList);
    stack_init(&sys->historyStack);

    sys->allRequestsCount = 0;
    sys->waitingForResourcesCount = 0;
    sys->eventLogCount = 0;
    sys->allocCounter = 100;
    sys->arrivalCounter = 0;
    sys->lastAllocationResult.type = ALLOC_EMPTY;
}

ReliefRequest* rq_find_request(ReliefQSystem* sys, const char* requestID) {
    if (!sys || !requestID) return 0;
    for (int i = 0; i < sys->allRequestsCount; i++) {
        if (rq_strcasecmp(sys->allRequests[i].requestID, requestID) == 0) {
            return &sys->allRequests[i];
        }
    }
    return 0;
}

static void remove_from_waiting_for_resources(ReliefQSystem* sys, const char* requestID) {
    if (!sys || !requestID) return;
    int found_idx = -1;
    for (int i = 0; i < sys->waitingForResourcesCount; i++) {
        if (rq_strcasecmp(sys->waitingForResources[i].requestID, requestID) == 0) {
            found_idx = i;
            break;
        }
    }
    if (found_idx != -1) {
        for (int i = found_idx; i < sys->waitingForResourcesCount - 1; i++) {
            sys->waitingForResources[i] = sys->waitingForResources[i + 1];
        }
        sys->waitingForResourcesCount--;
    }
}

static void add_to_waiting_for_resources(ReliefQSystem* sys, const ReliefRequest* req) {
    if (!sys || !req) return;
    for (int i = 0; i < sys->waitingForResourcesCount; i++) {
        if (rq_strcasecmp(sys->waitingForResources[i].requestID, req->requestID) == 0) {
            sys->waitingForResources[i] = *req;
            return;
        }
    }
    if (sys->waitingForResourcesCount < MAX_REQUESTS) {
        sys->waitingForResources[sys->waitingForResourcesCount++] = *req;
    }
}

int rq_submit_request(ReliefQSystem* sys, const ReliefRequest* req, int silent) {
    if (!sys || !req) return 0;
    if (sys->allRequestsCount >= MAX_REQUESTS) return 0;

    ReliefRequest r = *req;
    r.allocated = 0;
    r.status = STATUS_PENDING;
    r.arrivalOrder = sys->arrivalCounter++;

    sys->allRequests[sys->allRequestsCount++] = r;
    q_enqueue(&sys->waitingQueue, &r);

    if (!silent) {
        char msg[128];
        rq_strcpy(msg, r.requestID);
        rq_strcat(msg, " submitted and added to Queue");
        rq_log_event(sys, msg);
    }
    return 1;
}

int rq_classify_next(ReliefQSystem* sys, ReliefRequest* out_req, int silent) {
    if (!sys) return 0;

    ReliefRequest req;
    if (!q_dequeue(&sys->waitingQueue, &req)) {
        return 0;
    }

    /* Update status in master list */
    ReliefRequest* master = rq_find_request(sys, req.requestID);
    if (master) {
        req = *master;
    }

    pq_insert(&sys->priorityQueue, &req);

    if (out_req) {
        *out_req = req;
    }

    if (!silent) {
        char msg[128];
        rq_strcpy(msg, req.requestID);
        rq_strcat(msg, " classified as ");
        rq_strcat(msg, rq_priority_to_string(req.priority));
        rq_strcat(msg, " and moved to Priority Queue");
        rq_log_event(sys, msg);
    }
    return 1;
}

int rq_classify_all(ReliefQSystem* sys, int silent) {
    if (!sys) return 0;
    int count = 0;
    while (!q_is_empty(&sys->waitingQueue)) {
        if (rq_classify_next(sys, 0, silent)) {
            count++;
        }
    }
    return count;
}

int rq_allocate_request(ReliefQSystem* sys, ReliefRequest* req, AllocationResult* out_res) {
    if (!sys || !req) return 0;

    /* Look up latest record in allRequests */
    ReliefRequest* master = rq_find_request(sys, req->requestID);
    if (master) {
        req = master;
    }

    int available = res_get_available(&sys->resourceArray, req->resource);
    int still_needed = req->quantity - req->allocated;

    if (available <= 0) {
        req->status = STATUS_WAITING_FOR_RESOURCES;
        add_to_waiting_for_resources(sys, req);

        char msg[160];
        rq_strcpy(msg, req->requestID);
        rq_strcat(msg, " could not be allocated — 0 units of ");
        rq_strcat(msg, req->resource);
        rq_strcat(msg, " available. Marked \"Waiting for Resources\".");
        rq_log_event(sys, msg);

        if (out_res) {
            out_res->type = ALLOC_WAIT;
            out_res->req = *req;
            out_res->given = 0;
        }
        sys->lastAllocationResult.type = ALLOC_WAIT;
        sys->lastAllocationResult.req = *req;
        sys->lastAllocationResult.given = 0;
        return 1;
    }

    int given = res_consume(&sys->resourceArray, req->resource, still_needed);
    req->allocated += given;

    if (req->allocated >= req->quantity) {
        req->status = STATUS_COMPLETED;
    } else {
        req->status = STATUS_PARTIALLY_ALLOCATED;
    }

    remove_from_waiting_for_resources(sys, req->requestID);

    /* Store completed/distributed request in Linked List */
    if (!ll_search(&sys->completedList, req->requestID)) {
        ll_insert(&sys->completedList, req);
    }

    /* Push allocation operation onto Stack */
    sys->allocCounter++;
    AllocationRecord record;
    char num_buf[16];
    rq_strcpy(record.allocID, "A");
    rq_itoa(sys->allocCounter, num_buf);
    rq_strcat(record.allocID, num_buf);

    rq_strcpy(record.requestID, req->requestID);
    rq_strcpy(record.resource, req->resource);
    record.quantity = given;
    rq_strcpy(record.location, req->location);
    rq_strcpy(record.status, rq_status_to_string(req->status));
    record.time[0] = '\0';

    stack_push(&sys->historyStack, &record);

    char log_buf[256];
    rq_itoa(given, num_buf);
    rq_strcpy(log_buf, num_buf);
    rq_strcat(log_buf, " unit(s) of ");
    rq_strcat(log_buf, req->resource);
    rq_strcat(log_buf, " allocated to ");
    rq_strcat(log_buf, req->requestID);
    rq_strcat(log_buf, " (");
    rq_strcat(log_buf, req->location);
    rq_strcat(log_buf, ") — ");
    rq_strcat(log_buf, rq_status_to_string(req->status));
    rq_log_event(sys, log_buf);

    char log_stack[128];
    rq_strcpy(log_stack, "Allocation ");
    rq_strcat(log_stack, record.allocID);
    rq_strcat(log_stack, " pushed onto Stack");
    rq_log_event(sys, log_stack);

    if (req->status == STATUS_COMPLETED) {
        char log_ll[128];
        rq_strcpy(log_ll, req->requestID);
        rq_strcat(log_ll, " added to Linked List (Completed)");
        rq_log_event(sys, log_ll);
    }

    AllocationType atype = (req->status == STATUS_COMPLETED) ? ALLOC_OK : ALLOC_PARTIAL;
    if (out_res) {
        out_res->type = atype;
        out_res->req = *req;
        out_res->given = given;
        out_res->record = record;
    }
    sys->lastAllocationResult.type = atype;
    sys->lastAllocationResult.req = *req;
    sys->lastAllocationResult.given = given;
    sys->lastAllocationResult.record = record;

    return 1;
}

int rq_allocate_next(ReliefQSystem* sys, AllocationResult* out_res) {
    if (!sys) return 0;
    ReliefRequest req;
    if (!pq_dequeue(&sys->priorityQueue, &req)) {
        if (out_res) {
            out_res->type = ALLOC_EMPTY;
        }
        sys->lastAllocationResult.type = ALLOC_EMPTY;
        return 0;
    }
    return rq_allocate_request(sys, &req, out_res);
}

int rq_retry_allocation_by_id(ReliefQSystem* sys, const char* requestID, AllocationResult* out_res) {
    if (!sys || !requestID) return 0;
    ReliefRequest* req = rq_find_request(sys, requestID);
    if (!req) return 0;
    return rq_allocate_request(sys, req, out_res);
}

int rq_undo_last(ReliefQSystem* sys, AllocationRecord* out_rec) {
    if (!sys) return 0;

    AllocationRecord record;
    if (!stack_pop(&sys->historyStack, &record)) {
        return 0;
    }

    ReliefRequest* req = rq_find_request(sys, record.requestID);
    if (req) {
        res_restore(&sys->resourceArray, record.resource, record.quantity);
        req->allocated -= record.quantity;

        if (req->allocated <= 0) {
            req->allocated = 0;
            req->status = STATUS_PENDING;
            ll_delete(&sys->completedList, req->requestID);
            pq_insert(&sys->priorityQueue, req);
        } else {
            req->status = STATUS_PARTIALLY_ALLOCATED;
        }

        char msg[256];
        char num_buf[16];
        rq_strcpy(msg, "Undo: allocation ");
        rq_strcat(msg, record.allocID);
        rq_strcat(msg, " reversed — ");
        rq_itoa(record.quantity, num_buf);
        rq_strcat(msg, num_buf);
        rq_strcat(msg, " unit(s) of ");
        rq_strcat(msg, record.resource);
        rq_strcat(msg, " returned to inventory, ");
        rq_strcat(msg, req->requestID);
        rq_strcat(msg, " is now \"");
        rq_strcat(msg, rq_status_to_string(req->status));
        rq_strcat(msg, "\"");
        rq_log_event(sys, msg);
    }

    if (out_rec) {
        *out_rec = record;
    }
    return 1;
}

int rq_add_resource(ReliefQSystem* sys, const char* name, int quantity) {
    if (!sys || !name || quantity <= 0) return 0;
    int res = res_add(&sys->resourceArray, name, quantity);
    if (res >= 0) {
        char msg[128];
        char num_buf[16];
        rq_itoa(quantity, num_buf);
        rq_strcpy(msg, num_buf);
        rq_strcat(msg, " unit(s) of ");
        rq_strcat(msg, name);
        rq_strcat(msg, " added to inventory");
        rq_log_event(sys, msg);
        return 1;
    }
    return 0;
}

void rq_load_sample_data(ReliefQSystem* sys) {
    if (!sys) return;

    /* 1. Sample Resources */
    rq_add_resource(sys, "Food", 1000);
    rq_add_resource(sys, "Water", 1200);
    rq_add_resource(sys, "Medicines", 500);
    rq_add_resource(sys, "Blankets", 800);
    rq_add_resource(sys, "Tents", 300);
    rq_add_resource(sys, "Clothes", 600);
    rq_add_resource(sys, "Emergency Kits", 250);

    /* 2. Sample Requests */
    ReliefRequest samples[] = {
        {"R101", "Village A", "Pune",     "Flood",      500, "Water",          300, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 0},
        {"R102", "Village B", "Kolhapur", "Fire",       120, "Medicines",      80,  0, PRIORITY_HIGH,     STATUS_PENDING, "", 0},
        {"R103", "Village C", "Satara",   "Landslide",  200, "Tents",          150, 0, PRIORITY_LOW,      STATUS_PENDING, "", 0},
        {"R104", "Village D", "Sangli",   "Flood",      750, "Food",           400, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 0},
        {"R105", "Village E", "Nashik",   "Earthquake", 300, "Medicines",      600, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 0},
        {"R106", "Village F", "Pune",     "Cyclone",    180, "Blankets",       200, 0, PRIORITY_MEDIUM,   STATUS_PENDING, "", 0},
        {"R107", "Village G", "Satara",   "Drought",    400, "Water",          500, 0, PRIORITY_MEDIUM,   STATUS_PENDING, "", 0},
        {"R108", "Village H", "Sangli",   "Fire",       90,  "Emergency Kits", 60,  0, PRIORITY_HIGH,     STATUS_PENDING, "", 0},
        {"R109", "Village I", "Kolhapur", "Flood",      620, "Water",          350, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 0},
        {"R110", "Village J", "Nashik",   "Landslide",  150, "Clothes",        120, 0, PRIORITY_LOW,      STATUS_PENDING, "", 0}
    };

    int n = sizeof(samples) / sizeof(samples[0]);
    for (int i = 0; i < n; i++) {
        rq_submit_request(sys, &samples[i], 1);
    }

    /* 3. Classify all into Priority Queue */
    rq_classify_all(sys, 1);

    /* 4. Pre-run two allocations matching original bootstrap */
    rq_allocate_next(sys, 0);
    rq_allocate_next(sys, 0);

    rq_log_event(sys, "RELIEF-Q initialized with sample data");
}

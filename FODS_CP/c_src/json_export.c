#include "json_export.h"
#include "string_utils.h"

typedef struct {
    char* buf;
    int   max_len;
    int   pos;
} JsonWriter;

static void jw_init(JsonWriter* w, char* buf, int max_len) {
    w->buf = buf;
    w->max_len = max_len;
    w->pos = 0;
    if (buf && max_len > 0) {
        buf[0] = '\0';
    }
}

static void jw_raw(JsonWriter* w, const char* s) {
    if (!w->buf || !s) return;
    while (*s && w->pos < w->max_len - 1) {
        w->buf[w->pos++] = *s++;
    }
    w->buf[w->pos] = '\0';
}

static void jw_escaped_str(JsonWriter* w, const char* s) {
    if (!w->buf) return;
    jw_raw(w, "\"");
    if (s) {
        while (*s && w->pos < w->max_len - 2) {
            char c = *s++;
            if (c == '"' || c == '\\') {
                if (w->pos < w->max_len - 3) {
                    w->buf[w->pos++] = '\\';
                    w->buf[w->pos++] = c;
                }
            } else if (c == '\n') {
                if (w->pos < w->max_len - 3) {
                    w->buf[w->pos++] = '\\';
                    w->buf[w->pos++] = 'n';
                }
            } else {
                w->buf[w->pos++] = c;
            }
        }
    }
    jw_raw(w, "\"");
}

static void jw_int(JsonWriter* w, int val) {
    char temp[32];
    rq_itoa(val, temp);
    jw_raw(w, temp);
}

static void jw_key(JsonWriter* w, const char* key) {
    jw_escaped_str(w, key);
    jw_raw(w, ":");
}

static void jw_request(JsonWriter* w, const ReliefRequest* r) {
    jw_raw(w, "{");
    jw_key(w, "requestID");    jw_escaped_str(w, r->requestID);   jw_raw(w, ",");
    jw_key(w, "area");         jw_escaped_str(w, r->area);        jw_raw(w, ",");
    jw_key(w, "location");     jw_escaped_str(w, r->location);    jw_raw(w, ",");
    jw_key(w, "disasterType"); jw_escaped_str(w, r->disasterType);jw_raw(w, ",");
    jw_key(w, "population");   jw_int(w, r->population);          jw_raw(w, ",");
    jw_key(w, "resource");     jw_escaped_str(w, r->resource);    jw_raw(w, ",");
    jw_key(w, "quantity");     jw_int(w, r->quantity);            jw_raw(w, ",");
    jw_key(w, "allocated");    jw_int(w, r->allocated);           jw_raw(w, ",");
    jw_key(w, "priority");     jw_escaped_str(w, rq_priority_to_string(r->priority)); jw_raw(w, ",");
    jw_key(w, "status");       jw_escaped_str(w, rq_status_to_string(r->status));   jw_raw(w, ",");
    jw_key(w, "arrivalOrder"); jw_int(w, r->arrivalOrder);
    jw_raw(w, "}");
}

static void jw_record(JsonWriter* w, const AllocationRecord* r) {
    jw_raw(w, "{");
    jw_key(w, "allocID");   jw_escaped_str(w, r->allocID);   jw_raw(w, ",");
    jw_key(w, "requestID"); jw_escaped_str(w, r->requestID); jw_raw(w, ",");
    jw_key(w, "resource");  jw_escaped_str(w, r->resource);  jw_raw(w, ",");
    jw_key(w, "quantity");  jw_int(w, r->quantity);          jw_raw(w, ",");
    jw_key(w, "location");  jw_escaped_str(w, r->location);  jw_raw(w, ",");
    jw_key(w, "status");    jw_escaped_str(w, r->status);    jw_raw(w, ",");
    jw_key(w, "time");      jw_escaped_str(w, r->time);
    jw_raw(w, "}");
}

int json_export_system_state(const ReliefQSystem* sys, char* out_buf, int max_len) {
    if (!sys || !out_buf || max_len <= 0) return 0;

    JsonWriter w;
    jw_init(&w, out_buf, max_len);

    jw_raw(&w, "{");

    /* 1. Stats */
    int total = sys->allRequestsCount;
    int pending = 0, critical = 0, completed = 0;
    for (int i = 0; i < total; i++) {
        if (sys->allRequests[i].status == STATUS_PENDING) pending++;
        if (sys->allRequests[i].priority == PRIORITY_CRITICAL) critical++;
        if (sys->allRequests[i].status == STATUS_COMPLETED) completed++;
    }

    jw_key(&w, "stats");
    jw_raw(&w, "{");
    jw_key(&w, "total");           jw_int(&w, total);                                   jw_raw(&w, ",");
    jw_key(&w, "pending");         jw_int(&w, pending);                                 jw_raw(&w, ",");
    jw_key(&w, "critical");        jw_int(&w, critical);                                jw_raw(&w, ",");
    jw_key(&w, "completed");       jw_int(&w, completed);                               jw_raw(&w, ",");
    jw_key(&w, "resAvailable");    jw_int(&w, res_total_available(&sys->resourceArray));jw_raw(&w, ",");
    jw_key(&w, "resDistributed");  jw_int(&w, res_total_distributed(&sys->resourceArray));
    jw_raw(&w, "},");

    /* 2. Priority counts */
    int count_crit = 0, count_high = 0, count_med = 0, count_low = 0;
    for (int i = 0; i < total; i++) {
        if (sys->allRequests[i].priority == PRIORITY_CRITICAL) count_crit++;
        else if (sys->allRequests[i].priority == PRIORITY_HIGH) count_high++;
        else if (sys->allRequests[i].priority == PRIORITY_MEDIUM) count_med++;
        else if (sys->allRequests[i].priority == PRIORITY_LOW) count_low++;
    }
    jw_key(&w, "priorityCounts");
    jw_raw(&w, "{");
    jw_key(&w, "Critical"); jw_int(&w, count_crit); jw_raw(&w, ",");
    jw_key(&w, "High");     jw_int(&w, count_high); jw_raw(&w, ",");
    jw_key(&w, "Medium");   jw_int(&w, count_med);  jw_raw(&w, ",");
    jw_key(&w, "Low");      jw_int(&w, count_low);
    jw_raw(&w, "},");

    /* 3. Inventory */
    jw_key(&w, "inventory");
    jw_raw(&w, "[");
    for (int i = 0; i < sys->resourceArray.count; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_raw(&w, "{");
        jw_key(&w, "name");        jw_escaped_str(&w, sys->resourceArray.items[i].name); jw_raw(&w, ",");
        jw_key(&w, "total");       jw_int(&w, sys->resourceArray.items[i].total);       jw_raw(&w, ",");
        jw_key(&w, "available");   jw_int(&w, sys->resourceArray.items[i].available);   jw_raw(&w, ",");
        jw_key(&w, "distributed"); jw_int(&w, sys->resourceArray.items[i].distributed);
        jw_raw(&w, "}");
    }
    jw_raw(&w, "],");

    /* 4. Waiting Queue */
    jw_key(&w, "waitingQueueCount"); jw_int(&w, q_size(&sys->waitingQueue)); jw_raw(&w, ",");
    jw_key(&w, "waitingQueue");
    jw_raw(&w, "[");
    ReliefRequest temp_q[MAX_REQUESTS];
    int q_len = q_to_array(&sys->waitingQueue, temp_q, MAX_REQUESTS);
    for (int i = 0; i < q_len; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_request(&w, &temp_q[i]);
    }
    jw_raw(&w, "],");

    /* 5. Priority Queue Grouped */
    jw_key(&w, "priorityQueue");
    jw_raw(&w, "{");
    const char* pnames[4] = {"Critical", "High", "Medium", "Low"};
    for (int l = 0; l < 4; l++) {
        if (l > 0) jw_raw(&w, ",");
        jw_key(&w, pnames[l]);
        jw_raw(&w, "[");
        int lane_len = q_to_array(&sys->priorityQueue.levels[l], temp_q, MAX_REQUESTS);
        for (int i = 0; i < lane_len; i++) {
            if (i > 0) jw_raw(&w, ",");
            jw_request(&w, &temp_q[i]);
        }
        jw_raw(&w, "]");
    }
    jw_raw(&w, "},");

    /* 6. All requests */
    jw_key(&w, "allRequests");
    jw_raw(&w, "[");
    for (int i = 0; i < sys->allRequestsCount; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_request(&w, &sys->allRequests[i]);
    }
    jw_raw(&w, "],");

    /* 7. Waiting for resources */
    jw_key(&w, "waitingForResources");
    jw_raw(&w, "[");
    for (int i = 0; i < sys->waitingForResourcesCount; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_request(&w, &sys->waitingForResources[i]);
    }
    jw_raw(&w, "],");

    /* 8. History Stack */
    jw_key(&w, "historyStack");
    jw_raw(&w, "[");
    AllocationRecord temp_stack[MAX_HISTORY];
    int st_len = stack_to_array(&sys->historyStack, temp_stack, MAX_HISTORY);
    for (int i = 0; i < st_len; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_record(&w, &temp_stack[i]);
    }
    jw_raw(&w, "],");

    /* 9. Completed List (Linked List) */
    jw_key(&w, "completedList");
    jw_raw(&w, "[");
    int ll_len = ll_to_array(&sys->completedList, temp_q, MAX_REQUESTS);
    for (int i = 0; i < ll_len; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_request(&w, &temp_q[i]);
    }
    jw_raw(&w, "],");

    /* 10. Live Feed / Event Log */
    jw_key(&w, "liveFeed");
    jw_raw(&w, "[");
    for (int i = 0; i < sys->eventLogCount; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_raw(&w, "{");
        jw_key(&w, "time"); jw_escaped_str(&w, sys->eventLog[i].time); jw_raw(&w, ",");
        jw_key(&w, "text"); jw_escaped_str(&w, sys->eventLog[i].text);
        jw_raw(&w, "}");
    }
    jw_raw(&w, "],");

    /* 11. Last Allocation Result */
    jw_key(&w, "lastAllocResult");
    jw_raw(&w, "{");
    const char* type_str = "empty";
    if (sys->lastAllocationResult.type == ALLOC_WAIT) type_str = "wait";
    else if (sys->lastAllocationResult.type == ALLOC_PARTIAL) type_str = "partial";
    else if (sys->lastAllocationResult.type == ALLOC_OK) type_str = "ok";

    jw_key(&w, "type");  jw_escaped_str(&w, type_str); jw_raw(&w, ",");
    jw_key(&w, "given"); jw_int(&w, sys->lastAllocationResult.given); jw_raw(&w, ",");
    jw_key(&w, "req");   jw_request(&w, &sys->lastAllocationResult.req); jw_raw(&w, ",");
    jw_key(&w, "record");jw_record(&w, &sys->lastAllocationResult.record);
    jw_raw(&w, "},");

    /* 12. Reports aggregation */
    /* Most requested resource */
    const char* top_resource = "—";
    int top_resource_count = 0;
    for (int i = 0; i < sys->resourceArray.count; i++) {
        int cnt = 0;
        for (int j = 0; j < sys->allRequestsCount; j++) {
            if (rq_strcasecmp(sys->allRequests[j].resource, sys->resourceArray.items[i].name) == 0) {
                cnt++;
            }
        }
        if (cnt > top_resource_count) {
            top_resource_count = cnt;
            top_resource = sys->resourceArray.items[i].name;
        }
    }

    /* Disaster counts */
    const char* dtypes[6] = {"Flood", "Earthquake", "Cyclone", "Landslide", "Drought", "Fire"};
    jw_key(&w, "reports");
    jw_raw(&w, "{");
    jw_key(&w, "mostRequestedResource"); jw_escaped_str(&w, top_resource); jw_raw(&w, ",");
    jw_key(&w, "disasterCounts");
    jw_raw(&w, "{");
    for (int d = 0; d < 6; d++) {
        if (d > 0) jw_raw(&w, ",");
        int dcnt = 0;
        for (int j = 0; j < sys->allRequestsCount; j++) {
            if (rq_strcasecmp(sys->allRequests[j].disasterType, dtypes[d]) == 0) {
                dcnt++;
            }
        }
        jw_key(&w, dtypes[d]);
        jw_int(&w, dcnt);
    }
    jw_raw(&w, "}");
    jw_raw(&w, "}");

    jw_raw(&w, "}");
    return w.pos;
}

int json_export_search_results(
    const ReliefRequest* results,
    int count,
    const char* meta,
    char* out_buf,
    int max_len
) {
    if (!out_buf || max_len <= 0) return 0;
    JsonWriter w;
    jw_init(&w, out_buf, max_len);

    jw_raw(&w, "{");
    jw_key(&w, "count"); jw_int(&w, count); jw_raw(&w, ",");
    jw_key(&w, "meta");  jw_escaped_str(&w, meta ? meta : ""); jw_raw(&w, ",");
    jw_key(&w, "results");
    jw_raw(&w, "[");
    for (int i = 0; i < count; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_request(&w, &results[i]);
    }
    jw_raw(&w, "]");
    jw_raw(&w, "}");
    return w.pos;
}

int json_export_sort_results(
    const ReliefRequest* before,
    const ReliefRequest* after,
    int count,
    char* out_buf,
    int max_len
) {
    if (!out_buf || max_len <= 0) return 0;
    JsonWriter w;
    jw_init(&w, out_buf, max_len);

    jw_raw(&w, "{");
    jw_key(&w, "count"); jw_int(&w, count); jw_raw(&w, ",");
    jw_key(&w, "before");
    jw_raw(&w, "[");
    for (int i = 0; i < count; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_request(&w, &before[i]);
    }
    jw_raw(&w, "],");
    jw_key(&w, "after");
    jw_raw(&w, "[");
    for (int i = 0; i < count; i++) {
        if (i > 0) jw_raw(&w, ",");
        jw_request(&w, &after[i]);
    }
    jw_raw(&w, "]");
    jw_raw(&w, "}");
    return w.pos;
}

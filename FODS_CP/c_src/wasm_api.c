#include "reliefq_core.h"
#include "search_sort.h"
#include "json_export.h"
#include "string_utils.h"
#include "memory_pool.h"

#if defined(__wasm__) || defined(__wasm32__)
#define WASM_EXPORT __attribute__((visibility("default")))
#else
#define WASM_EXPORT
#endif

static ReliefQSystem g_system;
static char g_json_buffer[JSON_BUFFER_SIZE];

WASM_EXPORT void* rq_wasm_alloc(int size) {
    return rq_malloc((rq_size_t)size);
}

WASM_EXPORT void rq_wasm_free(void* ptr) {
    rq_free(ptr);
}

WASM_EXPORT char* rq_wasm_get_buffer(void) {
    return g_json_buffer;
}

WASM_EXPORT void rq_wasm_init(void) {
    rq_init_system(&g_system);
    rq_load_sample_data(&g_system);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
}

WASM_EXPORT const char* rq_wasm_get_state(void) {
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return g_json_buffer;
}

WASM_EXPORT int rq_wasm_submit_request(
    const char* id,
    const char* area,
    const char* loc,
    const char* disaster,
    int pop,
    const char* res,
    int qty,
    const char* priority
) {
    ReliefRequest req;
    rq_strncpy(req.requestID, id, sizeof(req.requestID));
    rq_strncpy(req.area, area, sizeof(req.area));
    rq_strncpy(req.location, loc, sizeof(req.location));
    rq_strncpy(req.disasterType, disaster, sizeof(req.disasterType));
    req.population = pop;
    rq_strncpy(req.resource, res, sizeof(req.resource));
    req.quantity = qty;
    req.allocated = 0;
    req.priority = rq_string_to_priority(priority);
    req.status = STATUS_PENDING;
    req.requestTime[0] = '\0';
    req.arrivalOrder = 0;

    int success = rq_submit_request(&g_system, &req, 0);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return success;
}

WASM_EXPORT int rq_wasm_classify_next(void) {
    ReliefRequest req;
    int success = rq_classify_next(&g_system, &req, 0);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return success;
}

WASM_EXPORT int rq_wasm_classify_all(void) {
    int count = rq_classify_all(&g_system, 0);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return count;
}

WASM_EXPORT int rq_wasm_allocate_next(void) {
    AllocationResult res;
    int ok = rq_allocate_next(&g_system, &res);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return ok;
}

WASM_EXPORT int rq_wasm_retry_allocation(const char* requestID) {
    AllocationResult res;
    int ok = rq_retry_allocation_by_id(&g_system, requestID, &res);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return ok;
}

WASM_EXPORT int rq_wasm_undo_last(void) {
    AllocationRecord rec;
    int ok = rq_undo_last(&g_system, &rec);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return ok;
}

WASM_EXPORT int rq_wasm_add_resource(const char* name, int quantity) {
    int ok = rq_add_resource(&g_system, name, quantity);
    json_export_system_state(&g_system, g_json_buffer, JSON_BUFFER_SIZE);
    return ok;
}

WASM_EXPORT const char* rq_wasm_linear_search(const char* field, const char* keyword) {
    ReliefRequest matches[MAX_REQUESTS];
    int count = linear_search_requests(
        g_system.allRequests,
        g_system.allRequestsCount,
        field,
        keyword,
        matches,
        MAX_REQUESTS
    );

    char meta[160];
    char c_buf[16], t_buf[16];
    rq_itoa(g_system.allRequestsCount, t_buf);
    rq_itoa(count, c_buf);
    rq_strcpy(meta, "Linear Search scanned ");
    rq_strcat(meta, t_buf);
    rq_strcat(meta, " record(s) and found ");
    rq_strcat(meta, c_buf);
    rq_strcat(meta, " match(es).");

    json_export_search_results(matches, count, meta, g_json_buffer, JSON_BUFFER_SIZE);
    return g_json_buffer;
}

WASM_EXPORT const char* rq_wasm_binary_search(const char* requestID) {
    ReliefRequest sorted[MAX_REQUESTS];
    bubble_sort_requests(
        g_system.allRequests,
        g_system.allRequestsCount,
        "requestID",
        1,
        sorted
    );

    ReliefRequest found;
    int steps[32];
    int steps_count = 0;
    int ok = binary_search_by_id(
        sorted,
        g_system.allRequestsCount,
        requestID,
        &found,
        steps,
        32,
        &steps_count
    );

    char meta[160];
    char s_buf[16];
    rq_itoa(steps_count, s_buf);
    if (ok) {
        rq_strcpy(meta, "Binary Search found ");
        rq_strcat(meta, requestID);
        rq_strcat(meta, " in ");
        rq_strcat(meta, s_buf);
        rq_strcat(meta, " comparison(s) (array pre-sorted by ID).");
        json_export_search_results(&found, 1, meta, g_json_buffer, JSON_BUFFER_SIZE);
    } else {
        rq_strcpy(meta, "Binary Search checked ");
        rq_strcat(meta, s_buf);
        rq_strcat(meta, " comparison(s) — ");
        rq_strcat(meta, requestID);
        rq_strcat(meta, " was not found.");
        json_export_search_results(0, 0, meta, g_json_buffer, JSON_BUFFER_SIZE);
    }
    return g_json_buffer;
}

WASM_EXPORT const char* rq_wasm_bubble_sort(const char* field, int ascending) {
    ReliefRequest sorted[MAX_REQUESTS];
    bubble_sort_requests(
        g_system.allRequests,
        g_system.allRequestsCount,
        field,
        ascending,
        sorted
    );

    json_export_sort_results(
        g_system.allRequests,
        sorted,
        g_system.allRequestsCount,
        g_json_buffer,
        JSON_BUFFER_SIZE
    );
    return g_json_buffer;
}

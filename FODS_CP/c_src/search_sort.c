#include "search_sort.h"
#include "string_utils.h"

int linear_search_requests(
    const ReliefRequest* requests,
    int count,
    const char* field,
    const char* keyword,
    ReliefRequest* results,
    int max_results
) {
    if (!requests || count <= 0 || !field || !keyword || !results || max_results <= 0) {
        return 0;
    }

    int match_count = 0;

    for (int i = 0; i < count && match_count < max_results; i++) {
        const char* val = "";
        if (rq_strcasecmp(field, "requestID") == 0) {
            val = requests[i].requestID;
        } else if (rq_strcasecmp(field, "location") == 0) {
            val = requests[i].location;
        } else if (rq_strcasecmp(field, "disasterType") == 0) {
            val = requests[i].disasterType;
        } else if (rq_strcasecmp(field, "resource") == 0) {
            val = requests[i].resource;
        } else if (rq_strcasecmp(field, "area") == 0) {
            val = requests[i].area;
        }

        if (rq_strstr_case(val, keyword)) {
            results[match_count++] = requests[i];
        }
    }

    return match_count;
}

int binary_search_by_id(
    const ReliefRequest* sorted_requests,
    int count,
    const char* target_id,
    ReliefRequest* out_found,
    int* steps,
    int max_steps,
    int* steps_count
) {
    if (steps_count) *steps_count = 0;
    if (!sorted_requests || count <= 0 || !target_id) {
        return 0;
    }

    int low = 0;
    int high = count - 1;
    int step_idx = 0;

    while (low <= high) {
        int mid = (low + high) / 2;
        if (steps && step_idx < max_steps) {
            steps[step_idx++] = mid;
        }

        int cmp = rq_strcasecmp(sorted_requests[mid].requestID, target_id);
        if (cmp == 0) {
            if (out_found) {
                *out_found = sorted_requests[mid];
            }
            if (steps_count) *steps_count = step_idx;
            return 1;
        } else if (cmp < 0) {
            low = mid + 1;
        } else {
            high = mid - 1;
        }
    }

    if (steps_count) *steps_count = step_idx;
    return 0;
}

void bubble_sort_requests(
    const ReliefRequest* requests,
    int count,
    const char* field,
    int ascending,
    ReliefRequest* sorted_dest
) {
    if (!requests || count <= 0 || !sorted_dest) return;

    /* Copy original array to destination */
    for (int i = 0; i < count; i++) {
        sorted_dest[i] = requests[i];
    }

    for (int i = 0; i < count - 1; i++) {
        for (int j = 0; j < count - i - 1; j++) {
            int should_swap = 0;

            if (rq_strcasecmp(field, "priority") == 0) {
                int a = rq_priority_rank(sorted_dest[j].priority);
                int b = rq_priority_rank(sorted_dest[j + 1].priority);
                should_swap = ascending ? (a > b) : (a < b);
            } else if (rq_strcasecmp(field, "population") == 0) {
                int a = sorted_dest[j].population;
                int b = sorted_dest[j + 1].population;
                should_swap = ascending ? (a > b) : (a < b);
            } else if (rq_strcasecmp(field, "quantity") == 0) {
                int a = sorted_dest[j].quantity;
                int b = sorted_dest[j + 1].quantity;
                should_swap = ascending ? (a > b) : (a < b);
            } else if (rq_strcasecmp(field, "requestID") == 0) {
                int cmp = rq_strcasecmp(sorted_dest[j].requestID, sorted_dest[j + 1].requestID);
                should_swap = ascending ? (cmp > 0) : (cmp < 0);
            }

            if (should_swap) {
                ReliefRequest temp = sorted_dest[j];
                sorted_dest[j] = sorted_dest[j + 1];
                sorted_dest[j + 1] = temp;
            }
        }
    }
}

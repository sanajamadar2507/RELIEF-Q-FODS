#ifndef SEARCH_SORT_H
#define SEARCH_SORT_H

#include "reliefq_types.h"

int linear_search_requests(
    const ReliefRequest* requests,
    int count,
    const char* field,
    const char* keyword,
    ReliefRequest* results,
    int max_results
);

int binary_search_by_id(
    const ReliefRequest* sorted_requests,
    int count,
    const char* target_id,
    ReliefRequest* out_found,
    int* steps,
    int max_steps,
    int* steps_count
);

void bubble_sort_requests(
    const ReliefRequest* requests,
    int count,
    const char* field,
    int ascending,
    ReliefRequest* sorted_dest
);

#endif /* SEARCH_SORT_H */

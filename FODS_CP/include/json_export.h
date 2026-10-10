#ifndef JSON_EXPORT_H
#define JSON_EXPORT_H

#include "reliefq_core.h"
#include "search_sort.h"

int json_export_system_state(const ReliefQSystem* sys, char* out_buf, int max_len);

int json_export_search_results(
    const ReliefRequest* results,
    int count,
    const char* meta,
    char* out_buf,
    int max_len
);

int json_export_sort_results(
    const ReliefRequest* before,
    const ReliefRequest* after,
    int count,
    char* out_buf,
    int max_len
);

#endif /* JSON_EXPORT_H */

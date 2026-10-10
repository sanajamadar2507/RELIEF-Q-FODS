#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include "reliefq_types.h"
#include "memory_pool.h"

rq_size_t rq_strlen(const char* s);
int       rq_strcmp(const char* s1, const char* s2);
int       rq_strcasecmp(const char* s1, const char* s2);
char*     rq_strcpy(char* dest, const char* src);
char*     rq_strncpy(char* dest, const char* src, rq_size_t n);
char*     rq_strcat(char* dest, const char* src);
int       rq_strstr_case(const char* haystack, const char* needle);
int       rq_itoa(int val, char* buf);

const char*   rq_priority_to_string(PriorityLevel p);
PriorityLevel rq_string_to_priority(const char* str);

const char*   rq_status_to_string(RequestStatus s);
RequestStatus rq_string_to_status(const char* str);

int rq_priority_rank(PriorityLevel p);

#endif /* STRING_UTILS_H */

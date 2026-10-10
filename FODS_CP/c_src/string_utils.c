#include "string_utils.h"

static char to_lower_char(char c) {
    if (c >= 'A' && c <= 'Z') {
        return (char)(c + ('a' - 'A'));
    }
    return c;
}

rq_size_t rq_strlen(const char* s) {
    if (!s) return 0;
    rq_size_t len = 0;
    while (s[len]) len++;
    return len;
}

int rq_strcmp(const char* s1, const char* s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    while (*s1 && (*s1 == *s2)) {
        s1++;
        s2++;
    }
    return *(const unsigned char*)s1 - *(const unsigned char*)s2;
}

int rq_strcasecmp(const char* s1, const char* s2) {
    if (!s1 && !s2) return 0;
    if (!s1) return -1;
    if (!s2) return 1;
    while (*s1 && (to_lower_char(*s1) == to_lower_char(*s2))) {
        s1++;
        s2++;
    }
    return (int)to_lower_char(*s1) - (int)to_lower_char(*s2);
}

char* rq_strcpy(char* dest, const char* src) {
    if (!dest) return 0;
    if (!src) {
        dest[0] = '\0';
        return dest;
    }
    char* d = dest;
    while ((*d++ = *src++)) {}
    return dest;
}

char* rq_strncpy(char* dest, const char* src, rq_size_t n) {
    if (!dest || n == 0) return dest;
    rq_size_t i = 0;
    if (src) {
        for (; i < n - 1 && src[i]; i++) {
            dest[i] = src[i];
        }
    }
    dest[i] = '\0';
    return dest;
}

char* rq_strcat(char* dest, const char* src) {
    if (!dest || !src) return dest;
    char* d = dest;
    while (*d) d++;
    while ((*d++ = *src++)) {}
    return dest;
}

int rq_strstr_case(const char* haystack, const char* needle) {
    if (!haystack || !needle) return 0;
    if (!*needle) return 1; /* empty needle matches anything */

    rq_size_t h_len = rq_strlen(haystack);
    rq_size_t n_len = rq_strlen(needle);
    if (n_len > h_len) return 0;

    for (rq_size_t i = 0; i <= h_len - n_len; i++) {
        rq_size_t j = 0;
        while (j < n_len && to_lower_char(haystack[i + j]) == to_lower_char(needle[j])) {
            j++;
        }
        if (j == n_len) return 1;
    }
    return 0;
}

int rq_itoa(int val, char* buf) {
    if (!buf) return 0;
    if (val == 0) {
        buf[0] = '0';
        buf[1] = '\0';
        return 1;
    }

    int i = 0;
    int is_neg = 0;
    long long n = val;
    if (n < 0) {
        is_neg = 1;
        n = -n;
    }

    char temp[32];
    int t_len = 0;
    while (n > 0) {
        temp[t_len++] = (char)('0' + (n % 10));
        n /= 10;
    }

    if (is_neg) {
        buf[i++] = '-';
    }
    for (int j = t_len - 1; j >= 0; j--) {
        buf[i++] = temp[j];
    }
    buf[i] = '\0';
    return i;
}

const char* rq_priority_to_string(PriorityLevel p) {
    switch (p) {
        case PRIORITY_CRITICAL: return "Critical";
        case PRIORITY_HIGH:     return "High";
        case PRIORITY_MEDIUM:   return "Medium";
        case PRIORITY_LOW:      return "Low";
        default:                return "Low";
    }
}

PriorityLevel rq_string_to_priority(const char* str) {
    if (!str) return PRIORITY_LOW;
    if (rq_strcasecmp(str, "Critical") == 0) return PRIORITY_CRITICAL;
    if (rq_strcasecmp(str, "High") == 0)     return PRIORITY_HIGH;
    if (rq_strcasecmp(str, "Medium") == 0)   return PRIORITY_MEDIUM;
    if (rq_strcasecmp(str, "Low") == 0)      return PRIORITY_LOW;
    return PRIORITY_LOW;
}

const char* rq_status_to_string(RequestStatus s) {
    switch (s) {
        case STATUS_PENDING:               return "Pending";
        case STATUS_WAITING_FOR_RESOURCES: return "Waiting for Resources";
        case STATUS_PARTIALLY_ALLOCATED:   return "Partially Allocated";
        case STATUS_COMPLETED:             return "Completed";
        default:                           return "Pending";
    }
}

RequestStatus rq_string_to_status(const char* str) {
    if (!str) return STATUS_PENDING;
    if (rq_strcasecmp(str, "Completed") == 0)             return STATUS_COMPLETED;
    if (rq_strcasecmp(str, "Partially Allocated") == 0)   return STATUS_PARTIALLY_ALLOCATED;
    if (rq_strcasecmp(str, "Waiting for Resources") == 0) return STATUS_WAITING_FOR_RESOURCES;
    if (rq_strcasecmp(str, "Pending") == 0)               return STATUS_PENDING;
    return STATUS_PENDING;
}

int rq_priority_rank(PriorityLevel p) {
    /* Critical: 1, High: 2, Medium: 3, Low: 4 (lower number = higher priority) */
    switch (p) {
        case PRIORITY_CRITICAL: return 1;
        case PRIORITY_HIGH:     return 2;
        case PRIORITY_MEDIUM:   return 3;
        case PRIORITY_LOW:      return 4;
        default:                return 5;
    }
}

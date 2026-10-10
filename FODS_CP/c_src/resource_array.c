#include "resource_array.h"
#include "string_utils.h"

void res_init(ResourceArray* ra) {
    if (!ra) return;
    ra->count = 0;
    for (int i = 0; i < MAX_RESOURCES; i++) {
        ra->items[i].name[0] = '\0';
        ra->items[i].total = 0;
        ra->items[i].available = 0;
        ra->items[i].distributed = 0;
    }
}

int res_find_index(const ResourceArray* ra, const char* name) {
    if (!ra || !name) return -1;
    for (int i = 0; i < ra->count; i++) {
        if (rq_strcasecmp(ra->items[i].name, name) == 0) {
            return i;
        }
    }
    return -1;
}

int res_add(ResourceArray* ra, const char* name, int quantity) {
    if (!ra || !name || quantity <= 0) return -1;

    int idx = res_find_index(ra, name);
    if (idx != -1) {
        ra->items[idx].total += quantity;
        ra->items[idx].available += quantity;
        return idx;
    }

    if (ra->count >= MAX_RESOURCES) {
        return -1; /* inventory full */
    }

    int new_idx = ra->count++;
    rq_strncpy(ra->items[new_idx].name, name, sizeof(ra->items[new_idx].name));
    ra->items[new_idx].total = quantity;
    ra->items[new_idx].available = quantity;
    ra->items[new_idx].distributed = 0;
    return new_idx;
}

int res_get_available(const ResourceArray* ra, const char* name) {
    if (!ra || !name) return 0;
    int idx = res_find_index(ra, name);
    if (idx == -1) return 0;
    return ra->items[idx].available;
}

int res_consume(ResourceArray* ra, const char* name, int quantity) {
    if (!ra || !name || quantity <= 0) return 0;
    int idx = res_find_index(ra, name);
    if (idx == -1) return 0;

    int avail = ra->items[idx].available;
    int given = (avail < quantity) ? avail : quantity;
    ra->items[idx].available -= given;
    ra->items[idx].distributed += given;
    return given;
}

void res_restore(ResourceArray* ra, const char* name, int quantity) {
    if (!ra || !name || quantity <= 0) return;
    int idx = res_find_index(ra, name);
    if (idx == -1) return;

    ra->items[idx].available += quantity;
    ra->items[idx].distributed -= quantity;
    if (ra->items[idx].distributed < 0) {
        ra->items[idx].distributed = 0;
    }
}

int res_total_available(const ResourceArray* ra) {
    if (!ra) return 0;
    int sum = 0;
    for (int i = 0; i < ra->count; i++) {
        sum += ra->items[i].available;
    }
    return sum;
}

int res_total_distributed(const ResourceArray* ra) {
    if (!ra) return 0;
    int sum = 0;
    for (int i = 0; i < ra->count; i++) {
        sum += ra->items[i].distributed;
    }
    return sum;
}

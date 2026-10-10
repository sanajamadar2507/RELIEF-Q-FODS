#ifndef RESOURCE_ARRAY_H
#define RESOURCE_ARRAY_H

#include "reliefq_types.h"

typedef struct {
    ResourceItem items[MAX_RESOURCES];
    int count;
} ResourceArray;

void res_init(ResourceArray* ra);
int  res_add(ResourceArray* ra, const char* name, int quantity);
int  res_find_index(const ResourceArray* ra, const char* name);
int  res_get_available(const ResourceArray* ra, const char* name);
int  res_consume(ResourceArray* ra, const char* name, int quantity);
void res_restore(ResourceArray* ra, const char* name, int quantity);
int  res_total_available(const ResourceArray* ra);
int  res_total_distributed(const ResourceArray* ra);

#endif /* RESOURCE_ARRAY_H */

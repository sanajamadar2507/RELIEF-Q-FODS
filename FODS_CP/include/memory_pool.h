#ifndef MEMORY_POOL_H
#define MEMORY_POOL_H

#if defined(__wasm__) || defined(__wasm32__)
typedef unsigned long rq_size_t;
void* memcpy(void* dest, const void* src, rq_size_t n);
void* memmove(void* dest, const void* src, rq_size_t n);
void* memset(void* dest, int c, rq_size_t n);
#else
#include <stddef.h>
#include <string.h>
typedef size_t rq_size_t;
#endif

void  rq_mem_init(void);
void* rq_malloc(rq_size_t size);
void  rq_free(void* ptr);
void  rq_mem_reset(void);

#endif /* MEMORY_POOL_H */

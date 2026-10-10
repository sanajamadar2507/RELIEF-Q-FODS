#include "memory_pool.h"

#define STATIC_HEAP_SIZE (512 * 1024)

static char s_heap[STATIC_HEAP_SIZE];
static rq_size_t s_heap_offset = 0;

/* Simple free list for small chunks up to 128 bytes (nodes) */
typedef struct BlockHeader {
    rq_size_t size;
    struct BlockHeader* next;
} BlockHeader;

static BlockHeader* s_free_list = 0;

void rq_mem_init(void) {
    s_heap_offset = 0;
    s_free_list = 0;
}

void* rq_malloc(rq_size_t size) {
    if (size == 0) return 0;

    /* Align to 8 bytes */
    rq_size_t aligned_size = (size + 7) & ~((rq_size_t)7);
    if (aligned_size < sizeof(BlockHeader)) {
        aligned_size = sizeof(BlockHeader);
    }

    /* Check free list for a reusable block */
    BlockHeader** curr = &s_free_list;
    while (*curr) {
        if ((*curr)->size >= aligned_size) {
            BlockHeader* match = *curr;
            *curr = match->next;
            return (void*)match;
        }
        curr = &((*curr)->next);
    }

    /* Otherwise, allocate from heap */
    if (s_heap_offset + aligned_size > STATIC_HEAP_SIZE) {
        return 0; /* Out of memory */
    }

    void* ptr = (void*)&s_heap[s_heap_offset];
    s_heap_offset += aligned_size;
    return ptr;
}

void rq_free(void* ptr) {
    if (!ptr) return;
    /* Put into free list */
    BlockHeader* block = (BlockHeader*)ptr;
    block->size = sizeof(BlockHeader); /* minimum reusable size */
    block->next = s_free_list;
    s_free_list = block;
}

void rq_mem_reset(void) {
    s_heap_offset = 0;
    s_free_list = 0;
}

#if defined(__wasm__) || defined(__wasm32__)
void* memcpy(void* dest, const void* src, rq_size_t n) {
    if (!dest || !src || n == 0) return dest;
    char* d = (char*)dest;
    const char* s = (const char*)src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

void* memmove(void* dest, const void* src, rq_size_t n) {
    if (!dest || !src || n == 0) return dest;
    char* d = (char*)dest;
    const char* s = (const char*)src;
    if (d < s) {
        while (n--) {
            *d++ = *s++;
        }
    } else {
        d += n;
        s += n;
        while (n--) {
            *--d = *--s;
        }
    }
    return dest;
}

void* memset(void* dest, int c, rq_size_t n) {
    if (!dest || n == 0) return dest;
    char* d = (char*)dest;
    while (n--) {
        *d++ = (char)c;
    }
    return dest;
}
#endif



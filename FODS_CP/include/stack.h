#ifndef STACK_H
#define STACK_H

#include "reliefq_types.h"

typedef struct {
    AllocationRecord items[MAX_HISTORY];
    int top; /* index of next free slot; top == 0 means empty */
} Stack;

void              stack_init(Stack* s);
int               stack_push(Stack* s, const AllocationRecord* rec);
int               stack_pop(Stack* s, AllocationRecord* out_rec);
AllocationRecord* stack_peek(Stack* s);
int               stack_is_empty(const Stack* s);
int               stack_size(const Stack* s);
void              stack_clear(Stack* s);
int               stack_to_array(const Stack* s, AllocationRecord* dest, int max_dest);

#endif /* STACK_H */

#include "stack.h"

void stack_init(Stack* s) {
    if (!s) return;
    s->top = 0;
}

int stack_push(Stack* s, const AllocationRecord* rec) {
    if (!s || !rec) return 0;
    if (s->top >= MAX_HISTORY) return 0; /* stack full */
    s->items[s->top++] = *rec;
    return 1;
}

int stack_pop(Stack* s, AllocationRecord* out_rec) {
    if (!s || s->top <= 0) return 0;
    s->top--;
    if (out_rec) {
        *out_rec = s->items[s->top];
    }
    return 1;
}

AllocationRecord* stack_peek(Stack* s) {
    if (!s || s->top <= 0) return 0;
    return &s->items[s->top - 1];
}

int stack_is_empty(const Stack* s) {
    return (!s || s->top == 0);
}

int stack_size(const Stack* s) {
    return s ? s->top : 0;
}

void stack_clear(Stack* s) {
    if (!s) return;
    s->top = 0;
}

/* Returns history with the MOST RECENT allocation first (top of stack first) */
int stack_to_array(const Stack* s, AllocationRecord* dest, int max_dest) {
    if (!s || !dest || max_dest <= 0) return 0;
    int count = 0;
    for (int i = s->top - 1; i >= 0 && count < max_dest; i--) {
        dest[count++] = s->items[i];
    }
    return count;
}

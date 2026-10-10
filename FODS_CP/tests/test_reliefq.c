#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "reliefq_core.h"
#include "search_sort.h"
#include "string_utils.h"

static int g_tests_run = 0;
static int g_tests_passed = 0;

#define TEST_ASSERT(cond, msg) do { \
    g_tests_run++; \
    if (cond) { \
        g_tests_passed++; \
        printf("  [PASS] %s\n", msg); \
    } else { \
        printf("  [FAIL] %s (Line %d: %s)\n", msg, __LINE__, #cond); \
    } \
} while (0)

void test_resource_array(void) {
    printf("\n=== TEST SUITE 1: ResourceArray (Array) ===\n");
    ResourceArray ra;
    res_init(&ra);
    TEST_ASSERT(ra.count == 0, "Initial resource array count is 0");

    res_add(&ra, "Water", 500);
    TEST_ASSERT(ra.count == 1, "Added Water (500), count is 1");
    TEST_ASSERT(res_get_available(&ra, "Water") == 500, "Water available is 500");

    /* Add more quantity to existing resource */
    res_add(&ra, "Water", 200);
    TEST_ASSERT(ra.count == 1, "Added Water (200), count remains 1");
    TEST_ASSERT(res_get_available(&ra, "Water") == 700, "Water available is now 700");

    /* Case-insensitive lookup */
    TEST_ASSERT(res_find_index(&ra, "water") == 0, "Case-insensitive lookup finds 'water' at index 0");

    /* Consume full/partial */
    int given = res_consume(&ra, "Water", 300);
    TEST_ASSERT(given == 300, "Consumed 300 units of Water");
    TEST_ASSERT(res_get_available(&ra, "Water") == 400, "Water available is now 400");
    TEST_ASSERT(ra.items[0].distributed == 300, "Water distributed is 300");

    /* Over-consume */
    given = res_consume(&ra, "Water", 500);
    TEST_ASSERT(given == 400, "Over-consume Water returned only available 400");
    TEST_ASSERT(res_get_available(&ra, "Water") == 0, "Water available is 0");
    TEST_ASSERT(ra.items[0].distributed == 700, "Water distributed is 700");

    /* Restore */
    res_restore(&ra, "Water", 300);
    TEST_ASSERT(res_get_available(&ra, "Water") == 300, "Restored 300 Water, available is 300");
    TEST_ASSERT(ra.items[0].distributed == 400, "Distributed decreased to 400");
}

void test_linked_list(void) {
    printf("\n=== TEST SUITE 2: LinkedList (Singly Linked List) ===\n");
    LinkedList ll;
    ll_init(&ll);
    TEST_ASSERT(ll_size(&ll) == 0, "Initial LinkedList size is 0");

    ReliefRequest r1 = {"R1", "Area 1", "Pune", "Flood", 100, "Food", 50, 50, PRIORITY_HIGH, STATUS_COMPLETED, "", 0};
    ReliefRequest r2 = {"R2", "Area 2", "Nashik", "Fire", 200, "Water", 80, 80, PRIORITY_CRITICAL, STATUS_COMPLETED, "", 1};
    ReliefRequest r3 = {"R3", "Area 3", "Satara", "Landslide", 300, "Tents", 30, 30, PRIORITY_LOW, STATUS_COMPLETED, "", 2};

    ll_insert(&ll, &r1);
    ll_insert(&ll, &r2);
    ll_insert(&ll, &r3);
    TEST_ASSERT(ll_size(&ll) == 3, "Inserted 3 items into LinkedList");

    ReliefRequest* found = ll_search(&ll, "R2");
    TEST_ASSERT(found != 0 && strcmp(found->area, "Area 2") == 0, "Searched and found R2 in LinkedList");

    found = ll_search(&ll, "R99");
    TEST_ASSERT(found == 0, "Non-existent request returns NULL");

    /* Delete middle */
    int del_ok = ll_delete(&ll, "R2");
    TEST_ASSERT(del_ok == 1, "Deleted R2 (middle node)");
    TEST_ASSERT(ll_size(&ll) == 2, "Size is now 2");
    TEST_ASSERT(ll_search(&ll, "R2") == 0, "R2 no longer in list");

    /* Delete head */
    del_ok = ll_delete(&ll, "R1");
    TEST_ASSERT(del_ok == 1, "Deleted R1 (head node)");
    TEST_ASSERT(ll_size(&ll) == 1, "Size is now 1");

    ll_clear(&ll);
    TEST_ASSERT(ll_size(&ll) == 0, "Cleared list, size is 0");
}

void test_queue(void) {
    printf("\n=== TEST SUITE 3: Queue (FIFO) ===\n");
    Queue q;
    q_init(&q);
    TEST_ASSERT(q_is_empty(&q) == 1, "Initial Queue is empty");

    ReliefRequest r1 = {"Q1", "Village A", "Pune", "Flood", 100, "Food", 50, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 0};
    ReliefRequest r2 = {"Q2", "Village B", "Kolhapur", "Fire", 200, "Water", 80, 0, PRIORITY_HIGH, STATUS_PENDING, "", 1};

    q_enqueue(&q, &r1);
    q_enqueue(&q, &r2);
    TEST_ASSERT(q_size(&q) == 2, "Enqueued 2 items, size is 2");

    ReliefRequest* peeked = q_peek(&q);
    TEST_ASSERT(peeked != 0 && strcmp(peeked->requestID, "Q1") == 0, "Peeked front of queue is Q1");

    ReliefRequest out;
    q_dequeue(&q, &out);
    TEST_ASSERT(strcmp(out.requestID, "Q1") == 0, "Dequeued first item is Q1 (FIFO)");
    TEST_ASSERT(q_size(&q) == 1, "Queue size is now 1");

    q_dequeue(&q, &out);
    TEST_ASSERT(strcmp(out.requestID, "Q2") == 0, "Dequeued second item is Q2 (FIFO)");
    TEST_ASSERT(q_is_empty(&q) == 1, "Queue is now empty");
}

void test_priority_queue(void) {
    printf("\n=== TEST SUITE 4: PriorityQueue (4-Lane Urgency) ===\n");
    PriorityQueue pq;
    pq_init(&pq);
    TEST_ASSERT(pq_is_empty(&pq) == 1, "Initial PriorityQueue is empty");

    /* Insert in arbitrary order: Medium, Critical, Low, High, Critical */
    ReliefRequest rMed  = {"R_MED",  "Area M", "Pune", "Flood", 100, "Food", 50, 0, PRIORITY_MEDIUM,   STATUS_PENDING, "", 0};
    ReliefRequest rCrit1 = {"R_CRIT1","Area C1","Pune", "Flood", 100, "Food", 50, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 1};
    ReliefRequest rLow  = {"R_LOW",  "Area L", "Pune", "Flood", 100, "Food", 50, 0, PRIORITY_LOW,      STATUS_PENDING, "", 2};
    ReliefRequest rHigh = {"R_HIGH", "Area H", "Pune", "Flood", 100, "Food", 50, 0, PRIORITY_HIGH,     STATUS_PENDING, "", 3};
    ReliefRequest rCrit2 = {"R_CRIT2","Area C2","Pune", "Flood", 100, "Food", 50, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 4};

    pq_insert(&pq, &rMed);
    pq_insert(&pq, &rCrit1);
    pq_insert(&pq, &rLow);
    pq_insert(&pq, &rHigh);
    pq_insert(&pq, &rCrit2);

    TEST_ASSERT(pq_size(&pq) == 5, "Inserted 5 items into PriorityQueue");

    /* Dequeue order MUST be: R_CRIT1, R_CRIT2 (FIFO within Critical), R_HIGH, R_MED, R_LOW */
    ReliefRequest out;
    pq_dequeue(&pq, &out);
    TEST_ASSERT(strcmp(out.requestID, "R_CRIT1") == 0, "1st dequeued is R_CRIT1 (Critical)");

    pq_dequeue(&pq, &out);
    TEST_ASSERT(strcmp(out.requestID, "R_CRIT2") == 0, "2nd dequeued is R_CRIT2 (FIFO within Critical)");

    pq_dequeue(&pq, &out);
    TEST_ASSERT(strcmp(out.requestID, "R_HIGH") == 0, "3rd dequeued is R_HIGH (High)");

    pq_dequeue(&pq, &out);
    TEST_ASSERT(strcmp(out.requestID, "R_MED") == 0, "4th dequeued is R_MED (Medium)");

    pq_dequeue(&pq, &out);
    TEST_ASSERT(strcmp(out.requestID, "R_LOW") == 0, "5th dequeued is R_LOW (Low)");

    TEST_ASSERT(pq_is_empty(&pq) == 1, "PriorityQueue is empty after 5 dequeues");
}

void test_stack(void) {
    printf("\n=== TEST SUITE 5: Stack (LIFO History / Undo) ===\n");
    Stack s;
    stack_init(&s);
    TEST_ASSERT(stack_is_empty(&s) == 1, "Initial Stack is empty");

    AllocationRecord a1 = {"A1", "R101", "Food", 100, "Pune", "Completed", ""};
    AllocationRecord a2 = {"A2", "R102", "Water", 200, "Kolhapur", "Completed", ""};

    stack_push(&s, &a1);
    stack_push(&s, &a2);
    TEST_ASSERT(stack_size(&s) == 2, "Pushed 2 records onto Stack");

    AllocationRecord* top = stack_peek(&s);
    TEST_ASSERT(top != 0 && strcmp(top->allocID, "A2") == 0, "Top of stack is A2 (most recent)");

    AllocationRecord out;
    stack_pop(&s, &out);
    TEST_ASSERT(strcmp(out.allocID, "A2") == 0, "Popped A2 first (LIFO)");

    stack_pop(&s, &out);
    TEST_ASSERT(strcmp(out.allocID, "A1") == 0, "Popped A1 second (LIFO)");
    TEST_ASSERT(stack_is_empty(&s) == 1, "Stack is empty after pops");
}

void test_search_and_sort(void) {
    printf("\n=== TEST SUITE 6: Search & Bubble Sort ===\n");
    ReliefRequest arr[4] = {
        {"R104", "Village D", "Sangli", "Flood", 750, "Food", 400, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 0},
        {"R102", "Village B", "Kolhapur", "Fire", 120, "Medicines", 80, 0, PRIORITY_HIGH, STATUS_PENDING, "", 1},
        {"R101", "Village A", "Pune", "Flood", 500, "Water", 300, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 2},
        {"R103", "Village C", "Satara", "Landslide", 200, "Tents", 150, 0, PRIORITY_LOW, STATUS_PENDING, "", 3}
    };

    /* Linear Search by location */
    ReliefRequest matches[4];
    int cnt = linear_search_requests(arr, 4, "location", "kol", matches, 4);
    TEST_ASSERT(cnt == 1 && strcmp(matches[0].requestID, "R102") == 0, "Linear search found 'kol' -> R102");

    cnt = linear_search_requests(arr, 4, "disasterType", "flood", matches, 4);
    TEST_ASSERT(cnt == 2, "Linear search found 2 'flood' requests");

    /* Bubble Sort by priority (Ascending) */
    ReliefRequest sorted_p[4];
    bubble_sort_requests(arr, 4, "priority", 1, sorted_p);
    TEST_ASSERT(sorted_p[0].priority == PRIORITY_CRITICAL, "1st sorted is Critical");
    TEST_ASSERT(sorted_p[1].priority == PRIORITY_CRITICAL, "2nd sorted is Critical");
    TEST_ASSERT(sorted_p[2].priority == PRIORITY_HIGH,     "3rd sorted is High");
    TEST_ASSERT(sorted_p[3].priority == PRIORITY_LOW,      "4th sorted is Low");

    /* Bubble Sort by requestID (Ascending) */
    ReliefRequest sorted_id[4];
    bubble_sort_requests(arr, 4, "requestID", 1, sorted_id);
    TEST_ASSERT(strcmp(sorted_id[0].requestID, "R101") == 0, "Sorted by ID: 1st is R101");
    TEST_ASSERT(strcmp(sorted_id[1].requestID, "R102") == 0, "Sorted by ID: 2nd is R102");
    TEST_ASSERT(strcmp(sorted_id[2].requestID, "R103") == 0, "Sorted by ID: 3rd is R103");
    TEST_ASSERT(strcmp(sorted_id[3].requestID, "R104") == 0, "Sorted by ID: 4th is R104");

    /* Binary Search on sorted_id */
    ReliefRequest found;
    int steps[8];
    int steps_cnt = 0;
    int ok = binary_search_by_id(sorted_id, 4, "R103", &found, steps, 8, &steps_cnt);
    TEST_ASSERT(ok == 1 && strcmp(found.area, "Village C") == 0, "Binary search found R103");
    TEST_ASSERT(steps_cnt > 0, "Binary search recorded step comparisons");

    ok = binary_search_by_id(sorted_id, 4, "R999", &found, steps, 8, &steps_cnt);
    TEST_ASSERT(ok == 0, "Binary search for non-existent R999 returns 0");
}

void test_full_workflow(void) {
    printf("\n=== TEST SUITE 7: End-to-End Workflow & Undo ===\n");
    ReliefQSystem sys;
    rq_init_system(&sys);
    rq_load_sample_data(&sys);

    TEST_ASSERT(sys.allRequestsCount == 10, "10 sample requests loaded");
    TEST_ASSERT(ll_size(&sys.completedList) == 2, "2 sample requests completed on bootstrap");
    TEST_ASSERT(stack_size(&sys.historyStack) == 2, "2 allocations in history stack");

    /* Undo one allocation */
    AllocationRecord rec;
    int undo_ok = rq_undo_last(&sys, &rec);
    TEST_ASSERT(undo_ok == 1, "Undo last allocation succeeded");
    TEST_ASSERT(stack_size(&sys.historyStack) == 1, "Stack size decreased to 1");
    TEST_ASSERT(ll_size(&sys.completedList) == 1, "Completed list size decreased to 1");

    /* Allocate next from Priority Queue (R105: needs 600 Medicines, stock is 500 -> partial allocation) */
    AllocationResult ares;
    int alloc_ok = rq_allocate_next(&sys, &ares);
    TEST_ASSERT(alloc_ok == 1, "Allocated next priority request");
    TEST_ASSERT(ares.type == ALLOC_PARTIAL, "Allocation of R105 (needs 600 Medicines, 500 in stock) succeeded with ALLOC_PARTIAL");

    /* Test insufficient stock scenario */
    ReliefRequest big_req = {"R_HUGE", "Crisis Zone", "Pune", "Flood", 5000, "Tents", 99999, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 99};
    rq_submit_request(&sys, &big_req, 1);
    rq_classify_next(&sys, 0, 1);

    /* Allocate big_req: stock of Tents is only 300, so partial allocation */
    rq_allocate_request(&sys, &big_req, &ares);
    TEST_ASSERT(ares.type == ALLOC_PARTIAL, "Massive request resulted in ALLOC_PARTIAL");
    TEST_ASSERT(ares.given == 300, "Allocated all 300 available Tents");
    TEST_ASSERT(res_get_available(&sys.resourceArray, "Tents") == 0, "Tents inventory is now 0");

    /* Try allocating another Tents request when stock is 0 */
    ReliefRequest zero_stock = {"R_ZERO", "Zero Zone", "Pune", "Flood", 100, "Tents", 50, 0, PRIORITY_CRITICAL, STATUS_PENDING, "", 100};
    rq_submit_request(&sys, &zero_stock, 1);
    rq_classify_next(&sys, 0, 1);
    rq_allocate_request(&sys, &zero_stock, &ares);
    TEST_ASSERT(ares.type == ALLOC_WAIT, "0 available stock resulted in ALLOC_WAIT");
    TEST_ASSERT(ares.req.status == STATUS_WAITING_FOR_RESOURCES, "Status marked WAITING_FOR_RESOURCES");
}

int main(void) {
    printf("========================================================\n");
    printf("  RUNNING RELIEF-Q C AUTOMATED TEST SUITE (C11)\n");
    printf("========================================================\n");

    test_resource_array();
    test_linked_list();
    test_queue();
    test_priority_queue();
    test_stack();
    test_search_and_sort();
    test_full_workflow();

    printf("\n========================================================\n");
    printf("  TEST RESULTS: %d / %d PASSED\n", g_tests_passed, g_tests_run);
    printf("========================================================\n");

    return (g_tests_passed == g_tests_run) ? 0 : 1;
}

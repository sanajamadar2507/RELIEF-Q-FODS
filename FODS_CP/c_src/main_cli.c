#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "reliefq_core.h"
#include "search_sort.h"
#include "string_utils.h"

static void print_banner(void) {
    printf("=====================================================================\n");
    printf("  RELIEF-Q  --  Disaster Relief Allocation Simulator (Native C)\n");
    printf("  Fundamental Data Structures: Array, Struct, LinkedList, Queue, PQ, Stack\n");
    printf("=====================================================================\n\n");
}

static void print_menu(void) {
    printf("Select an operation:\n");
    printf("  1. View Dashboard & Statistics\n");
    printf("  2. View Resource Inventory (Array)\n");
    printf("  3. View Waiting Queue (FIFO Queue)\n");
    printf("  4. View Priority Queue Lanes (4-Level PQ)\n");
    printf("  5. View Completed Requests (Linked List)\n");
    printf("  6. View Allocation History (Stack - LIFO)\n");
    printf("  7. Submit New Relief Request (Queue)\n");
    printf("  8. Classify Next Request (Queue -> Priority Queue)\n");
    printf("  9. Allocate Next Priority Request\n");
    printf(" 10. Undo Last Allocation (Stack pop -> Restore)\n");
    printf(" 11. Search Requests (Linear / Binary Search)\n");
    printf(" 12. Sort Requests (Bubble Sort)\n");
    printf(" 13. Add / Top-up Resource Inventory\n");
    printf(" 14. Reset Simulation to Sample Data\n");
    printf("  0. Exit\n");
    printf("Choice > ");
}

static void show_dashboard(const ReliefQSystem* sys) {
    int total = sys->allRequestsCount;
    int pending = 0, critical = 0, completed = 0;
    for (int i = 0; i < total; i++) {
        if (sys->allRequests[i].status == STATUS_PENDING) pending++;
        if (sys->allRequests[i].priority == PRIORITY_CRITICAL) critical++;
        if (sys->allRequests[i].status == STATUS_COMPLETED) completed++;
    }

    printf("\n--- DASHBOARD STATS ---\n");
    printf("Total Requests:       %d\n", total);
    printf("Pending Requests:     %d\n", pending);
    printf("Critical Requests:    %d\n", critical);
    printf("Completed Requests:   %d\n", completed);
    printf("Resources Available:  %d\n", res_total_available(&sys->resourceArray));
    printf("Resources Distributed:%d\n", res_total_distributed(&sys->resourceArray));
    printf("Waiting in Queue:     %d\n", q_size(&sys->waitingQueue));
    printf("In Priority Queue:    %d\n", pq_size(&sys->priorityQueue));
    printf("In Linked List:       %d\n", ll_size(&sys->completedList));
    printf("In Stack:             %d\n\n", stack_size(&sys->historyStack));

    printf("Recent Operations Feed:\n");
    int start = (sys->eventLogCount > 5) ? (sys->eventLogCount - 5) : 0;
    for (int i = sys->eventLogCount - 1; i >= start; i--) {
        printf("  - %s\n", sys->eventLog[i].text);
    }
    printf("\n");
}

static void show_inventory(const ReliefQSystem* sys) {
    printf("\n--- RESOURCE INVENTORY (ARRAY) ---\n");
    printf("%-16s %-10s %-12s %-12s\n", "Resource", "Total", "Available", "Distributed");
    printf("----------------------------------------------------\n");
    for (int i = 0; i < sys->resourceArray.count; i++) {
        const ResourceItem* item = &sys->resourceArray.items[i];
        printf("%-16s %-10d %-12d %-12d\n", item->name, item->total, item->available, item->distributed);
    }
    printf("\n");
}

static void show_waiting_queue(const ReliefQSystem* sys) {
    printf("\n--- WAITING QUEUE (FIFO) ---\n");
    ReliefRequest arr[MAX_REQUESTS];
    int count = q_to_array(&sys->waitingQueue, arr, MAX_REQUESTS);
    if (count == 0) {
        printf("Queue is empty -- all requests have been classified.\n\n");
        return;
    }
    printf("%-4s %-8s %-14s %-10s %-14s %-6s\n", "#", "ID", "Area", "Priority", "Resource", "Qty");
    printf("------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-4d %-8s %-14s %-10s %-14s %-6d\n",
            i + 1, arr[i].requestID, arr[i].area, rq_priority_to_string(arr[i].priority),
            arr[i].resource, arr[i].quantity);
    }
    printf("\n");
}

static void show_priority_queue(const ReliefQSystem* sys) {
    printf("\n--- PRIORITY QUEUE (4 INTERNAL LANES) ---\n");
    const char* lanes[4] = {"Critical", "High", "Medium", "Low"};
    ReliefRequest arr[MAX_REQUESTS];
    for (int l = 0; l < 4; l++) {
        int cnt = q_to_array(&sys->priorityQueue.levels[l], arr, MAX_REQUESTS);
        printf("[%s Lane: %d request(s)]\n", lanes[l], cnt);
        for (int i = 0; i < cnt; i++) {
            printf("  -> %s (%s) needs %d %s\n", arr[i].requestID, arr[i].area, arr[i].quantity, arr[i].resource);
        }
    }
    printf("\n");
}

static void show_completed_list(const ReliefQSystem* sys) {
    printf("\n--- COMPLETED / DISTRIBUTED REQUESTS (LINKED LIST) ---\n");
    ReliefRequest arr[MAX_REQUESTS];
    int count = ll_to_array(&sys->completedList, arr, MAX_REQUESTS);
    if (count == 0) {
        printf("Linked List is empty -- no completed requests yet.\n\n");
        return;
    }
    printf("%-8s %-14s %-14s %-10s %-16s\n", "ID", "Area", "Resource", "Allocated", "Status");
    printf("----------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-8s %-14s %-14s %-10d %-16s\n",
            arr[i].requestID, arr[i].area, arr[i].resource, arr[i].allocated,
            rq_status_to_string(arr[i].status));
    }
    printf("\n");
}

static void show_history_stack(const ReliefQSystem* sys) {
    printf("\n--- ALLOCATION HISTORY (STACK - LIFO) ---\n");
    AllocationRecord arr[MAX_HISTORY];
    int count = stack_to_array(&sys->historyStack, arr, MAX_HISTORY);
    if (count == 0) {
        printf("Stack is empty -- no allocations performed yet.\n\n");
        return;
    }
    printf("%-8s %-8s %-14s %-8s %-14s %-16s\n", "AllocID", "ReqID", "Resource", "Qty", "Location", "Status");
    printf("------------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {
        printf("%-8s %-8s %-14s %-8d %-14s %-16s\n",
            arr[i].allocID, arr[i].requestID, arr[i].resource, arr[i].quantity,
            arr[i].location, arr[i].status);
    }
    printf("\n");
}

int main(void) {
    ReliefQSystem sys;
    rq_init_system(&sys);
    rq_load_sample_data(&sys);

    print_banner();
    printf("Loaded sample data: 7 resource categories, 10 sample requests.\n");
    printf("Silently pre-allocated R101 and R104 to populate initial simulation state.\n\n");

    char line[128];
    while (1) {
        print_menu();
        if (!fgets(line, sizeof(line), stdin)) break;
        int choice = atoi(line);

        if (choice == 0) {
            printf("Exiting RELIEF-Q C Simulator. Goodbye!\n");
            break;
        }

        switch (choice) {
            case 1:
                show_dashboard(&sys);
                break;
            case 2:
                show_inventory(&sys);
                break;
            case 3:
                show_waiting_queue(&sys);
                break;
            case 4:
                show_priority_queue(&sys);
                break;
            case 5:
                show_completed_list(&sys);
                break;
            case 6:
                show_history_stack(&sys);
                break;
            case 7: {
                ReliefRequest req;
                printf("Enter Request ID (e.g. R111): ");
                if (!fgets(req.requestID, sizeof(req.requestID), stdin)) break;
                req.requestID[strcspn(req.requestID, "\r\n")] = 0;

                printf("Enter Disaster Area (e.g. Village K): ");
                if (!fgets(req.area, sizeof(req.area), stdin)) break;
                req.area[strcspn(req.area, "\r\n")] = 0;

                printf("Enter Location (e.g. Pune): ");
                if (!fgets(req.location, sizeof(req.location), stdin)) break;
                req.location[strcspn(req.location, "\r\n")] = 0;

                printf("Enter Disaster Type (Flood/Fire/Earthquake/Cyclone/Landslide/Drought): ");
                if (!fgets(req.disasterType, sizeof(req.disasterType), stdin)) break;
                req.disasterType[strcspn(req.disasterType, "\r\n")] = 0;

                printf("Enter Affected Population: ");
                if (!fgets(line, sizeof(line), stdin)) break;
                req.population = atoi(line);

                printf("Enter Resource Required (Food/Water/Medicines/Blankets/Tents/Clothes/Emergency Kits): ");
                if (!fgets(req.resource, sizeof(req.resource), stdin)) break;
                req.resource[strcspn(req.resource, "\r\n")] = 0;

                printf("Enter Quantity Required: ");
                if (!fgets(line, sizeof(line), stdin)) break;
                req.quantity = atoi(line);

                printf("Enter Priority (Critical/High/Medium/Low): ");
                if (!fgets(line, sizeof(line), stdin)) break;
                line[strcspn(line, "\r\n")] = 0;
                req.priority = rq_string_to_priority(line);

                if (rq_submit_request(&sys, &req, 0)) {
                    printf("\n[SUCCESS] Request %s enqueued into waiting Queue!\n\n", req.requestID);
                } else {
                    printf("\n[ERROR] Failed to enqueue request.\n\n");
                }
                break;
            }
            case 8: {
                ReliefRequest req;
                if (rq_classify_next(&sys, &req, 0)) {
                    printf("\n[SUCCESS] %s moved to Priority Queue (Priority: %s)\n\n",
                        req.requestID, rq_priority_to_string(req.priority));
                } else {
                    printf("\n[INFO] Waiting Queue is empty -- no requests to classify.\n\n");
                }
                break;
            }
            case 9: {
                AllocationResult res;
                if (rq_allocate_next(&sys, &res)) {
                    if (res.type == ALLOC_WAIT) {
                        printf("\n[WAIT] %s placed on Waiting List (0 units of %s available)\n\n",
                            res.req.requestID, res.req.resource);
                    } else if (res.type == ALLOC_PARTIAL) {
                        printf("\n[PARTIAL] %s partially allocated %d units of %s (remaining: %d)\n\n",
                            res.req.requestID, res.given, res.req.resource, res.req.quantity - res.req.allocated);
                    } else if (res.type == ALLOC_OK) {
                        printf("\n[COMPLETED] %s fully allocated %d units of %s!\n\n",
                            res.req.requestID, res.given, res.req.resource);
                    }
                } else {
                    printf("\n[INFO] Priority Queue is empty -- classify more requests first.\n\n");
                }
                break;
            }
            case 10: {
                AllocationRecord rec;
                if (rq_undo_last(&sys, &rec)) {
                    printf("\n[UNDO] Allocation %s reversed: returned %d %s to inventory.\n\n",
                        rec.allocID, rec.quantity, rec.resource);
                } else {
                    printf("\n[INFO] Stack is empty -- no allocations to undo.\n\n");
                }
                break;
            }
            case 11: {
                printf("Enter search keyword: ");
                if (!fgets(line, sizeof(line), stdin)) break;
                line[strcspn(line, "\r\n")] = 0;

                ReliefRequest matches[MAX_REQUESTS];
                int cnt = linear_search_requests(sys.allRequests, sys.allRequestsCount, "location", line, matches, MAX_REQUESTS);
                if (cnt == 0) {
                    cnt = linear_search_requests(sys.allRequests, sys.allRequestsCount, "requestID", line, matches, MAX_REQUESTS);
                }
                printf("\nFound %d matching record(s):\n", cnt);
                for (int i = 0; i < cnt; i++) {
                    printf("  - %s: %s (%s) -- %d %s [%s] Status: %s\n",
                        matches[i].requestID, matches[i].area, matches[i].location,
                        matches[i].quantity, matches[i].resource,
                        rq_priority_to_string(matches[i].priority),
                        rq_status_to_string(matches[i].status));
                }
                printf("\n");
                break;
            }
            case 12: {
                printf("Sort by: 1. Priority  2. Population  3. Quantity  4. RequestID\nChoice > ");
                if (!fgets(line, sizeof(line), stdin)) break;
                int s_field = atoi(line);
                const char* f_name = "priority";
                if (s_field == 2) f_name = "population";
                else if (s_field == 3) f_name = "quantity";
                else if (s_field == 4) f_name = "requestID";

                ReliefRequest sorted[MAX_REQUESTS];
                bubble_sort_requests(sys.allRequests, sys.allRequestsCount, f_name, 1, sorted);
                printf("\n--- BUBBLE SORT RESULTS (by %s, Ascending) ---\n", f_name);
                for (int i = 0; i < sys.allRequestsCount; i++) {
                    printf("%-8s | %-10s | Pop: %-5d | Qty: %-5d\n",
                        sorted[i].requestID, rq_priority_to_string(sorted[i].priority),
                        sorted[i].population, sorted[i].quantity);
                }
                printf("\n");
                break;
            }
            case 13: {
                char resName[32];
                printf("Enter resource name (e.g. Food, Water, etc.): ");
                if (!fgets(resName, sizeof(resName), stdin)) break;
                resName[strcspn(resName, "\r\n")] = 0;

                printf("Enter quantity to add: ");
                if (!fgets(line, sizeof(line), stdin)) break;
                int qty = atoi(line);

                if (rq_add_resource(&sys, resName, qty)) {
                    printf("\n[SUCCESS] Added %d units of %s to inventory.\n\n", qty, resName);
                } else {
                    printf("\n[ERROR] Failed to add resource.\n\n");
                }
                break;
            }
            case 14:
                rq_init_system(&sys);
                rq_load_sample_data(&sys);
                printf("\n[RESET] Simulation reset to initial sample state.\n\n");
                break;
            default:
                printf("Invalid option. Please try again.\n\n");
                break;
        }
    }
    return 0;
}

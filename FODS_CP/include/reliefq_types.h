#ifndef RELIEFQ_TYPES_H
#define RELIEFQ_TYPES_H

#define MAX_REQUESTS        128
#define MAX_RESOURCES       32
#define MAX_HISTORY         256
#define MAX_LOG_EVENTS      64
#define MAX_SEARCH_RESULTS  128
#define MAX_SORT_RESULTS    128
#define JSON_BUFFER_SIZE    (256 * 1024)

typedef enum {
    PRIORITY_CRITICAL = 0,
    PRIORITY_HIGH     = 1,
    PRIORITY_MEDIUM   = 2,
    PRIORITY_LOW      = 3,
    PRIORITY_UNKNOWN  = -1
} PriorityLevel;

typedef enum {
    STATUS_PENDING               = 0,
    STATUS_WAITING_FOR_RESOURCES = 1,
    STATUS_PARTIALLY_ALLOCATED   = 2,
    STATUS_COMPLETED             = 3
} RequestStatus;

typedef enum {
    ALLOC_EMPTY   = 0,
    ALLOC_WAIT    = 1,
    ALLOC_PARTIAL = 2,
    ALLOC_OK      = 3
} AllocationType;

/* Structure: ReliefRequest */
typedef struct {
    char requestID[32];
    char area[64];
    char location[64];
    char disasterType[32];
    int  population;
    char resource[32];
    int  quantity;       /* required quantity */
    int  allocated;      /* quantity given so far */
    PriorityLevel priority;
    RequestStatus status;
    char requestTime[32];
    int  arrivalOrder;   /* FIFO tie-breaker within same priority */
} ReliefRequest;

/* Resource Item in ResourceArray */
typedef struct {
    char name[32];
    int  total;
    int  available;
    int  distributed;
} ResourceItem;

/* Allocation Record for History Stack (LIFO) */
typedef struct {
    char allocID[32];
    char requestID[32];
    char resource[32];
    int  quantity;
    char location[64];
    char status[32];
    char time[32];
} AllocationRecord;

/* Allocation Result returned by allocate operations */
typedef struct {
    AllocationType type;
    ReliefRequest  req;
    int            given;
    AllocationRecord record;
} AllocationResult;

/* Event Log record for Live Operations Feed */
typedef struct {
    char time[32];
    char text[256];
} EventLog;

#endif /* RELIEFQ_TYPES_H */

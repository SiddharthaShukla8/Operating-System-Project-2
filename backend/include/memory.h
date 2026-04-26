#ifndef MEMORY_H
#define MEMORY_H

typedef enum MemoryOwner {
    MEMORY_OWNER_NONE = 0,
    MEMORY_OWNER_PERSISTENT = 1,
    MEMORY_OWNER_SESSION = 2,
    MEMORY_OWNER_TEMP = 3
} MemoryOwner;

typedef struct MemoryBlock {
    int offset;
    int size;
    int used;
    int active;
    int allocation_id;
    MemoryOwner owner;
    struct MemoryBlock *next;
} MemoryBlock;

typedef struct MemoryStats {
    int total_pool;
    int used_bytes;
    int free_bytes;
    int allocations;
    int deallocations;
    int active_allocations;
    int fragmentation_blocks;
    int largest_free_block;
    int session_allocations;
    int session_deallocations;
    int session_used_bytes;
} MemoryStats;

void memory_init(void);
void *memory_alloc(int size);
void *memory_alloc_owned(int size, MemoryOwner owner);
int memory_free(void *ptr);
void memory_reset(void);
MemoryStats memory_usage(void);
const char *memory_debug(void);

/* Compatibility wrappers for older code paths */
void *my_alloc(int size);
void my_reset(void);

#endif

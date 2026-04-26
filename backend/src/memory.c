#include "../include/memory.h"
#include <stdio.h>

#define MEMORY_POOL_SIZE 65536
#define MAX_MEMORY_BLOCKS 2048

static unsigned char memory_pool[MEMORY_POOL_SIZE];
static MemoryBlock block_table[MAX_MEMORY_BLOCKS];
static MemoryBlock *memory_head = 0;
static int memory_initialized = 0;

static int total_allocations = 0;
static int total_deallocations = 0;
static int session_allocations = 0;
static int session_deallocations = 0;
static int next_allocation_id = 1;

static void clear_descriptor(MemoryBlock *block) {
    if (!block) return;

    block->offset = 0;
    block->size = 0;
    block->used = 0;
    block->active = 0;
    block->allocation_id = 0;
    block->owner = MEMORY_OWNER_NONE;
    block->next = 0;
}

static MemoryBlock *memory_new_block(void) {
    int index = 0;

    while (index < MAX_MEMORY_BLOCKS) {
        if (!block_table[index].active) {
            block_table[index].active = 1;
            block_table[index].used = 0;
            block_table[index].allocation_id = 0;
            block_table[index].owner = MEMORY_OWNER_NONE;
            block_table[index].next = 0;
            return &block_table[index];
        }
        index++;
    }

    return 0;
}

static void memory_merge_free_blocks(void) {
    MemoryBlock *current = memory_head;

    while (current && current->next) {
        if (!current->used && !current->next->used) {
            MemoryBlock *merged = current->next;
            current->size += merged->size;
            current->next = merged->next;
            clear_descriptor(merged);
            continue;
        }

        current = current->next;
    }
}

static void memory_mark_block_free(MemoryBlock *block, int track_stats) {
    MemoryOwner previous_owner;

    if (!block || !block->used) return;

    previous_owner = block->owner;
    block->used = 0;
    block->allocation_id = 0;
    block->owner = MEMORY_OWNER_NONE;

    if (track_stats) {
        total_deallocations++;
        if (previous_owner != MEMORY_OWNER_PERSISTENT) {
            session_deallocations++;
        }
    }
}

void memory_init(void) {
    int index = 0;

    if (memory_initialized) return;

    while (index < MAX_MEMORY_BLOCKS) {
        clear_descriptor(&block_table[index]);
        index++;
    }

    memory_head = &block_table[0];
    memory_head->offset = 0;
    memory_head->size = MEMORY_POOL_SIZE;
    memory_head->used = 0;
    memory_head->active = 1;
    memory_head->allocation_id = 0;
    memory_head->owner = MEMORY_OWNER_NONE;
    memory_head->next = 0;

    memory_initialized = 1;
}

void *memory_alloc_owned(int size, MemoryOwner owner) {
    MemoryBlock *current;

    if (size <= 0) return 0;

    memory_init();
    current = memory_head;

    while (current) {
        if (!current->used && current->size >= size) {
            if (current->size > size) {
                MemoryBlock *split = memory_new_block();
                if (!split) return 0;

                split->offset = current->offset + size;
                split->size = current->size - size;
                split->used = 0;
                split->owner = MEMORY_OWNER_NONE;
                split->allocation_id = 0;
                split->next = current->next;
                current->next = split;
                current->size = size;
            }

            current->used = 1;
            current->owner = owner;
            current->allocation_id = next_allocation_id++;
            total_allocations++;
            if (owner != MEMORY_OWNER_PERSISTENT) {
                session_allocations++;
            }

            return (void *)(memory_pool + current->offset);
        }

        current = current->next;
    }

    return 0;
}

void *memory_alloc(int size) {
    return memory_alloc_owned(size, MEMORY_OWNER_SESSION);
}

int memory_free(void *ptr) {
    MemoryBlock *current;
    int offset;

    if (!ptr) return 0;

    memory_init();
    offset = (int)((unsigned char *)ptr - memory_pool);

    if (offset < 0 || offset >= MEMORY_POOL_SIZE) {
        return 0;
    }

    current = memory_head;
    while (current) {
        if (current->used && current->offset == offset) {
            memory_mark_block_free(current, 1);
            memory_merge_free_blocks();
            return 1;
        }
        current = current->next;
    }

    return 0;
}

void memory_reset(void) {
    MemoryBlock *current;

    memory_init();

    current = memory_head;
    while (current) {
        if (current->used && current->owner != MEMORY_OWNER_PERSISTENT) {
            memory_mark_block_free(current, 1);
        }
        current = current->next;
    }

    memory_merge_free_blocks();
    session_allocations = 0;
    session_deallocations = 0;
}

MemoryStats memory_usage(void) {
    MemoryStats stats;
    MemoryBlock *current;

    memory_init();

    stats.total_pool = MEMORY_POOL_SIZE;
    stats.used_bytes = 0;
    stats.free_bytes = 0;
    stats.allocations = total_allocations;
    stats.deallocations = total_deallocations;
    stats.active_allocations = 0;
    stats.fragmentation_blocks = 0;
    stats.largest_free_block = 0;
    stats.session_allocations = session_allocations;
    stats.session_deallocations = session_deallocations;
    stats.session_used_bytes = 0;

    current = memory_head;
    while (current) {
        if (current->used) {
            stats.used_bytes += current->size;
            stats.active_allocations++;

            if (current->owner != MEMORY_OWNER_PERSISTENT) {
                stats.session_used_bytes += current->size;
            }
        } else {
            stats.free_bytes += current->size;
            stats.fragmentation_blocks++;
            if (current->size > stats.largest_free_block) {
                stats.largest_free_block = current->size;
            }
        }

        current = current->next;
    }

    return stats;
}

const char *memory_debug(void) {
    static char buffer[256];
    MemoryStats stats = memory_usage();

    snprintf(
        buffer,
        sizeof(buffer),
        "Total Pool: %d bytes\nUsed: %d bytes\nFree: %d bytes\nAllocations: %d\nDeallocations: %d\nFragmentation Blocks: %d",
        stats.total_pool,
        stats.used_bytes,
        stats.free_bytes,
        stats.allocations,
        stats.deallocations,
        stats.fragmentation_blocks
    );

    return buffer;
}

void *my_alloc(int size) {
    return memory_alloc(size);
}

void my_reset(void) {
    memory_reset();
}

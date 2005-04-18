/**
 * @file memory_pool.h
 * @brief Zero-fragmentation slab/arena memory pool for ultra-low latency packet switching.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#ifndef TELECOM_MEMORY_POOL_H
#define TELECOM_MEMORY_POOL_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct telecom_slab {
    void *memory;
    size_t block_size;
    size_t num_blocks;
    uint8_t *bitmap;
    size_t free_blocks;
} telecom_slab_t;

typedef struct telecom_memory_pool {
    telecom_slab_t small_slab;   /* 64 bytes (Signaling packets) */
    telecom_slab_t medium_slab;  /* 512 bytes (ISUP messages) */
    telecom_slab_t large_slab;   /* 4096 bytes (CDR records / PCM frame) */
    uint64_t total_allocations;
    uint64_t total_deallocations;
} telecom_memory_pool_t;

telecom_memory_pool_t* memory_pool_create(size_t small_count, size_t med_count, size_t large_count);
void memory_pool_destroy(telecom_memory_pool_t *pool);

void* pool_alloc(telecom_memory_pool_t *pool, size_t size);
void pool_free(telecom_memory_pool_t *pool, void *ptr);

void memory_pool_stats(const telecom_memory_pool_t *pool, uint64_t *allocs, uint64_t *frees);

#ifdef __cplusplus
}
#endif

#endif /* TELECOM_MEMORY_POOL_H */

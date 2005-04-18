/**
 * @file memory_pool.c
 * @brief Slab/arena memory allocator for zero-fragmentation switching.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#include "telecom/memory_pool.h"
#include <stdlib.h>
#include <string.h>

static bool init_slab(telecom_slab_t *slab, size_t block_size, size_t num_blocks) {
    slab->block_size = block_size;
    slab->num_blocks = num_blocks;
    slab->free_blocks = num_blocks;
    
    slab->memory = malloc(block_size * num_blocks);
    if (!slab->memory) return false;
    
    size_t bitmap_bytes = (num_blocks + 7) / 8;
    slab->bitmap = (uint8_t*)calloc(bitmap_bytes, sizeof(uint8_t));
    if (!slab->bitmap) {
        free(slab->memory);
        return false;
    }
    
    return true;
}

static void free_slab(telecom_slab_t *slab) {
    if (slab->memory) free(slab->memory);
    if (slab->bitmap) free(slab->bitmap);
}

telecom_memory_pool_t* memory_pool_create(size_t small_count, size_t med_count, size_t large_count) {
    telecom_memory_pool_t *pool = (telecom_memory_pool_t*)malloc(sizeof(telecom_memory_pool_t));
    if (!pool) return NULL;
    
    memset(pool, 0, sizeof(telecom_memory_pool_t));
    
    if (!init_slab(&pool->small_slab, 64, small_count) ||
        !init_slab(&pool->medium_slab, 512, med_count) ||
        !init_slab(&pool->large_slab, 4096, large_count)) {
        memory_pool_destroy(pool);
        return NULL;
    }
    
    return pool;
}

void memory_pool_destroy(telecom_memory_pool_t *pool) {
    if (pool) {
        free_slab(&pool->small_slab);
        free_slab(&pool->medium_slab);
        free_slab(&pool->large_slab);
        free(pool);
    }
}

static void* slab_alloc(telecom_slab_t *slab) {
    if (slab->free_blocks == 0) return NULL;
    
    for (size_t i = 0; i < slab->num_blocks; i++) {
        size_t byte_idx = i / 8;
        uint8_t bit_mask = 1 << (i % 8);
        if (!(slab->bitmap[byte_idx] & bit_mask)) {
            slab->bitmap[byte_idx] |= bit_mask;
            slab->free_blocks--;
            return (uint8_t*)slab->memory + (i * slab->block_size);
        }
    }
    return NULL;
}

static bool slab_contains(const telecom_slab_t *slab, const void *ptr) {
    uintptr_t start = (uintptr_t)slab->memory;
    uintptr_t end = start + (slab->block_size * slab->num_blocks);
    uintptr_t p = (uintptr_t)ptr;
    return (p >= start && p < end);
}

static void slab_free(telecom_slab_t *slab, void *ptr) {
    if (!slab_contains(slab, ptr)) return;
    
    size_t offset = (uint8_t*)ptr - (uint8_t*)slab->memory;
    size_t idx = offset / slab->block_size;
    size_t byte_idx = idx / 8;
    uint8_t bit_mask = 1 << (idx % 8);
    
    if (slab->bitmap[byte_idx] & bit_mask) {
        slab->bitmap[byte_idx] &= ~bit_mask;
        slab->free_blocks++;
    }
}

void* pool_alloc(telecom_memory_pool_t *pool, size_t size) {
    if (!pool) return NULL;
    
    void *ptr = NULL;
    if (size <= 64) {
        ptr = slab_alloc(&pool->small_slab);
    } else if (size <= 512) {
        ptr = slab_alloc(&pool->medium_slab);
    } else if (size <= 4096) {
        ptr = slab_alloc(&pool->large_slab);
    } else {
        /* Fallback for oversized packets */
        ptr = malloc(size);
    }
    
    if (ptr) pool->total_allocations++;
    return ptr;
}

void pool_free(telecom_memory_pool_t *pool, void *ptr) {
    if (!pool || !ptr) return;
    
    if (slab_contains(&pool->small_slab, ptr)) {
        slab_free(&pool->small_slab, ptr);
    } else if (slab_contains(&pool->medium_slab, ptr)) {
        slab_free(&pool->medium_slab, ptr);
    } else if (slab_contains(&pool->large_slab, ptr)) {
        slab_free(&pool->large_slab, ptr);
    } else {
        free(ptr);
    }
    
    pool->total_deallocations++;
}

void memory_pool_stats(const telecom_memory_pool_t *pool, uint64_t *allocs, uint64_t *frees) {
    if (pool) {
        if (allocs) *allocs = pool->total_allocations;
        if (frees) *frees = pool->total_deallocations;
    }
}

/**
 * @file ring_buffer.h
 * @brief High-throughput lock-free circular ring buffer with x86 memory barriers.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#ifndef TELECOM_RING_BUFFER_H
#define TELECOM_RING_BUFFER_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

#define CACHE_LINE_SIZE 64

/**
 * @struct telecom_ring_buffer
 * @brief Cache-aligned lock-free single-producer single-consumer circular buffer.
 */
typedef struct telecom_ring_buffer {
    uint8_t *buffer;
    size_t capacity;       /* Must be power of 2 */
    size_t mask;
    
    /* Producer cache line */
    alignas(CACHE_LINE_SIZE) volatile size_t head;
    
    /* Consumer cache line */
    alignas(CACHE_LINE_SIZE) volatile size_t tail;
    
    /* Metrics */
    uint64_t total_pushed;
    uint64_t total_popped;
    uint64_t drops;
} telecom_ring_buffer_t;

telecom_ring_buffer_t* ring_buffer_create(size_t capacity);
void ring_buffer_destroy(telecom_ring_buffer_t *rb);

bool ring_buffer_push(telecom_ring_buffer_t *rb, const void *data, size_t len);
bool ring_buffer_pop(telecom_ring_buffer_t *rb, void *dest, size_t len);
size_t ring_buffer_available_read(const telecom_ring_buffer_t *rb);
size_t ring_buffer_available_write(const telecom_ring_buffer_t *rb);

#ifdef __cplusplus
}
#endif

#endif /* TELECOM_RING_BUFFER_H */

/**
 * @file ring_buffer.c
 * @brief Lock-free circular ring buffer implementation.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#include "telecom/ring_buffer.h"
#include <stdlib.h>
#include <string.h>

#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__) || defined(_M_IX86)
#include <immintrin.h>
#define COMPILER_BARRIER() __asm__ __volatile__("" ::: "memory")
#else
#define COMPILER_BARRIER()
#endif

static bool is_power_of_two(size_t n) {
    return (n != 0) && ((n & (n - 1)) == 0);
}

telecom_ring_buffer_t* ring_buffer_create(size_t capacity) {
    if (!is_power_of_two(capacity)) {
        /* Round up to nearest power of two */
        size_t p = 1;
        while (p < capacity) p <<= 1;
        capacity = p;
    }
    
    telecom_ring_buffer_t *rb = (telecom_ring_buffer_t*)malloc(sizeof(telecom_ring_buffer_t));
    if (!rb) return NULL;
    
    rb->buffer = (uint8_t*)malloc(capacity);
    if (!rb->buffer) {
        free(rb);
        return NULL;
    }
    
    rb->capacity = capacity;
    rb->mask = capacity - 1;
    rb->head = 0;
    rb->tail = 0;
    rb->total_pushed = 0;
    rb->total_popped = 0;
    rb->drops = 0;
    
    return rb;
}

void ring_buffer_destroy(telecom_ring_buffer_t *rb) {
    if (rb) {
        if (rb->buffer) free(rb->buffer);
        free(rb);
    }
}

size_t ring_buffer_available_write(const telecom_ring_buffer_t *rb) {
    size_t head = rb->head;
    size_t tail = rb->tail;
    return rb->capacity - (head - tail);
}

size_t ring_buffer_available_read(const telecom_ring_buffer_t *rb) {
    size_t head = rb->head;
    size_t tail = rb->tail;
    return head - tail;
}

bool ring_buffer_push(telecom_ring_buffer_t *rb, const void *data, size_t len) {
    if (!rb || !data || len == 0) return false;
    
    if (ring_buffer_available_write(rb) < len) {
        rb->drops++;
        return false;
    }
    
    size_t head = rb->head;
    size_t offset = head & rb->mask;
    size_t to_end = rb->capacity - offset;
    
    if (len <= to_end) {
        memcpy(rb->buffer + offset, data, len);
    } else {
        memcpy(rb->buffer + offset, data, to_end);
        memcpy(rb->buffer, (const uint8_t*)data + to_end, len - to_end);
    }
    
    COMPILER_BARRIER();
    rb->head = head + len;
    rb->total_pushed += len;
    
    return true;
}

bool ring_buffer_pop(telecom_ring_buffer_t *rb, void *dest, size_t len) {
    if (!rb || !dest || len == 0) return false;
    
    if (ring_buffer_available_read(rb) < len) {
        return false;
    }
    
    size_t tail = rb->tail;
    size_t offset = tail & rb->mask;
    size_t to_end = rb->capacity - offset;
    
    if (len <= to_end) {
        memcpy(dest, rb->buffer + offset, len);
    } else {
        memcpy(dest, rb->buffer + offset, to_end);
        memcpy((uint8_t*)dest + to_end, rb->buffer, len - to_end);
    }
    
    COMPILER_BARRIER();
    rb->tail = tail + len;
    rb->total_popped += len;
    
    return true;
}

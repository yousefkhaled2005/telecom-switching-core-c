/**
 * @file packet_switch.h
 * @brief High-throughput zero-copy asynchronous telecom packet switching core.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#ifndef TELECOM_PACKET_SWITCH_H
#define TELECOM_PACKET_SWITCH_H

#include "telecom/ss7_isup.h"
#include "telecom/memory_pool.h"
#include "telecom/ring_buffer.h"
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_ROUTES 1024
#define MAX_CIRCUITS 65536

typedef struct telecom_route {
    uint32_t destination_prefix;
    uint32_t prefix_length;
    uint32_t gateway_point_code;
    uint8_t trunk_group_id;
    uint32_t cost_metric;
    bool is_active;
} telecom_route_t;

typedef struct switch_metrics {
    uint64_t total_packets_routed;
    uint64_t total_iam_processed;
    uint64_t total_calls_connected;
    uint64_t total_calls_released;
    uint64_t active_concurrent_calls;
    uint64_t route_lookup_failures;
    uint64_t processing_latency_sum_ns;
} switch_metrics_t;

typedef struct packet_switch {
    telecom_memory_pool_t *mem_pool;
    telecom_ring_buffer_t *ingress_queue;
    telecom_ring_buffer_t *egress_queue;
    
    telecom_route_t routing_table[MAX_ROUTES];
    size_t route_count;
    
    isup_circuit_t *circuits[MAX_CIRCUITS];
    switch_metrics_t metrics;
    
    volatile bool running;
} packet_switch_t;

packet_switch_t* packet_switch_create(size_t queue_depth);
void packet_switch_destroy(packet_switch_t *ps);

bool packet_switch_add_route(packet_switch_t *ps, uint32_t prefix, uint32_t len, uint32_t dpc, uint8_t trunk);
const telecom_route_t* packet_switch_lookup_route(const packet_switch_t *ps, const char *dialed_digits);

bool packet_switch_process_signaling_frame(packet_switch_t *ps, const uint8_t *frame_bytes, size_t len);
void packet_switch_dump_metrics(const packet_switch_t *ps);

#ifdef __cplusplus
}
#endif

#endif /* TELECOM_PACKET_SWITCH_H */

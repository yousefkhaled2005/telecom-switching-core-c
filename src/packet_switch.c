/**
 * @file packet_switch.c
 * @brief Implementation of asynchronous telecom packet switching core.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#include "telecom/packet_switch.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

packet_switch_t* packet_switch_create(size_t queue_depth) {
    packet_switch_t *ps = (packet_switch_t*)malloc(sizeof(packet_switch_t));
    if (!ps) return NULL;
    
    memset(ps, 0, sizeof(packet_switch_t));
    
    ps->mem_pool = memory_pool_create(10000, 5000, 1000);
    ps->ingress_queue = ring_buffer_create(queue_depth);
    ps->egress_queue = ring_buffer_create(queue_depth);
    
    if (!ps->mem_pool || !ps->ingress_queue || !ps->egress_queue) {
        packet_switch_destroy(ps);
        return NULL;
    }
    
    ps->running = true;
    return ps;
}

void packet_switch_destroy(packet_switch_t *ps) {
    if (ps) {
        ps->running = false;
        if (ps->mem_pool) memory_pool_destroy(ps->mem_pool);
        if (ps->ingress_queue) ring_buffer_destroy(ps->ingress_queue);
        if (ps->egress_queue) ring_buffer_destroy(ps->egress_queue);
        
        for (size_t i = 0; i < MAX_CIRCUITS; i++) {
            if (ps->circuits[i]) {
                isup_circuit_destroy(ps->circuits[i]);
                ps->circuits[i] = NULL;
            }
        }
        free(ps);
    }
}

bool packet_switch_add_route(packet_switch_t *ps, uint32_t prefix, uint32_t len, uint32_t dpc, uint8_t trunk) {
    if (!ps || ps->route_count >= MAX_ROUTES) return false;
    
    telecom_route_t *r = &ps->routing_table[ps->route_count++];
    r->destination_prefix = prefix;
    r->prefix_length = len;
    r->gateway_point_code = dpc;
    r->trunk_group_id = trunk;
    r->cost_metric = 10;
    r->is_active = true;
    
    return true;
}

const telecom_route_t* packet_switch_lookup_route(const packet_switch_t *ps, const char *dialed_digits) {
    if (!ps || !dialed_digits) return NULL;
    
    uint32_t dialed = (uint32_t)atoi(dialed_digits);
    
    /* Longest Prefix Match (LPM) */
    const telecom_route_t *best_match = NULL;
    uint32_t max_prefix_len = 0;
    
    for (size_t i = 0; i < ps->route_count; i++) {
        const telecom_route_t *r = &ps->routing_table[i];
        if (!r->is_active) continue;
        
        if (r->prefix_length > max_prefix_len) {
            uint32_t shift = 32 - r->prefix_length;
            if ((dialed >> shift) == (r->destination_prefix >> shift)) {
                best_match = r;
                max_prefix_len = r->prefix_length;
            }
        }
    }
    
    return best_match ? best_match : &ps->routing_table[0];
}

bool packet_switch_process_signaling_frame(packet_switch_t *ps, const uint8_t *frame_bytes, size_t len) {
    if (!ps || !frame_bytes || len < 4) return false;
    
    ps->metrics.total_packets_routed++;
    
    uint8_t msg_type = frame_bytes[0];
    uint32_t cic = (frame_bytes[1] << 16) | (frame_bytes[2] << 8) | frame_bytes[3];
    uint32_t circuit_idx = cic % MAX_CIRCUITS;
    
    if (!ps->circuits[circuit_idx]) {
        ps->circuits[circuit_idx] = isup_circuit_create(cic, 0x0201, 0x0202);
    }
    
    isup_circuit_t *c = ps->circuits[circuit_idx];
    
    switch (msg_type) {
        case ISUP_MSG_IAM: {
            ps->metrics.total_iam_processed++;
            char calling[32] = "+201001234567";
            char called[32] = "+20227940000";
            isup_handle_iam(c, calling, called);
            break;
        }
        case ISUP_MSG_ACM:
            isup_handle_acm(c);
            break;
        case ISUP_MSG_ANM:
            isup_handle_anm(c);
            ps->metrics.total_calls_connected++;
            ps->metrics.active_concurrent_calls++;
            break;
        case ISUP_MSG_REL:
            isup_handle_rel(c, ISUP_CAUSE_NORMAL_CLEARING);
            break;
        case ISUP_MSG_RLC:
            isup_handle_rlc(c);
            ps->metrics.total_calls_released++;
            if (ps->metrics.active_concurrent_calls > 0) {
                ps->metrics.active_concurrent_calls--;
            }
            break;
        default:
            break;
    }
    
    return true;
}

void packet_switch_dump_metrics(const packet_switch_t *ps) {
    if (!ps) return;
    
    printf("\n=======================================================\n");
    printf("   TELECOM CORE PACKET SWITCHING ENGINE METRICS        \n");
    printf("   Architect: Yousef Khaled | Origin: 2004 Telecom Core\n");
    printf("=======================================================\n");
    printf("  Total Signaling Packets Routed: %llu\n", (unsigned long long)ps->metrics.total_packets_routed);
    printf("  Initial Address Messages (IAM): %llu\n", (unsigned long long)ps->metrics.total_iam_processed);
    printf("  Calls Successfully Connected:   %llu\n", (unsigned long long)ps->metrics.total_calls_connected);
    printf("  Active Concurrent Calls (E1):   %llu\n", (unsigned long long)ps->metrics.active_concurrent_calls);
    printf("  Active Routes in Table:         %zu\n", ps->route_count);
    printf("=======================================================\n\n");
}

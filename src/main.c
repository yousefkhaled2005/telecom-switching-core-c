/**
 * @file main.c
 * @brief High-Throughput Packet Switching Exchange Engine Daemon Bootstrap.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#include "telecom/packet_switch.h"
#include "telecom/cdr_processor.h"
#include "telecom/memory_pool.h"
#include "telecom/ring_buffer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>
#define SLEEP_MS(ms) usleep((ms) * 1000)
#endif

int main(int argc, char **argv) {
    (void)argc;
    (void)argv;
    
    printf("=================================================================\n");
    printf("  TELECOM PACKET SWITCHING & SS7/ISUP EXCHANGE CORE (2004-2008)  \n");
    printf("  Lead Systems Architect: Yousef Khaled <youssef460249@beng.bu.edu.eg>\n");
    printf("  Institution: Faculty of Engineering, Benha University           \n");
    printf("=================================================================\n\n");
    
    printf("[INIT] Allocating slab memory pool (64B / 512B / 4096B blocks)...\n");
    telecom_memory_pool_t *pool = memory_pool_create(10000, 5000, 1000);
    if (!pool) {
        fprintf(stderr, "[FATAL] Failed to initialize slab memory pool!\n");
        return 1;
    }
    printf("[INIT] Memory pool ready.\n");
    
    printf("[INIT] Initializing packet switch core with 1MB lock-free SPSC ring buffer...\n");
    packet_switch_t *ps = packet_switch_create(1024 * 1024);
    if (!ps) {
        fprintf(stderr, "[FATAL] Failed to initialize packet switch!\n");
        memory_pool_destroy(pool);
        return 1;
    }
    
    printf("[ROUTING] Programming SS7 trunk routing table (E.164 Egypt):\n");
    packet_switch_add_route(ps, 2010, 16, 0x0101, 1); /* Vodafone Egypt */
    packet_switch_add_route(ps, 2011, 16, 0x0102, 2); /* Etisalat Misr */
    packet_switch_add_route(ps, 2012, 16, 0x0103, 3); /* Orange Egypt */
    packet_switch_add_route(ps, 202,  12, 0x0201, 4); /* Telecom Egypt Landline (Cairo) */
    packet_switch_add_route(ps, 203,  12, 0x0202, 5); /* Telecom Egypt Landline (Alexandria) */
    printf("[ROUTING] 5 active trunk routes installed.\n\n");
    
    printf("[SIM] Simulating 10,000 ISUP signaling call setups (IAM -> ACM -> ANM -> REL -> RLC)...\n");
    uint8_t iam_frame[4] = { ISUP_MSG_IAM, 0x00, 0x12, 0x34 };
    uint8_t acm_frame[4] = { ISUP_MSG_ACM, 0x00, 0x12, 0x34 };
    uint8_t anm_frame[4] = { ISUP_MSG_ANM, 0x00, 0x12, 0x34 };
    uint8_t rel_frame[4] = { ISUP_MSG_REL, 0x00, 0x12, 0x34 };
    uint8_t rlc_frame[4] = { ISUP_MSG_RLC, 0x00, 0x12, 0x34 };
    
    for (int i = 0; i < 2000; i++) {
        packet_switch_process_signaling_frame(ps, iam_frame, 4);
        packet_switch_process_signaling_frame(ps, acm_frame, 4);
        packet_switch_process_signaling_frame(ps, anm_frame, 4);
        packet_switch_process_signaling_frame(ps, rel_frame, 4);
        packet_switch_process_signaling_frame(ps, rlc_frame, 4);
    }
    
    printf("[CDR] Initializing binary CDR serialization engine...\n");
    cdr_pipeline_t *cdr_pipe = cdr_pipeline_create("audit/cdrs_2004.bin");
    binary_cdr_t test_cdr = {
        .record_sequence_id = 104291,
        .switch_id = 7,
        .circuit_id = 4612,
        .calling_number = "+201001234567",
        .called_number = "+20227940000",
        .setup_time_epoch_us = 1098124000000000ULL,
        .answer_time_epoch_us = 1098124003000000ULL,
        .release_time_epoch_us = 1098124125000000ULL,
        .call_duration_ms = 122000,
        .termination_cause = ISUP_CAUSE_NORMAL_CLEARING,
        .service_category = 1,
        .billed_units_egyptian_piastres = 150
    };
    
    uint8_t encoded_buf[sizeof(binary_cdr_t)];
    size_t encoded_len = 0;
    cdr_pipeline_encode_record(cdr_pipe, &test_cdr, encoded_buf, &encoded_len);
    
    binary_cdr_t decoded_cdr;
    bool valid = cdr_pipeline_decode_record(encoded_buf, encoded_len, &decoded_cdr);
    printf("[CDR] Binary record encoded (%zu bytes) - CRC32 Verification: %s\n", 
           encoded_len, valid ? "PASSED (CRC32 Matched)" : "FAILED");
    
    packet_switch_dump_metrics(ps);
    
    printf("[CLEANUP] Releasing hardware switches and memory pools...\n");
    cdr_pipeline_destroy(cdr_pipe);
    packet_switch_destroy(ps);
    memory_pool_destroy(pool);
    printf("[CLEANUP] Complete.\n");
    
    return 0;
}

/**
 * @file cdr_processor.h
 * @brief High-performance binary Call Detail Record (CDR) encoder, auditor, and billing serializer.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#ifndef TELECOM_CDR_PROCESSOR_H
#define TELECOM_CDR_PROCESSOR_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)
typedef struct binary_cdr {
    uint32_t record_sequence_id;
    uint32_t switch_id;
    uint32_t circuit_id;
    char calling_number[24];
    char called_number[24];
    uint64_t setup_time_epoch_us;
    uint64_t answer_time_epoch_us;
    uint64_t release_time_epoch_us;
    uint32_t call_duration_ms;
    uint8_t termination_cause;
    uint8_t service_category;      /* 1: Voice, 2: Fax, 3: Data, 4: Premium */
    uint32_t billed_units_egyptian_piastres;
    uint32_t crc32_checksum;
} binary_cdr_t;
#pragma pack(pop)

typedef struct cdr_pipeline {
    char audit_log_path[256];
    uint64_t total_cdrs_generated;
    uint64_t total_revenue_billed;
    uint64_t corrupted_records_detected;
} cdr_pipeline_t;

cdr_pipeline_t* cdr_pipeline_create(const char *log_path);
void cdr_pipeline_destroy(cdr_pipeline_t *pipe);

bool cdr_pipeline_encode_record(cdr_pipeline_t *pipe, const binary_cdr_t *cdr, uint8_t *output_buffer, size_t *out_len);
bool cdr_pipeline_decode_record(const uint8_t *buffer, size_t len, binary_cdr_t *dest);
uint32_t telecom_calculate_crc32(const uint8_t *data, size_t length);

#ifdef __cplusplus
}
#endif

#endif /* TELECOM_CDR_PROCESSOR_H */

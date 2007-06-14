/**
 * @file cdr_processor.c
 * @brief High-efficiency binary Call Detail Record encoder with CRC32 verification.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#include "telecom/cdr_processor.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* IEEE 802.3 CRC32 Lookup Table */
static uint32_t crc32_table[256];
static bool crc_table_initialized = false;

static void init_crc32_table(void) {
    uint32_t polynomial = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint8_t j = 8; j > 0; j--) {
            if (crc & 1) {
                crc = (crc >> 1) ^ polynomial;
            } else {
                crc >>= 1;
            }
        }
        crc32_table[i] = crc;
    }
    crc_table_initialized = true;
}

uint32_t telecom_calculate_crc32(const uint8_t *data, size_t length) {
    if (!crc_table_initialized) init_crc32_table();
    
    uint32_t crc = 0xFFFFFFFF;
    for (size_t i = 0; i < length; i++) {
        uint8_t table_idx = (crc ^ data[i]) & 0xFF;
        crc = (crc >> 8) ^ crc32_table[table_idx];
    }
    return crc ^ 0xFFFFFFFF;
}

cdr_pipeline_t* cdr_pipeline_create(const char *log_path) {
    cdr_pipeline_t *pipe = (cdr_pipeline_t*)malloc(sizeof(cdr_pipeline_t));
    if (!pipe) return NULL;
    
    memset(pipe, 0, sizeof(cdr_pipeline_t));
    if (log_path) {
        strncpy(pipe->audit_log_path, log_path, sizeof(pipe->audit_log_path) - 1);
    }
    
    return pipe;
}

void cdr_pipeline_destroy(cdr_pipeline_t *pipe) {
    if (pipe) free(pipe);
}

bool cdr_pipeline_encode_record(cdr_pipeline_t *pipe, const binary_cdr_t *cdr, uint8_t *output_buffer, size_t *out_len) {
    if (!cdr || !output_buffer || !out_len) return false;
    
    binary_cdr_t rec = *cdr;
    /* Compute CRC32 excluding the crc32 field itself */
    size_t data_len = sizeof(binary_cdr_t) - sizeof(uint32_t);
    rec.crc32_checksum = telecom_calculate_crc32((const uint8_t*)&rec, data_len);
    
    memcpy(output_buffer, &rec, sizeof(binary_cdr_t));
    *out_len = sizeof(binary_cdr_t);
    
    if (pipe) {
        pipe->total_cdrs_generated++;
        pipe->total_revenue_billed += rec.billed_units_egyptian_piastres;
    }
    
    return true;
}

bool cdr_pipeline_decode_record(const uint8_t *buffer, size_t len, binary_cdr_t *dest) {
    if (!buffer || len < sizeof(binary_cdr_t) || !dest) return false;
    
    memcpy(dest, buffer, sizeof(binary_cdr_t));
    
    size_t data_len = sizeof(binary_cdr_t) - sizeof(uint32_t);
    uint32_t calculated = telecom_calculate_crc32(buffer, data_len);
    
    return (calculated == dest->crc32_checksum);
}

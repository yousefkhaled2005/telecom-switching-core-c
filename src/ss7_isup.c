/**
 * @file ss7_isup.c
 * @brief Implementation of ITU-T Q.763 ISUP Signaling State Machine.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#include "telecom/ss7_isup.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#if defined(_WIN32)
#include <windows.h>
static uint64_t get_current_epoch_us(void) {
    FILETIME ft;
    GetSystemTimeAsFileTime(&ft);
    uint64_t t = ((uint64_t)ft.dwHighDateTime << 32) | ft.dwLowDateTime;
    return (t - 116444736000000000ULL) / 10;
}
#else
#include <sys/time.h>
static uint64_t get_current_epoch_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (uint64_t)tv.tv_sec * 1000000ULL + tv.tv_usec;
}
#endif

isup_circuit_t* isup_circuit_create(uint32_t cic, uint32_t opc, uint32_t dpc) {
    isup_circuit_t *c = (isup_circuit_t*)malloc(sizeof(isup_circuit_t));
    if (!c) return NULL;
    
    memset(c, 0, sizeof(isup_circuit_t));
    c->cic = cic;
    c->opc = opc;
    c->dpc = dpc;
    c->state = ISUP_CIRCUIT_IDLE;
    c->timer_t1 = 15; /* ITU-T standard 15s */
    c->timer_t9 = 30; /* ITU-T standard 30s */
    
    return c;
}

void isup_circuit_destroy(isup_circuit_t *circuit) {
    if (circuit) free(circuit);
}

bool isup_handle_iam(isup_circuit_t *c, const char *calling, const char *called) {
    if (!c || c->state != ISUP_CIRCUIT_IDLE) {
        return false;
    }
    
    strncpy(c->calling_party, calling, sizeof(c->calling_party) - 1);
    strncpy(c->called_party, called, sizeof(c->called_party) - 1);
    c->call_start_timestamp_us = get_current_epoch_us();
    c->state = ISUP_CIRCUIT_WAIT_ACM;
    
    return true;
}

bool isup_handle_acm(isup_circuit_t *c) {
    if (!c || c->state != ISUP_CIRCUIT_WAIT_ACM) {
        return false;
    }
    
    c->state = ISUP_CIRCUIT_WAIT_ANM;
    return true;
}

bool isup_handle_anm(isup_circuit_t *c) {
    if (!c || c->state != ISUP_CIRCUIT_WAIT_ANM) {
        return false;
    }
    
    c->call_answer_timestamp_us = get_current_epoch_us();
    c->state = ISUP_CIRCUIT_ACTIVE;
    return true;
}

bool isup_handle_rel(isup_circuit_t *c, uint8_t cause) {
    if (!c || c->state == ISUP_CIRCUIT_IDLE || c->state == ISUP_CIRCUIT_WAIT_RLC) {
        return false;
    }
    
    c->release_cause = cause;
    c->call_end_timestamp_us = get_current_epoch_us();
    c->state = ISUP_CIRCUIT_WAIT_RLC;
    return true;
}

bool isup_handle_rlc(isup_circuit_t *c) {
    if (!c || c->state != ISUP_CIRCUIT_WAIT_RLC) {
        return false;
    }
    
    c->state = ISUP_CIRCUIT_IDLE;
    return true;
}

const char* isup_state_to_str(isup_circuit_state_t state) {
    switch (state) {
        case ISUP_CIRCUIT_IDLE:     return "IDLE";
        case ISUP_CIRCUIT_WAIT_ACM: return "WAIT_ACM";
        case ISUP_CIRCUIT_WAIT_ANM: return "WAIT_ANM";
        case ISUP_CIRCUIT_ACTIVE:   return "ACTIVE_CALL";
        case ISUP_CIRCUIT_WAIT_RLC: return "WAIT_RLC";
        case ISUP_CIRCUIT_BLOCKED:  return "BLOCKED";
        case ISUP_CIRCUIT_FAULT:    return "CIRCUIT_FAULT";
        default:                    return "UNKNOWN";
    }
}

const char* isup_msg_type_to_str(isup_message_type_t type) {
    switch (type) {
        case ISUP_MSG_IAM: return "IAM (Initial Address Message)";
        case ISUP_MSG_SAM: return "SAM (Subsequent Address Message)";
        case ISUP_MSG_ACM: return "ACM (Address Complete Message)";
        case ISUP_MSG_ANM: return "ANM (Answer Message)";
        case ISUP_MSG_REL: return "REL (Release)";
        case ISUP_MSG_RLC: return "RLC (Release Complete)";
        case ISUP_MSG_CPG: return "CPG (Call Progress)";
        case ISUP_MSG_RSC: return "RSC (Reset Circuit)";
        case ISUP_MSG_GRA: return "GRA (Circuit Group Reset Ack)";
        default:           return "UNKNOWN_MSG";
    }
}

/**
 * @file ss7_isup.h
 * @brief ITU-T Q.763 ISDN User Part (ISUP) Signaling State Machine Specification.
 * @author Yousef Khaled (Telecom Systems Engineering - 2004)
 */

#ifndef TELECOM_SS7_ISUP_H
#define TELECOM_SS7_ISUP_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ISUP Message Types (ITU-T Q.763) */
typedef enum {
    ISUP_MSG_IAM = 0x01,  /* Initial Address Message */
    ISUP_MSG_SAM = 0x02,  /* Subsequent Address Message */
    ISUP_MSG_ACM = 0x06,  /* Address Complete Message */
    ISUP_MSG_ANM = 0x09,  /* Answer Message */
    ISUP_MSG_REL = 0x0C,  /* Release */
    ISUP_MSG_RLC = 0x10,  /* Release Complete */
    ISUP_MSG_CPG = 0x2C,  /* Call Progress */
    ISUP_MSG_RSC = 0x12,  /* Reset Circuit */
    ISUP_MSG_GRA = 0x29   /* Circuit Group Reset Acknowledgment */
} isup_message_type_t;

/* Circuit Supervision States */
typedef enum {
    ISUP_CIRCUIT_IDLE = 0,
    ISUP_CIRCUIT_WAIT_ACM,
    ISUP_CIRCUIT_WAIT_ANM,
    ISUP_CIRCUIT_ACTIVE,
    ISUP_CIRCUIT_WAIT_RLC,
    ISUP_CIRCUIT_BLOCKED,
    ISUP_CIRCUIT_FAULT
} isup_circuit_state_t;

/* ISUP Cause Codes (ITU-T Q.850) */
typedef enum {
    ISUP_CAUSE_NORMAL_CLEARING       = 16,
    ISUP_CAUSE_USER_BUSY             = 17,
    ISUP_CAUSE_NO_USER_RESPONDING    = 18,
    ISUP_CAUSE_CALL_REJECTED         = 21,
    ISUP_CAUSE_CIRCUIT_UNAVAILABLE   = 34,
    ISUP_CAUSE_NETWORK_OUT_OF_ORDER  = 38,
    ISUP_CAUSE_RECOVERY_ON_TIMER     = 102
} isup_cause_code_t;

/* ISUP Circuit Control Block (CCB) */
typedef struct isup_circuit {
    uint32_t cic;                      /* Circuit Identification Code */
    uint32_t opc;                      /* Originating Point Code */
    uint32_t dpc;                      /* Destination Point Code */
    isup_circuit_state_t state;
    char calling_party[32];            /* E.164 Number */
    char called_party[32];             /* E.164 Number */
    uint64_t call_start_timestamp_us;
    uint64_t call_answer_timestamp_us;
    uint64_t call_end_timestamp_us;
    uint8_t release_cause;
    uint32_t timer_t1;                 /* Awaiting ACM timer (s) */
    uint32_t timer_t9;                 /* Awaiting ANM timer (s) */
} isup_circuit_t;

/* Public API */
isup_circuit_t* isup_circuit_create(uint32_t cic, uint32_t opc, uint32_t dpc);
void isup_circuit_destroy(isup_circuit_t *circuit);

bool isup_handle_iam(isup_circuit_t *c, const char *calling, const char *called);
bool isup_handle_acm(isup_circuit_t *c);
bool isup_handle_anm(isup_circuit_t *c);
bool isup_handle_rel(isup_circuit_t *c, uint8_t cause);
bool isup_handle_rlc(isup_circuit_t *c);

const char* isup_state_to_str(isup_circuit_state_t state);
const char* isup_msg_type_to_str(isup_message_type_t type);

#ifdef __cplusplus
}
#endif

#endif /* TELECOM_SS7_ISUP_H */

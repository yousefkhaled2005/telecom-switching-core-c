# ==============================================================================
# @file atomic_ops_x86.s
# @brief High-performance lock-free assembly primitives for x86 / x86_64
# @author Yousef Khaled (Telecom Systems Engineering - 2004)
# ==============================================================================

.intel_syntax noprefix
.text

.globl telecom_atomic_cas_32
.globl telecom_atomic_cas_64
.globl telecom_atomic_add_64
.globl telecom_read_tsc
.globl telecom_memory_barrier

# uint32_t telecom_atomic_cas_32(uint32_t *target, uint32_t expected, uint32_t desired)
# Windows x64: RCX = target, EDX = expected, R8D = desired
# Linux x86_64: RDI = target, ESI = expected, EDX = desired
telecom_atomic_cas_32:
#if defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW64__)
    mov eax, edx
    lock cmpxchg [rcx], r8d
    ret
#else
    mov eax, esi
    lock cmpxchg [rdi], edx
    ret
#endif

# uint64_t telecom_atomic_add_64(uint64_t *target, uint64_t value)
telecom_atomic_add_64:
#if defined(_WIN64) || defined(__CYGWIN__) || defined(__MINGW64__)
    mov rax, rdx
    lock xadd [rcx], rax
    add rax, rdx
    ret
#else
    mov rax, rsi
    lock xadd [rdi], rax
    add rax, rsi
    ret
#endif

# uint64_t telecom_read_tsc(void)
# Reads the Time Stamp Counter (clock cycles elapsed since reset)
telecom_read_tsc:
    rdtsc
    shl rdx, 32
    or rax, rdx
    ret

# void telecom_memory_barrier(void)
# Issues full serializing CPU memory fence
telecom_memory_barrier:
    mfence
    ret

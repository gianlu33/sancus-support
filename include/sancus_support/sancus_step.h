#ifndef SANCUS_STEP_H
#define SANCUS_STEP_H
#include <msp430.h>
#include "timer.h"
#include <stdint.h>
#include <sancus/sm_support.h>


/* ======== SANCUS STEP DBG CONSTANTS ======== */
#define RETI_LENGTH (0x5)
#define JMP_LENGTH (0x2)

#define EXTRA_DELAY (0x2)
#define ENTRY_DELAY (0x3)

#define INIT_LATENCY 1

void __ss_print_latency(void);
int __ss_get_latency(void);

// sancus step interface
void __ss_start(void);
void __ss_init(void);
void __ss_end(void);

// sancus step configuration parameters
int __ss_dbg_entry_delay;
int __ss_dbg_measuring_reti_latency;
int __ss_isr_reti_latency;
int __ss_sm_exit_latency;
int __ss_isr_interrupted_sm;

extern struct SancusModule ssdbg;
int SM_ENTRY(ssdbg) __ss_dbg_get_info(void);

volatile int __ss_isr_tar_entry;

#define SANCUS_STEP_ISR_ENTRY2(fct_single_step, fct_end)                       \
__attribute__((naked)) __attribute__((interrupt(TIMER_IRQ_VECTOR2)))           \
void timerA_isr_entry2(void)                                                   \
{                                                                              \
    __asm__(                                                                   \
        /* copy TAR value to __ss_isr_tar_entry */                             \
        "mov &%0, &%2\n\t"                                                     \
        /* disable timer (copy TACTL_DISABLE to TACTL) */                      \
        "mov %9, &%4\n\t"                                                      \
        /* subtract TACCR0 from __ss_isr_tar_entry */                          \
        "sub &%5, &%2\n\t"                                                     \
        /* __ss_isr_interrupted_sm = false */                                  \
        "mov #0x0, &%1\n\t"                                                    \
        /* check whether r1 register is set (0 if module got interrupted?) */  \
        "cmp #0x0, r1\n\t"                                                     \
        /* if module was not interrupted, jump to label "1" */                 \
        "jne 1f\n\t"                                                           \
        /* __ss_isr_interrupted_sm = true */                                   \
        "mov #0x1, &%1\n\t"                                                    \
        /* r1 = __isr_sp */                                                    \
        "mov &%3, r1\n\t"                                                      \
        /*  */                                                                 \
        "push r15\n\t"                                                         \
        /*  */                                                                 \
        "push #0x0\n\t"                                                        \
        /* check whether __ss_dbg_measuring_reti_latency is set */             \
        "cmp #0x0, &%7\n\t"                                                    \
        /* if not, jump to label "2" */                                        \
        "jz 2f\n\t"                                                            \
        /* TACTL = TACTL_CONTINUOUS */                                         \
        "mov %8, &%4\n\t"                                                      \
        /* jump to label "3" */                                                \
        "jmp 3f\n\t"                                                           \
        /* label 2: not measuring __ss_isr_reti_latency */                     \
        "2: \n\t"                                                              \
        /* call the first function parameter */                                \
        "call #" #fct_single_step "\n\t"                                       \
        /*  */                                                                 \
        "push r15\n\t"                                                         \
        /* r15 = __ss_isr_reti_latency */                                      \
        "mov &%6, r15\n\t"                                                     \
        /* r15 += 6 */                                                         \
        "add #0x6, r15 ;\n\t"                                                  \
        /* TACCR0 = r15 */                                                     \
        "mov r15, &%5\n\t"                                                     \
        /*  */                                                                 \
        "pop r15\n\t"                                                          \
        /* TACCTL0 = TACCTL_ENABLE_CONT */                                     \
        "mov %11, &%12\n\t"                                                    \
        /* TACTL = TACTL_ENABLE_CONT */                                        \
        "mov %13, &%4\n\t"                                                     \
        /* jump to return instruction */                                       \
        "jmp 3f\n\t"                                                           \
        /* label 1: module was not interrupted */                              \
        "1: \n\t"                                                              \
        /* TACTL = TACTL_DISABLE */                                            \
        "mov %9, &%4; disable timer\n\t"                                       \
        /* call the second function parameter */                               \
        "call #" #fct_end "\n\t"                                               \
        /* return from the ISR */                                              \
        "3: reti\n\t"                                                          \
        :                                                                      \
        :                                                                      \
        "m"(TAR),                                                    /* %0 */  \
        "m"(__ss_isr_interrupted_sm),                                /* %1 */  \
        "m"(__ss_isr_tar_entry),                                     /* %2 */  \
        "m"(__isr_sp),                                               /* %3 */  \
        "m"(TACTL),                                                  /* %4 */  \
        "m"(TACCR0),                                                 /* %5 */  \
        "m"(__ss_isr_reti_latency),                                  /* %6 */  \
        "m"(__ss_dbg_measuring_reti_latency),                        /* %7 */  \
        "i"(TACTL_CONTINUOUS),                                       /* %8 */  \
        "i"(TACTL_DISABLE),                                          /* %9 */  \
        "i"(TACTL_ENABLE),                                           /* %10 */ \
        "i"(TACCTL_ENABLE_CONT),                                     /* %11 */ \
        "m"(TACCTL0),                                                /* %12 */ \
        "i"(TACTL_ENABLE_CONT)                                       /* %13 */ \
        :                                                                      \
    );                                                                         \
}

#endif

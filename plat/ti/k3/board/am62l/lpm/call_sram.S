/*
 * Copyright (c) 2024-2025, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
	.section .wkupsram, "ax"

#include <arch.h>
#include <asm_macros.S>
#include <board_def.h>

.global k3_lpm_switch_stack
.global k3_lpm_resume
.global k3_lpm_resume_c
.globl	plat_invalidate_icache

/*
 * x0 : jump address,
 * x1 : stack address,
 * x2 : arg,
 * x3 : stack address (temporary)
 */
func k3_lpm_switch_stack

	/* lr to stack */

	/* change stack pointer */
	mov	x3, sp
	mov	sp, x1

	/* save stack pointer */
	sub	sp, sp, #16
	stp	x0, x3, [sp]

	/* data synchronization barrier */
	dsb	sy

	/* jump to code */
	mov	x1, x0
	mov	x0, x2
	blr	x1

	/* load stack pointer */
	ldp 	x0, x2, [sp,#0]

	/* change stack pointer */
	mov	sp, x2

	/* return */
	ldp	x29, x30, [sp,#-16]
	ret
endfunc k3_lpm_switch_stack

.section  ".wkupsram.text", "ax"
    .global CacheP_wbInvAll
    .func CacheP_wbInvAll
CacheP_wbInvAll:
    mrs     x0, clidr_el1
    and     w3, w0, #0x07000000     /* Get 2 x Level of Coherence */
    lsr     w3, w3, #23
    cbz     w3, 5f
    mov     w10, #0                 /* w10 = 2 x cache level */
    mov     w8, #1                  /* w8 = Constant 0b1 */
1:
    add     w2, w10, w10, lsr #1    /* Caclulate 3x cache level */
    lsr     w1, w0, w2              /* Extract cache type for this level */
    and     w1, w1, #0x7
    cmp     w1, #2
    blt     4f                      /* No data or unified cache */
    msr     csselr_el1, x10         /* Select this cache level */
    isb                             /* Synchronize change of csselr */
    mrs     x1, ccsidr_el1          /* Read ccsidr */
    and     w2, w1, #7              /* w2 = log2(linelen)-4 */
    add     w2, w2, #4              /* w2 = log2(linelen) */
    ubfx    w4, w1, #3, #10         /* w4 = max way number, right aligned */
    clz     w5, w4                  /* w5 = 32-log2(ways), bit position of
                                        way in dc operand */
    lsl     w9, w4, w5              /* w9 = max way number, aligned to
                                        position in DC operand */
    lsl     w16, w8, w5             /* w16 = amount to decrement way number
                                        per iteration */
2:
    ubfx    w7, w1, #13, #15        /* w7 = max set number, right aligned */
    lsl     w7, w7, w2              /* w7 = max set number, aligned to
                                        position in DC operand */
    lsl     w17, w8, w2             /* w17 = amount to decrement set number
                                        per iteration */
3:
    orr     w11, w10, w9            /* w11 = combine way num & cache ...*/
    orr     w11, w11, w7            /* ... num and set num for DC operand */
    dc      cisw, x11               /* Do data cache clean and invalidate
                                        by set and way */
    subs    w7, w7, w17             /* Decrement set number */
    bge     3b
    subs    x9, x9, x16             /* Decrement way number */
    bge     2b
4:
    add     w10, w10, #2            /* Increment 2 x cache level */
    cmp     w3, w10
    dsb     sy                      /* Ensure completion of previous cache
                                        maintenance operation */
    bgt     1b
5:
    ret
    .endfunc

    .func CacheP_disableEL3
CacheP_disableEL3:
    mrs     x0, sctlr_el3            /* read SCTLR_EL3 */
    bic     x0, x0, #0x0004          /* clear C bit */
    msr     sctlr_el3, x0            /* DCache disabled */

    mrs     x0, sctlr_el3            /* read SCTLR_EL3 */
    bic     x0, x0, #0x1000          /* clear I bit */
    msr     sctlr_el3, x0            /* ICache disabled */
    ic      iallu                    /* invalidate all ICache */
    dsb     sy
    isb
    ret
    .endfunc

#define HWIP_GIC_BASE_ADDR      (0x1800000U)
#define HWIP_GICR_BASE_ADDR      (0x1880000U)
#define HWIP_GICS_BASE_ADDR      (0x1890000U)
#define HWIP_GICX_OFFSET    0x0020000


.macro GICD_WRITE_LOOP x, y, offset
    str     w1, [x0, #\offset]
    .if \y-\x
    GICD_WRITE_LOOP "(\x+1)", \y, "(\offset+4)"
    .endif
.endm

.macro INIT_GICD_IGROUPR
    ldr     x0, =HWIP_GIC_BASE_ADDR
    mvn     w1, wzr
    GICD_WRITE_LOOP 0, 31, 0x0080
.endm

.macro INIT_GICD_IGRPMODR
    ldr     x0, =HWIP_GIC_BASE_ADDR
    mov     w1, wzr
    GICD_WRITE_LOOP 0, 31, 0x0D00
.endm


    .func gnu_targets_arm_rtsv8A_startupAsm
gnu_targets_arm_rtsv8A_startupAsm:
    /*
     * ---------------------
     * Boot code starts here
     * ---------------------
     */

    mov     x0, #0x3C0
    msr     daif, x0            /* Mask all interrupts */
    isb                         /* Synchronize processor context */

    /*
     * ------------------------
     * Initialize stack pointer
     * ------------------------
     */
    msr     spsel, #1           /* Use SP_ELx for ELx */
    movz x0, #(DEVICE_WKUP_SRAM_STACK_BASE_L)
	movk x0, #(DEVICE_WKUP_SRAM_STACK_BASE_H), lsl #16
	mov	sp, x0

    /* Setup the vector table for all levels */
    ldr     x0, =lpm_HwiP_gicv3Vectors
    msr     vbar_el1, x0         /* Set vector table base address */

    /* do more initialization in C, go to main() */
	bl k3_lpm_resume_c

    .endfunc

.macro lpm_vector_entry name
        .align  7
\name:
.endm

/* ---------------------------------------------
 * while one handler
 * ---------------------------------------------
 */
    .func defaultExcHandler
defaultExcHandler:
	b	defaultExcHandler
    .endfunc


    .func HwiP_armv8GetGicxAddr
HwiP_armv8GetGicxAddr:
    ldr     x1, =HWIP_GICX_OFFSET
    mrs     x2, mpidr_el1
    ubfx    x3, x2, #8, #8      /* x3 = Cluster Id */
    and     x2, x2, #0xFF       /* x2 = Core Id */
    sub     x3, x3, #0
    mrs     x4, s3_1_c11_c0_2   /* Read L2CTLR_EL1 */
    ubfx    x4, x4, #24, #2     /* x4 = Number of cores per cluster */
    lsl     x3, x3, x4          /* x3 = clusterIdx * numCoresPerCluster */
    add     x2, x2, x3
    madd    x0, x1, x2, x0
    ret
    .endfunc

    .global lpm_HwiP_gicv3Vectors
    .align  11
lpm_HwiP_gicv3Vectors:

lpm_vector_entry el1SyncSP0
    B defaultExcHandler


lpm_vector_entry el1IrqSP0
    B defaultExcHandler



lpm_vector_entry el1FiqSP0
    b       defaultExcHandler



lpm_vector_entry el1SErrorSP0
    b       defaultExcHandler


/*
 *************************************************************************
 * Exception from currentEL, using SPx
 *************************************************************************
 */
lpm_vector_entry el1SyncSPx
    b       defaultExcHandler



lpm_vector_entry el1IrqSPx
    b       defaultExcHandler



lpm_vector_entry el1FiqSPx
    b       defaultExcHandler



lpm_vector_entry el1SErrorSPx
    b       defaultExcHandler



/*
 *************************************************************************
 * Exception from lowerEL, all lowerEL using Aarch64
 *************************************************************************
 */
lpm_vector_entry el0SyncAarch64
    b       defaultExcHandler



lpm_vector_entry el0IrqAarch64
    b       defaultExcHandler



lpm_vector_entry el0FiqAarch64
    b       defaultExcHandler


lpm_vector_entry el0SErrorAarch64
    b       defaultExcHandler


/*
 *************************************************************************
 * Exception from lowerEL, all lowerEL using Aarch32
 *************************************************************************
 */
lpm_vector_entry el0SyncAarch32
    b       defaultExcHandler


lpm_vector_entry el0IrqAarch32
    b       defaultExcHandler


lpm_vector_entry el0FiqAarch32
    b       defaultExcHandler


lpm_vector_entry el0SErrorAarch32
    b       defaultExcHandler



func k3_lpm_resume


    bl     CacheP_wbInvAll      /* Invalidate all existing caches  */
    bl     CacheP_disableEL3    /* Disabling Instruction and data cache for EL3 level */

    movz x0, #(DEVICE_WKUP_SRAM_STACK_BASE_L)
	movk x0, #(DEVICE_WKUP_SRAM_STACK_BASE_H), lsl #16
	mov	sp, x0
	bl k3_lpm_resume_c
	ret
endfunc k3_lpm_resume



/* ---------------------------------------------
 * void plat_invalidate_icache(void)
 * Instruction Cache Invalidate All to PoU
 * ---------------------------------------------
 */
func plat_invalidate_icache
	ic	iallu

	ret
endfunc plat_invalidate_icache

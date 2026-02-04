/*
 * Copyright (c) 2024-2026, Texas Instruments Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
	.section .wkupsram, "ax"

#include <arch.h>
#include <asm_macros.S>
#include <platform_def.h>

.global k3low_lpm_switch_stack
.global k3low_lpm_resume
.global k3low_lpm_resume_c
.globl	plat_invalidate_icache

/*
 * x0 : jump address,
 * x1 : stack address,
 * x2 : arg,
 * x3 : stack address (temporary)
 */
func k3low_lpm_switch_stack

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
endfunc k3low_lpm_switch_stack

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

func k3low_lpm_resume
    bl     CacheP_wbInvAll      /* Invalidate all existing caches  */
    bl     CacheP_disableEL3    /* Disabling Instruction and data cache for EL3 level */

    movz x0, #(DEVICE_WKUP_SRAM_STACK_BASE_L)
	movk x0, #(DEVICE_WKUP_SRAM_STACK_BASE_H), lsl #16
	mov	sp, x0
	bl k3low_lpm_resume_c
	ret
endfunc k3low_lpm_resume


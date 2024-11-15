/*
 * Copyright (c) 2025, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <lib/extensions/sysreg128.h>

        .global read_par_el1
        .global write_par_el1
        .global read_ttbr0_el1
        .global write_ttbr0_el1
        .global read_ttbr1_el1
        .global write_ttbr1_el1
        .global read_ttbr0_el2
        .global write_ttbr0_el2
        .global read_ttbr1_el2
        .global write_ttbr1_el2
        .global read_vttbr_el2
        .global write_vttbr_el2
        .global read_rcwmask_el1
        .global write_rcwmask_el1
        .global read_rcwsmask_el1
        .global write_rcwsmask_el1

/*
 * _mrrs - Move System register to two adjacent general-purpose
 * registers.
 * Instruction: MRRS <Xt>, <Xt+1>, (<systemreg>|S<op0>_<op1>_<Cn>_<Cm>_<op2>)
 *
 * Arguments/Opcode bit field:
 * regins: System register opcode.
 *
 * Clobbers: x0,x1,x2
 */
.macro _mrrs regins:req
#if ENABLE_FEAT_D128 == 2
        is_feat_sysreg128_present_asm x0
        bne     1f
        /* If FEAT_SYSREG128 is not implemented then use mrs */
        .inst   0xD5300000 | (\regins)  /* mrs  x0, \regins */
        ret
#endif
1:
        .inst   0xD5700000 | (\regins)  /* mrrs x0, x1, \regins */
        ret
.endm

/*
 * _msrr - Move two adjacent general-purpose registers to System register.
 * Instruction: MSRR (<systemreg>|S<op0>_<op1>_<Cn>_<Cm>_<op2>), <Xt>, <Xt+1>
 *
 * Arguments/Opcode bit field:
 * regins: System register opcode.
 *
 * Clobbers: x0,x1,x2
 */
.macro _msrr regins:req
#if ENABLE_FEAT_D128 == 2
        /* Don't tamper x0 and x1 as they may be used for msrr */
        is_feat_sysreg128_present_asm x2
        bne     1f
        /* If FEAT_SYSREG128 is not implemented then use msr */
        .inst   0xD5100000 | (\regins)  /* msr  \regins, x0 */
        ret
#endif
1:
        .inst   0xD5500000 | (\regins)  /* msrr \regins, x0, x1 */
        ret
.endm

func read_par_el1
        _mrrs   0x87400 /* S3_0_C7_C4_0 */
endfunc read_par_el1

func write_par_el1
        _msrr   0x87400
endfunc write_par_el1

func read_ttbr0_el1
        _mrrs   0x82000 /* S3_0_C2_C0_0 */
endfunc read_ttbr0_el1

func write_ttbr0_el1
        _msrr 0x82000
endfunc write_ttbr0_el1

func read_ttbr1_el1
        _mrrs 0x82020 /* S3_0_C2_C0_1 */
endfunc read_ttbr1_el1

func write_ttbr1_el1
        _msrr 0x82020
endfunc write_ttbr1_el1

func read_ttbr0_el2
        _mrrs 0xC2000 /* S3_4_C2_C0_0 */
endfunc read_ttbr0_el2

func write_ttbr0_el2
        _msrr 0xC2000
endfunc write_ttbr0_el2

func read_ttbr1_el2
        _mrrs 0xC2020 /* S3_4_C2_C0_1 */
endfunc read_ttbr1_el2

func write_ttbr1_el2
        _msrr 0xC2020
endfunc write_ttbr1_el2

func read_vttbr_el2
        _mrrs 0xC2100 /* S3_4_C2_C1_0 */
endfunc read_vttbr_el2

func write_vttbr_el2
        _msrr 0xC2100
endfunc write_vttbr_el2

func read_rcwmask_el1
        _mrrs 0x8D0C0 /* S3_0_C13_C0_6 */
endfunc read_rcwmask_el1

func write_rcwmask_el1
        _msrr 0x8D0C0
endfunc write_rcwmask_el1

func read_rcwsmask_el1
        _mrrs 0x8D060 /* S3_0_C13_C0_3 */
endfunc read_rcwsmask_el1

func write_rcwsmask_el1
        _msrr 0x8D060
endfunc write_rcwsmask_el1

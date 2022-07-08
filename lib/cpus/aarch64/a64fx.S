/*
 * Copyright (c) 2022, Fujitsu Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <a64fx.h>
#include <cpu_macros.S>
#include <plat_macros.S>

func a64fx_core_pwr_dwn
endfunc a64fx_core_pwr_dwn

func a64fx_cluster_pwr_dwn
endfunc a64fx_cluster_pwr_dwn

#if REPORT_ERRATA
/*
 * Errata printing function for A64FX. Must follow AAPCS.
 */
func a64fx_errata_report
        ret
endfunc a64fx_errata_report
#endif

        /* ---------------------------------------------
         * This function provides cpu specific
         * register information for crash reporting.
         * It needs to return with x6 pointing to
         * a list of register names in ascii and
         * x8 - x15 having values of registers to be
         * reported.
         * ---------------------------------------------
         */
.section .rodata.a64fx_regs, "aS"
a64fx_regs:  /* The ascii list of register names to be reported */
        .asciz  ""

func a64fx_cpu_reg_dump
        adr     x6, a64fx_regs
        ret
endfunc a64fx_cpu_reg_dump

declare_cpu_ops a64fx, A64FX_MIDR, CPU_NO_RESET_FUNC \
                a64fx_core_pwr_dwn, \
                a64fx_cluster_pwr_dwn


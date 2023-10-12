/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <asm_macros.S>
#include <common/bl_common.h>
#include <cortex_a510.h>
#include <cpu_macros.S>
#include <plat_macros.S>

/* Hardware handled coherency */
#if HW_ASSISTED_COHERENCY == 0
#error "Cortex-A510 must be compiled with HW_ASSISTED_COHERENCY enabled"
#endif

/* 64-bit only core */
#if CTX_INCLUDE_AARCH32_REGS == 1
#error "Cortex-A510 supports only AArch64. Compile with CTX_INCLUDE_AARCH32_REGS=0"
#endif

workaround_reset_start cortex_a510, ERRATUM(1922240), ERRATA_A510_1922240
	/* Apply the workaround by setting IMP_CMPXACTLR_EL1[11:10] = 0b11. */
	sysreg_bitfield_insert CORTEX_A510_CMPXACTLR_EL1, CORTEX_A510_CMPXACTLR_EL1_SNPPREFERUNIQUE_DISABLE, \
	CORTEX_A510_CMPXACTLR_EL1_SNPPREFERUNIQUE_SHIFT, CORTEX_A510_CMPXACTLR_EL1_SNPPREFERUNIQUE_WIDTH
workaround_reset_end cortex_a510, ERRATUM(1922240)

check_erratum_ls cortex_a510, ERRATUM(1922240), CPU_REV(0, 0)

workaround_reset_start cortex_a510, ERRATUM(2041909), ERRATA_A510_2041909
	/* Apply workaround */
	mov	x0, xzr
	msr	S3_6_C15_C4_0, x0
	isb

	mov	x0, #0x8500000
	msr	S3_6_C15_C4_2, x0

	mov	x0, #0x1F700000
	movk	x0, #0x8, lsl #32
	msr	S3_6_C15_C4_3, x0

	mov	x0, #0x3F1
	movk	x0, #0x110, lsl #16
	msr	S3_6_C15_C4_1, x0
workaround_reset_end cortex_a510, ERRATUM(2041909)

check_erratum_range cortex_a510, ERRATUM(2041909), CPU_REV(0, 2), CPU_REV(0, 2)

workaround_reset_start cortex_a510, ERRATUM(2042739), ERRATA_A510_2042739
	/* Apply the workaround by disabling ReadPreferUnique. */
	sysreg_bitfield_insert CORTEX_A510_CPUECTLR_EL1, CORTEX_A510_CPUECTLR_EL1_READPREFERUNIQUE_DISABLE, \
		CORTEX_A510_CPUECTLR_EL1_READPREFERUNIQUE_SHIFT, CORTEX_A510_CPUECTLR_EL1_READPREFERUNIQUE_WIDTH
workaround_reset_end cortex_a510, ERRATUM(2042739)

check_erratum_ls cortex_a510, ERRATUM(2042739), CPU_REV(0, 2)

workaround_reset_start cortex_a510, ERRATUM(2080326), ERRATA_A510_2080326
	/* Apply workaround */
	mov x0, #1
	msr S3_6_C15_C4_0, x0
	isb

	mov x0, #0x0100
	movk x0, #0x0E08, lsl #16
	msr S3_6_C15_C4_2, x0

	mov x0, #0x0300
	movk x0, #0x0F1F, lsl #16
	movk x0, #0x0008, lsl #32
	msr S3_6_C15_C4_3, x0

	mov x0, #0x03F1
	movk x0, #0x00C0, lsl #16
	msr S3_6_C15_C4_1, x0

	isb
workaround_reset_end cortex_a510, ERRATUM(2080326)

check_erratum_range cortex_a510, ERRATUM(2080326), CPU_REV(0, 2), CPU_REV(0, 2)

workaround_reset_start cortex_a510, ERRATUM(2172148), ERRATA_A510_2172148
	/*
	 * Force L2 allocation of transient lines by setting
	 * CPUECTLR_EL1.RSCTL=0b01 and CPUECTLR_EL1.NTCTL=0b01.
	 */
	mrs	x0, CORTEX_A510_CPUECTLR_EL1
	mov	x1, #1
	bfi	x0, x1, #CORTEX_A510_CPUECTLR_EL1_RSCTL_SHIFT, #2
	bfi	x0, x1, #CORTEX_A510_CPUECTLR_EL1_NTCTL_SHIFT, #2
	msr	CORTEX_A510_CPUECTLR_EL1, x0
workaround_reset_end cortex_a510, ERRATUM(2172148)

check_erratum_ls cortex_a510, ERRATUM(2172148), CPU_REV(1, 0)

workaround_reset_start cortex_a510, ERRATUM(2218950), ERRATA_A510_2218950
	/* Set bit 18 in CPUACTLR_EL1 */
	sysreg_bitfield_insert CORTEX_A510_CPUACTLR_EL1, CORTEX_A510_CPUACTLR_EL1_ALIAS_LOADSTORE_DISABLE, \
	CORTEX_A510_CPUACTLR_EL1_ALIAS_LOADSTORE_SHIFT, CORTEX_A510_CPUACTLR_EL1_ALIAS_LOADSTORE_WIDTH

	/* Set bit 25 in CMPXACTLR_EL1 */
	sysreg_bitfield_insert CORTEX_A510_CMPXACTLR_EL1, CORTEX_A510_CMPXACTLR_EL1_ALIAS_LOADSTORE_DISABLE, \
	CORTEX_A510_CMPXACTLR_EL1_ALIAS_LOADSTORE_SHIFT, CORTEX_A510_CMPXACTLR_EL1_ALIAS_LOADSTORE_WIDTH

workaround_reset_end cortex_a510, ERRATUM(2218950)

check_erratum_ls cortex_a510, ERRATUM(2218950), CPU_REV(1, 0)

	/* --------------------------------------------------
	 * This workaround is not a typical errata fix. MPMM
	 * is disabled here, but this conflicts with the BL31
	 * MPMM support. So in addition to simply disabling
	 * the feature, a flag is set in the MPMM library
	 * indicating that it should not be enabled even if
	 * ENABLE_MPMM=1.
	 * --------------------------------------------------
	 */
workaround_reset_start cortex_a510, ERRATUM(2250311), ERRATA_A510_2250311
	/* Disable MPMM */
	mrs	x0, CPUMPMMCR_EL3
	bfm	x0, xzr, #0, #0 /* bfc instruction does not work in GCC */
	msr	CPUMPMMCR_EL3, x0

#if ENABLE_MPMM && IMAGE_BL31
	/* If ENABLE_MPMM is set, tell the runtime lib to skip enabling it. */
	bl mpmm_errata_disable
#endif
workaround_reset_end cortex_a510, ERRATUM(2250311)

check_erratum_ls cortex_a510, ERRATUM(2250311), CPU_REV(1, 0)

workaround_reset_start cortex_a510, ERRATUM(2288014), ERRATA_A510_2288014
	/* Apply the workaround by setting IMP_CPUACTLR_EL1[18] = 0b1. */
	sysreg_bitfield_insert CORTEX_A510_CPUACTLR_EL1, CORTEX_A510_CPUACTLR_EL1_DATA_CORRUPT_DISABLE, \
	CORTEX_A510_CPUACTLR_EL1_DATA_CORRUPT_SHIFT, CORTEX_A510_CPUACTLR_EL1_DATA_CORRUPT_WIDTH
workaround_reset_end cortex_a510, ERRATUM(2288014)

check_erratum_ls cortex_a510, ERRATUM(2288014), CPU_REV(1, 0)

workaround_reset_start cortex_a510, ERRATUM(2347730), ERRATA_A510_2347730
	/*
	 * Set CPUACTLR_EL1[17] to 1'b1, which disables
	 * specific microarchitectural clock gating
	 * behaviour.
	 */
	sysreg_bit_set CORTEX_A510_CPUACTLR_EL1, CORTEX_A510_CPUACTLR_EL1_BIT_17
workaround_reset_end cortex_a510, ERRATUM(2347730)

check_erratum_ls cortex_a510, ERRATUM(2347730), CPU_REV(1, 1)

workaround_reset_start cortex_a510, ERRATUM(2371937), ERRATA_A510_2371937
	/*
	 * Cacheable atomic operations can be forced
	 * to be executed near by setting
	 * IMP_CPUECTLR_EL1.ATOM=0b010. ATOM is found
	 * in [40:38] of CPUECTLR_EL1.
	 */
	sysreg_bitfield_insert CORTEX_A510_CPUECTLR_EL1, CORTEX_A510_CPUECTLR_EL1_ATOM_EXECALLINSTRNEAR, \
		CORTEX_A510_CPUECTLR_EL1_ATOM_SHIFT, CORTEX_A510_CPUECTLR_EL1_ATOM_WIDTH
workaround_reset_end cortex_a510, ERRATUM(2371937)

check_erratum_ls cortex_a510, ERRATUM(2371937), CPU_REV(1, 1)

workaround_reset_start cortex_a510, ERRATUM(2666669), ERRATA_A510_2666669
	sysreg_bit_set CORTEX_A510_CPUACTLR_EL1, CORTEX_A510_CPUACTLR_EL1_BIT_38
workaround_reset_end cortex_a510, ERRATUM(2666669)

check_erratum_ls cortex_a510, ERRATUM(2666669), CPU_REV(1, 1)

.global erratum_cortex_a510_2684597_wa
workaround_runtime_start cortex_a510, ERRATUM(2684597), ERRATA_A510_2684597, CORTEX_A510_MIDR
	/*
	 * Many assemblers do not yet understand the "tsb csync" mnemonic,
	 * so use the equivalent hint instruction.
	 */
	hint	#18			/* tsb csync */
workaround_runtime_end cortex_a510, ERRATUM(2684597)

check_erratum_ls cortex_a510, ERRATUM(2684597), CPU_REV(1, 2)

/*
 * ERRATA_DSU_2313941 :
 * The errata is defined in dsu_helpers.S but applies to cortex_a510
 * as well. Henceforth creating symbolic names to the already existing errata
 * workaround functions to get them registered under the Errata Framework.
 */
.equ check_erratum_cortex_a510_2313941, check_errata_dsu_2313941
.equ erratum_cortex_a510_2313941_wa, errata_dsu_2313941_wa
add_erratum_entry cortex_a510, ERRATUM(2313941), ERRATA_DSU_2313941, APPLY_AT_RESET

	/* ----------------------------------------------------
	 * HW will do the cache maintenance while powering down
	 * ----------------------------------------------------
	 */
func cortex_a510_core_pwr_dwn
	/* ---------------------------------------------------
	 * Enable CPU power down bit in power control register
	 * ---------------------------------------------------
	 */
	sysreg_bit_set CORTEX_A510_CPUPWRCTLR_EL1, CORTEX_A510_CPUPWRCTLR_EL1_CORE_PWRDN_BIT
	isb
	ret
endfunc cortex_a510_core_pwr_dwn

errata_report_shim cortex_a510

cpu_reset_func_start cortex_a510
	/* Disable speculative loads */
	msr	SSBS, xzr
cpu_reset_func_end cortex_a510

	/* ---------------------------------------------
	 * This function provides Cortex-A510 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a510_regs, "aS"
cortex_a510_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", ""

func cortex_a510_cpu_reg_dump
	adr	x6, cortex_a510_regs
	mrs	x8, CORTEX_A510_CPUECTLR_EL1
	ret
endfunc cortex_a510_cpu_reg_dump

declare_cpu_ops cortex_a510, CORTEX_A510_MIDR, \
	cortex_a510_reset_func, \
	cortex_a510_core_pwr_dwn

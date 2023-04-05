/*
 * Copyright (c) 2014-2023, Arm Limited and Contributors. All rights reserved.
 * Copyright (c) 2020, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include <arch.h>
#include <asm_macros.S>
#include <assert_macros.S>
#include <common/bl_common.h>
#include <common/debug.h>
#include <cortex_a57.h>
#include <cpu_macros.S>
#include <plat_macros.S>

	/* ---------------------------------------------
	 * Disable L1 data cache and unified L2 cache
	 * ---------------------------------------------
	 */
func cortex_a57_disable_dcache
	sysreg_bit_clear sctlr_el3, SCTLR_C_BIT
	isb
	ret
endfunc cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable all types of L2 prefetches.
	 * ---------------------------------------------
	 */
func cortex_a57_disable_l2_prefetch
	mrs	x0, CORTEX_A57_ECTLR_EL1
	orr	x0, x0, #CORTEX_A57_ECTLR_DIS_TWD_ACC_PFTCH_BIT
	mov	x1, #CORTEX_A57_ECTLR_L2_IPFTCH_DIST_MASK
	orr	x1, x1, #CORTEX_A57_ECTLR_L2_DPFTCH_DIST_MASK
	bic	x0, x0, x1
	msr	CORTEX_A57_ECTLR_EL1, x0
	isb
	dsb	ish
	ret
endfunc cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Disable intra-cluster coherency
	 * ---------------------------------------------
	 */
func cortex_a57_disable_smp
	sysreg_bit_clear CORTEX_A57_ECTLR_EL1, CORTEX_A57_ECTLR_SMP_BIT
	ret
endfunc cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Disable debug interfaces
	 * ---------------------------------------------
	 */
func cortex_a57_disable_ext_debug
	mov	x0, #1
	msr	osdlr_el1, x0
	isb

	apply_erratum cortex_a57, ERRATUM(817169), ERRATA_A57_817169

	dsb	sy
	ret
endfunc cortex_a57_disable_ext_debug

/*
 * Disable the over-read from the LDNP/STNP instruction. The SDEN doesn't
 * provide and erratum number, so assign it an obvious 1
 */
workaround_reset_start cortex_a57, ERRATUM(1), A57_DISABLE_NON_TEMPORAL_HINT
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_DIS_OVERREAD
workaround_reset_end cortex_a57, ERRATUM(1)

check_erratum_ls cortex_a57, ERRATUM(1), CPU_REV(1, 2)

workaround_reset_start cortex_a57, ERRATUM(806969), ERRATA_A57_806969
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_NO_ALLOC_WBWA
workaround_reset_end cortex_a57, ERRATUM(806969)

check_erratum_ls cortex_a57, ERRATUM(806969), CPU_REV(0, 0)

/* erratum always worked around, but report it correctly */
check_erratum_ls cortex_a57, ERRATUM(813419), CPU_REV(0, 0)
add_erratum_entry cortex_a57, ERRATUM(813419), ERRATUM_ALWAYS_CHOSEN, NO_APPLY_AT_RESET

workaround_reset_start cortex_a57, ERRATUM(813420), ERRATA_A57_813420
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_DCC_AS_DCCI
workaround_reset_end cortex_a57, ERRATUM(813420)

check_erratum_ls cortex_a57, ERRATUM(813420), CPU_REV(0, 0)

workaround_reset_start cortex_a57, ERRATUM(814670), ERRATA_A57_814670
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_DIS_DMB_NULLIFICATION
workaround_reset_end cortex_a57, ERRATUM(814670)

check_erratum_ls cortex_a57, ERRATUM(814670), CPU_REV(0, 0)

workaround_runtime_start cortex_a57, ERRATUM(817169), ERRATA_A57_817169, CORTEX_A57_MIDR
	/* Invalidate any TLB address */
	mov	x0, #0
	tlbi	vae3, x0
workaround_runtime_end cortex_a57, ERRATUM(817169), NO_ISB

check_erratum_ls cortex_a57, ERRATUM(817169), CPU_REV(0, 1)

workaround_reset_start cortex_a57, ERRATUM(826974), ERRATA_A57_826974
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_DIS_LOAD_PASS_DMB
workaround_reset_end cortex_a57, ERRATUM(826974)

check_erratum_ls cortex_a57, ERRATUM(826974), CPU_REV(1, 1)

workaround_reset_start cortex_a57, ERRATUM(826977), ERRATA_A57_826977
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_GRE_NGRE_AS_NGNRE
workaround_reset_end cortex_a57, ERRATUM(826977)

check_erratum_ls cortex_a57, ERRATUM(826977), CPU_REV(1, 1)

workaround_reset_start cortex_a57, ERRATUM(828024), ERRATA_A57_828024
	mrs	x1, CORTEX_A57_CPUACTLR_EL1
	/*
	 * Setting the relevant bits in CPUACTLR_EL1 has to be done in 2
	 * instructions here because the resulting bitmask doesn't fit in a
	 * 16-bit value so it cannot be encoded in a single instruction.
	 */
	orr	x1, x1, #CORTEX_A57_CPUACTLR_EL1_NO_ALLOC_WBWA
	orr	x1, x1, #(CORTEX_A57_CPUACTLR_EL1_DIS_L1_STREAMING | \
			  CORTEX_A57_CPUACTLR_EL1_DIS_STREAMING)
	msr	CORTEX_A57_CPUACTLR_EL1, x1
workaround_reset_end cortex_a57, ERRATUM(828024)

check_erratum_ls cortex_a57, ERRATUM(828024), CPU_REV(1, 1)

workaround_reset_start cortex_a57, ERRATUM(829520), ERRATA_A57_829520
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_DIS_INDIRECT_PREDICTOR
workaround_reset_end cortex_a57, ERRATUM(829520)

check_erratum_ls cortex_a57, ERRATUM(829520), CPU_REV(1, 2)

workaround_reset_start cortex_a57, ERRATUM(833471), ERRATA_A57_833471
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_FORCE_FPSCR_FLUSH
workaround_reset_end cortex_a57, ERRATUM(833471)

check_erratum_ls cortex_a57, ERRATUM(833471), CPU_REV(1, 2)

workaround_reset_start cortex_a57, ERRATUM(859972), ERRATA_A57_859972
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_DIS_INSTR_PREFETCH
workaround_reset_end cortex_a57, ERRATUM(859972)

check_erratum_ls cortex_a57, ERRATUM(859972), CPU_REV(1, 3)

check_erratum_chosen cortex_a57, ERRATUM(1319537), ERRATA_A57_1319537
/* erratum has no workaround in the cpu. Generic code must take care */
add_erratum_entry cortex_a57, ERRATUM(1319537), ERRATA_A57_1319537, NO_APPLY_AT_RESET

workaround_reset_start cortex_a57, CVE(2017, 5715), WORKAROUND_CVE_2017_5715
#if IMAGE_BL31
	override_vector_table wa_cve_2017_5715_mmu_vbar
#endif
workaround_reset_end cortex_a57, CVE(2017, 5715)

check_erratum_chosen cortex_a57, CVE(2017, 5715), WORKAROUND_CVE_2017_5715

workaround_reset_start cortex_a57, CVE(2018, 3639), WORKAROUND_CVE_2018_3639
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_DIS_LOAD_PASS_STORE
	isb
	dsb	sy
workaround_reset_end cortex_a57, CVE(2018, 3639)

check_erratum_chosen cortex_a57, CVE(2018, 3639), WORKAROUND_CVE_2018_3639

workaround_reset_start cortex_a57, CVE(2022, 23960), WORKAROUND_CVE_2022_23960
#if IMAGE_BL31
	override_vector_table wa_cve_2017_5715_mmu_vbar
#endif
workaround_reset_end cortex_a57, CVE(2022, 23960)

check_erratum_chosen cortex_a57, CVE(2022, 23960), WORKAROUND_CVE_2022_23960

cpu_reset_func_start cortex_a57
#if A57_ENABLE_NONCACHEABLE_LOAD_FWD
	/* Enable higher performance non-cacheable load forwarding */
	sysreg_bit_set CORTEX_A57_CPUACTLR_EL1, CORTEX_A57_CPUACTLR_EL1_EN_NC_LOAD_FWD
#endif
	/* Enable the SMP bit. */
	sysreg_bit_set CORTEX_A57_ECTLR_EL1, CORTEX_A57_ECTLR_SMP_BIT
cpu_reset_func_end cortex_a57

func check_smccc_arch_workaround_3
	mov	x0, #ERRATA_APPLIES
	ret
endfunc check_smccc_arch_workaround_3

	/* ----------------------------------------------------
	 * The CPU Ops core power down function for Cortex-A57.
	 * ----------------------------------------------------
	 */
func cortex_a57_core_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

	/* ---------------------------------------------
	 * Flush L1 caches.
	 * ---------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_core_pwr_dwn

	/* -------------------------------------------------------
	 * The CPU Ops cluster power down function for Cortex-A57.
	 * -------------------------------------------------------
	 */
func cortex_a57_cluster_pwr_dwn
	mov	x18, x30

	/* ---------------------------------------------
	 * Turn off caches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_dcache

	/* ---------------------------------------------
	 * Disable the L2 prefetches.
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_l2_prefetch

#if !SKIP_A57_L1_FLUSH_PWR_DWN
	/* -------------------------------------------------
	 * Flush the L1 caches.
	 * -------------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level1
#endif
	/* ---------------------------------------------
	 * Disable the optional ACP.
	 * ---------------------------------------------
	 */
	bl	plat_disable_acp

	/* -------------------------------------------------
	 * Flush the L2 caches.
	 * -------------------------------------------------
	 */
	mov	x0, #DCCISW
	bl	dcsw_op_level2

	/* ---------------------------------------------
	 * Come out of intra cluster coherency
	 * ---------------------------------------------
	 */
	bl	cortex_a57_disable_smp

	/* ---------------------------------------------
	 * Force the debug interfaces to be quiescent
	 * ---------------------------------------------
	 */
	mov	x30, x18
	b	cortex_a57_disable_ext_debug
endfunc cortex_a57_cluster_pwr_dwn

errata_report_shim cortex_a57

	/* ---------------------------------------------
	 * This function provides cortex_a57 specific
	 * register information for crash reporting.
	 * It needs to return with x6 pointing to
	 * a list of register names in ascii and
	 * x8 - x15 having values of registers to be
	 * reported.
	 * ---------------------------------------------
	 */
.section .rodata.cortex_a57_regs, "aS"
cortex_a57_regs:  /* The ascii list of register names to be reported */
	.asciz	"cpuectlr_el1", "cpumerrsr_el1", "l2merrsr_el1", ""

func cortex_a57_cpu_reg_dump
	adr	x6, cortex_a57_regs
	mrs	x8, CORTEX_A57_ECTLR_EL1
	mrs	x9, CORTEX_A57_MERRSR_EL1
	mrs	x10, CORTEX_A57_L2MERRSR_EL1
	ret
endfunc cortex_a57_cpu_reg_dump

declare_cpu_ops_wa cortex_a57, CORTEX_A57_MIDR, \
	cortex_a57_reset_func, \
	check_erratum_cortex_a57_5715, \
	CPU_NO_EXTRA2_FUNC, \
	check_smccc_arch_workaround_3, \
	cortex_a57_core_pwr_dwn, \
	cortex_a57_cluster_pwr_dwn

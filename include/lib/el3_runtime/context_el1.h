/*
 * Copyright (c) 2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CONTEXT_EL1_H
#define CONTEXT_EL1_H

#include <lib/extensions/sysreg128.h>

#ifndef __ASSEMBLER__

/*******************************************************************************
 * EL1 Registers:
 * AArch64 EL1 system register context structure for preserving the
 * architectural state during world switches.
 ******************************************************************************/

typedef struct el1_common_regs {
	uint64_t spsr_el1;
	uint64_t elr_el1;

#if (!ERRATA_SPECULATIVE_AT)
	uint64_t sctlr_el1;
	uint64_t tcr_el1;
#endif /* ERRATA_SPECULATIVE_AT=0 */

	uint64_t cpacr_el1;
	uint64_t csselr_el1;
	uint64_t sp_el1;
	uint64_t esr_el1;
	uint64_t mair_el1;
	uint64_t amair_el1;
	uint64_t actlr_el1;
	uint64_t tpidr_el1;
	uint64_t tpidr_el0;
	uint64_t tpidrro_el0;
	uint64_t far_el1;
	uint64_t afsr0_el1;
	uint64_t afsr1_el1;
	uint64_t contextidr_el1;
	uint64_t vbar_el1;
	uint64_t mdccint_el1;
	uint64_t mdscr_el1;
	sysreg_t par_el1;
	sysreg_t ttbr0_el1;
	sysreg_t ttbr1_el1;
} el1_common_regs_t;

typedef struct el1_aarch32_regs {
	uint64_t spsr_abt;
	uint64_t spsr_und;
	uint64_t spsr_irq;
	uint64_t spsr_fiq;
	uint64_t dacr32_el2;
	uint64_t ifsr32_el2;
} el1_aarch32_regs_t;

typedef struct el1_arch_timer_regs {
	uint64_t cntp_ctl_el0;
	uint64_t cntp_cval_el0;
	uint64_t cntv_ctl_el0;
	uint64_t cntv_cval_el0;
	uint64_t cntkctl_el1;
} el1_arch_timer_regs_t;

typedef struct el1_mte2_regs {
	uint64_t tfsre0_el1;
	uint64_t tfsr_el1;
	uint64_t rgsr_el1;
	uint64_t gcr_el1;
} el1_mte2_regs_t;

typedef struct el1_ras_regs {
	uint64_t disr_el1;
} el1_ras_regs_t;

typedef struct el1_s1pie_regs {
	uint64_t pire0_el1;
	uint64_t pir_el1;
} el1_s1pie_regs_t;

typedef struct el1_s1poe_regs {
	uint64_t por_el1;
} el1_s1poe_regs_t;

typedef struct el1_s2poe_regs {
	uint64_t s2por_el1;
} el1_s2poe_regs_t;

typedef struct el1_tcr2_regs {
	uint64_t tcr2_el1;
} el1_tcr2_regs_t;

typedef struct el1_trf_regs {
	uint64_t trfcr_el1;
} el1_trf_regs_t;

typedef struct el1_csv2_2_regs {
	uint64_t scxtnum_el0;
	uint64_t scxtnum_el1;
} el1_csv2_2_regs_t;

typedef struct el1_gcs_regs {
	uint64_t gcscr_el1;
	uint64_t gcscre0_el1;
	uint64_t gcspr_el1;
	uint64_t gcspr_el0;
} el1_gcs_regs_t;

typedef struct el1_the_regs {
	sysreg_t rcwmask_el1;
	sysreg_t rcwsmask_el1;
} el1_the_regs_t;

typedef struct el1_sctlr2_regs {
	uint64_t sctlr2_el1;
} el1_sctlr2_regs_t;

typedef struct el1_ls64_regs {
	uint64_t accdata_el1;
} el1_ls64_regs_t;

typedef struct el1_sysregs {

	el1_common_regs_t common;

#if CTX_INCLUDE_AARCH32_REGS
	el1_aarch32_regs_t el1_aarch32;
#endif

#if NS_TIMER_SWITCH
	el1_arch_timer_regs_t arch_timer;
#endif

#if ENABLE_FEAT_MTE2
	el1_mte2_regs_t mte2;
#endif

#if ENABLE_FEAT_RAS
	el1_ras_regs_t ras;
#endif

#if ENABLE_FEAT_S1PIE
	el1_s1pie_regs_t s1pie;
#endif

#if ENABLE_FEAT_S1POE
	el1_s1poe_regs_t s1poe;
#endif

#if ENABLE_FEAT_S2POE
	el1_s2poe_regs_t s2poe;
#endif

#if ENABLE_FEAT_TCR2
	el1_tcr2_regs_t tcr2;
#endif

#if ENABLE_TRF_FOR_NS
	el1_trf_regs_t trf;
#endif

#if ENABLE_FEAT_CSV2_2
	el1_csv2_2_regs_t csv2_2;
#endif

#if ENABLE_FEAT_GCS
	el1_gcs_regs_t gcs;
#endif

#if ENABLE_FEAT_THE
	el1_the_regs_t the;
#endif

#if ENABLE_FEAT_SCTLR2
	el1_sctlr2_regs_t sctlr2;
#endif

#if ENABLE_FEAT_LS64_ACCDATA
	el1_ls64_regs_t ls64;
#endif
} el1_sysregs_t;


/*
 * Macros to access members related to individual features of the el1_sysregs_t
 * structures.
 */

#define read_el1_ctx_common(ctx, reg)		(((ctx)->common).reg)

#define write_el1_ctx_common(ctx, reg, val)	((((ctx)->common).reg)	\
							= (uint64_t) (val))

#if NS_TIMER_SWITCH
#define read_el1_ctx_arch_timer(ctx, reg)		(((ctx)->arch_timer).reg)
#define write_el1_ctx_arch_timer(ctx, reg, val)	((((ctx)->arch_timer).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_arch_timer(ctx, reg)		ULL(0)
#define write_el1_ctx_arch_timer(ctx, reg, val)
#endif /* NS_TIMER_SWITCH */

#if CTX_INCLUDE_AARCH32_REGS
#define read_el1_ctx_aarch32(ctx, reg)		(((ctx)->el1_aarch32).reg)
#define write_el1_ctx_aarch32(ctx, reg, val)	((((ctx)->el1_aarch32).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_aarch32(ctx, reg)		ULL(0)
#define write_el1_ctx_aarch32(ctx, reg, val)
#endif /* CTX_INCLUDE_AARCH32_REGS */

#if ENABLE_FEAT_MTE2
#define read_el1_ctx_mte2(ctx, reg)		(((ctx)->mte2).reg)
#define write_el1_ctx_mte2(ctx, reg, val)	((((ctx)->mte2).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_mte2(ctx, reg)		ULL(0)
#define write_el1_ctx_mte2(ctx, reg, val)
#endif /* ENABLE_FEAT_MTE2 */

#if ENABLE_FEAT_RAS
#define read_el1_ctx_ras(ctx, reg)		(((ctx)->ras).reg)
#define write_el1_ctx_ras(ctx, reg, val)	((((ctx)->ras).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_ras(ctx, reg)		ULL(0)
#define write_el1_ctx_ras(ctx, reg, val)
#endif /* ENABLE_FEAT_RAS */

#if ENABLE_FEAT_S1PIE
#define read_el1_ctx_s1pie(ctx, reg)		(((ctx)->s1pie).reg)
#define write_el1_ctx_s1pie(ctx, reg, val)	((((ctx)->s1pie).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_s1pie(ctx, reg)		ULL(0)
#define write_el1_ctx_s1pie(ctx, reg, val)
#endif /* ENABLE_FEAT_S1PIE */

#if ENABLE_FEAT_S1POE
#define read_el1_ctx_s1poe(ctx, reg)		(((ctx)->s1poe).reg)
#define write_el1_ctx_s1poe(ctx, reg, val)	((((ctx)->s1poe).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_s1poe(ctx, reg)		ULL(0)
#define write_el1_ctx_s1poe(ctx, reg, val)
#endif /* ENABLE_FEAT_S1POE */

#if ENABLE_FEAT_S2POE
#define read_el1_ctx_s2poe(ctx, reg)		(((ctx)->s2poe).reg)
#define write_el1_ctx_s2poe(ctx, reg, val)	((((ctx)->s2poe).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_s2poe(ctx, reg)		ULL(0)
#define write_el1_ctx_s2poe(ctx, reg, val)
#endif /* ENABLE_FEAT_S2POE */

#if ENABLE_FEAT_TCR2
#define read_el1_ctx_tcr2(ctx, reg)		(((ctx)->tcr2).reg)
#define write_el1_ctx_tcr2(ctx, reg, val)	((((ctx)->tcr2).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_tcr2(ctx, reg)		ULL(0)
#define write_el1_ctx_tcr2(ctx, reg, val)
#endif /* ENABLE_FEAT_TCR2 */

#if ENABLE_TRF_FOR_NS
#define read_el1_ctx_trf(ctx, reg)		(((ctx)->trf).reg)
#define write_el1_ctx_trf(ctx, reg, val)	((((ctx)->trf).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_trf(ctx, reg)		ULL(0)
#define write_el1_ctx_trf(ctx, reg, val)
#endif /* ENABLE_TRF_FOR_NS */

#if ENABLE_FEAT_CSV2_2
#define read_el1_ctx_csv2_2(ctx, reg)		(((ctx)->csv2_2).reg)
#define write_el1_ctx_csv2_2(ctx, reg, val)	((((ctx)->csv2_2).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_csv2_2(ctx, reg)		ULL(0)
#define write_el1_ctx_csv2_2(ctx, reg, val)
#endif /* ENABLE_FEAT_CSV2_2 */

#if ENABLE_FEAT_GCS
#define read_el1_ctx_gcs(ctx, reg)		(((ctx)->gcs).reg)
#define write_el1_ctx_gcs(ctx, reg, val)	((((ctx)->gcs).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_gcs(ctx, reg)		ULL(0)
#define write_el1_ctx_gcs(ctx, reg, val)
#endif /* ENABLE_FEAT_GCS */

#if ENABLE_FEAT_THE
#define read_el1_ctx_the(ctx, reg)		(((ctx)->the).reg)
#define write_el1_ctx_the(ctx, reg, val)	((((ctx)->the).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_the(ctx, reg)		ULL(0)
#define write_el1_ctx_the(ctx, reg, val)
#endif /* ENABLE_FEAT_THE */

#if ENABLE_FEAT_SCTLR2
#define read_el1_ctx_sctlr2(ctx, reg)		(((ctx)->sctlr2).reg)
#define write_el1_ctx_sctlr2(ctx, reg, val)	((((ctx)->sctlr2).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_sctlr2(ctx, reg)		ULL(0)
#define write_el1_ctx_sctlr2(ctx, reg, val)
#endif /* ENABLE_FEAT_SCTLR2 */

#if ENABLE_FEAT_LS64_ACCDATA
#define read_el1_ctx_ls64(ctx, reg)		(((ctx)->ls64).reg)
#define write_el1_ctx_ls64(ctx, reg, val)	((((ctx)->ls64).reg)	\
							= (uint64_t) (val))
#else
#define read_el1_ctx_ls64(ctx, reg)		ULL(0)
#define write_el1_ctx_ls64(ctx, reg, val)
#endif /* ENABLE_FEAT_LS64_ACCDATA */
/******************************************************************************/
#endif /* __ASSEMBLER__ */

#endif /* CONTEXT_EL1_H */

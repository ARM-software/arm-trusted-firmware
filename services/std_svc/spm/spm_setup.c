/*
 * Copyright (c) 2017-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <platform_def.h>

#include <arch.h>
#include <arch_helpers.h>
#include <context.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <services/sp_res_desc.h>
#include <sprt_host.h>

#include "spm_private.h"
#include "spm_shim_private.h"

/* Setup context of the Secure Partition */
void spm_sp_setup(sp_context_t *sp_ctx)
{
	cpu_context_t *ctx = &(sp_ctx->cpu_ctx);

	/*
	 * Initialize CPU context
	 * ----------------------
	 */

	entry_point_info_t ep_info = {0};

	SET_PARAM_HEAD(&ep_info, PARAM_EP, VERSION_1, SECURE | EP_ST_ENABLE);

	/* Setup entrypoint and SPSR */
	ep_info.pc = sp_ctx->rd.attribute.entrypoint;
	ep_info.spsr = SPSR_64(MODE_EL0, MODE_SP_EL0, DISABLE_ALL_EXCEPTIONS);

	/*
	 * X0: Unused (MBZ).
	 * X1: Unused (MBZ).
	 * X2: cookie value (Implementation Defined)
	 * X3: cookie value (Implementation Defined)
	 * X4 to X7 = 0
	 */
	ep_info.args.arg0 = 0;
	ep_info.args.arg1 = 0;
	ep_info.args.arg2 = PLAT_SPM_COOKIE_0;
	ep_info.args.arg3 = PLAT_SPM_COOKIE_1;

	cm_setup_context(ctx, &ep_info);

	/*
	 * Setup translation tables
	 * ------------------------
	 */

	/* Assign translation tables context. */
	spm_sp_xlat_context_alloc(sp_ctx);

	sp_map_memory_regions(sp_ctx);

	/*
	 * MMU-related registers
	 * ---------------------
	 */
	xlat_ctx_t *xlat_ctx = sp_ctx->xlat_ctx_handle;

	uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, 0, xlat_ctx->base_table,
		      xlat_ctx->pa_max_address, xlat_ctx->va_max_address,
		      EL1_EL0_REGIME);

	write_ctx_reg(get_sysregs_ctx(ctx), CTX_MAIR_EL1,
		      mmu_cfg_params[MMU_CFG_MAIR]);

	/* Enable translations using TTBR1_EL1 */
	int t1sz = 64 - __builtin_ctzll(SPM_SHIM_XLAT_VIRT_ADDR_SPACE_SIZE);
	mmu_cfg_params[MMU_CFG_TCR] &= ~TCR_EPD1_BIT;
	mmu_cfg_params[MMU_CFG_TCR] |=
		((uint64_t)t1sz << TCR_T1SZ_SHIFT) |
		TCR_SH1_INNER_SHAREABLE |
		TCR_RGN1_OUTER_WBA | TCR_RGN1_INNER_WBA |
		TCR_TG1_4K;

	write_ctx_reg(get_sysregs_ctx(ctx), CTX_TCR_EL1,
		      mmu_cfg_params[MMU_CFG_TCR]);

	write_ctx_reg(get_sysregs_ctx(ctx), CTX_TTBR0_EL1,
		      mmu_cfg_params[MMU_CFG_TTBR0]);

	write_ctx_reg(get_sysregs_ctx(ctx), CTX_TTBR1_EL1,
		      (uint64_t)spm_exceptions_xlat_get_base_table());

	/* Setup SCTLR_EL1 */
	u_register_t sctlr_el1 = read_ctx_reg(get_sysregs_ctx(ctx), CTX_SCTLR_EL1);

	sctlr_el1 |=
		/*SCTLR_EL1_RES1 |*/
		/* Don't trap DC CVAU, DC CIVAC, DC CVAC, DC CVAP, or IC IVAU */
		SCTLR_UCI_BIT							|
		/* RW regions at xlat regime EL1&0 are forced to be XN. */
		SCTLR_WXN_BIT							|
		/* Don't trap to EL1 execution of WFI or WFE at EL0. */
		SCTLR_NTWI_BIT | SCTLR_NTWE_BIT					|
		/* Don't trap to EL1 accesses to CTR_EL0 from EL0. */
		SCTLR_UCT_BIT							|
		/* Don't trap to EL1 execution of DZ ZVA at EL0. */
		SCTLR_DZE_BIT							|
		/* Enable SP Alignment check for EL0 */
		SCTLR_SA0_BIT							|
		/* Allow cacheable data and instr. accesses to normal memory. */
		SCTLR_C_BIT | SCTLR_I_BIT					|
		/* Alignment fault checking enabled when at EL1 and EL0. */
		SCTLR_A_BIT							|
		/* Enable MMU. */
		SCTLR_M_BIT
	;

	sctlr_el1 &= ~(
		/* Explicit data accesses at EL0 are little-endian. */
		SCTLR_E0E_BIT							|
		/* Accesses to DAIF from EL0 are trapped to EL1. */
		SCTLR_UMA_BIT
	);

	write_ctx_reg(get_sysregs_ctx(ctx), CTX_SCTLR_EL1, sctlr_el1);

	/*
	 * Setup other system registers
	 * ----------------------------
	 */

	/*
	 * Shim exception vector base address. It is mapped at the start of the
	 * address space accessed by TTBR1_EL1, which means that the base
	 * address of the exception vectors depends on the size of the address
	 * space specified in TCR_EL1.T1SZ.
	 */
	write_ctx_reg(get_sysregs_ctx(ctx), CTX_VBAR_EL1,
		      UINT64_MAX - (SPM_SHIM_XLAT_VIRT_ADDR_SPACE_SIZE - 1ULL));

	/*
	 * FPEN: Allow the Secure Partition to access FP/SIMD registers.
	 * Note that SPM will not do any saving/restoring of these registers on
	 * behalf of the SP. This falls under the SP's responsibility.
	 * TTA: Enable access to trace registers.
	 * ZEN (v8.2): Trap SVE instructions and access to SVE registers.
	 */
	write_ctx_reg(get_sysregs_ctx(ctx), CTX_CPACR_EL1,
			CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_NONE));

	/*
	 * Prepare shared buffers
	 * ----------------------
	 */

	/* Initialize SPRT queues */
	sprt_initialize_queues((void *)sp_ctx->spm_sp_buffer_base,
			       sp_ctx->spm_sp_buffer_size);
}

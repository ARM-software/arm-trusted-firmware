/*
 * Copyright (c) 2017-2022, ARM Limited and Contributors. All rights reserved.
 * Copyright (c) 2021, NVIDIA Corporation. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <context.h>
#include <common/debug.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <services/spm_mm_partition.h>

#include "spm_common.h"
#include "spm_mm_private.h"
#include "spm_mm_shim_private.h"

/* Setup context of the Secure Partition */
void spm_sp_setup(sp_context_t *sp_ctx)
{
	cpu_context_t *ctx = &(sp_ctx->cpu_ctx);

	/* Pointer to the MP information from the platform port. */
	const spm_mm_boot_info_t *sp_boot_info =
			plat_get_secure_partition_boot_info(NULL);

	/*
	 * Initialize CPU context
	 * ----------------------
	 */

	entry_point_info_t ep_info = {0};

	SET_PARAM_HEAD(&ep_info, PARAM_EP, VERSION_1, SECURE | EP_ST_ENABLE);

	/* Setup entrypoint and SPSR */
	ep_info.pc = sp_boot_info->sp_image_base;
	ep_info.spsr = SPSR_64(MODE_EL0, MODE_SP_EL0, DISABLE_ALL_EXCEPTIONS);

	/*
	 * X0: Virtual address of a buffer shared between EL3 and Secure EL0.
	 *     The buffer will be mapped in the Secure EL1 translation regime
	 *     with Normal IS WBWA attributes and RO data and Execute Never
	 *     instruction access permissions.
	 *
	 * X1: Size of the buffer in bytes
	 *
	 * X2: cookie value (Implementation Defined)
	 *
	 * X3: cookie value (Implementation Defined)
	 *
	 * X4 to X7 = 0
	 */
	ep_info.args.arg0 = sp_boot_info->sp_shared_buf_base;
	ep_info.args.arg1 = sp_boot_info->sp_shared_buf_size;
	ep_info.args.arg2 = PLAT_SPM_COOKIE_0;
	ep_info.args.arg3 = PLAT_SPM_COOKIE_1;

	cm_setup_context(ctx, &ep_info);

	/*
	 * SP_EL0: A non-zero value will indicate to the SP that the SPM has
	 * initialized the stack pointer for the current CPU through
	 * implementation defined means. The value will be 0 otherwise.
	 */
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_SP_EL0,
			sp_boot_info->sp_stack_base + sp_boot_info->sp_pcpu_stack_size);

	/*
	 * Setup translation tables
	 * ------------------------
	 */

#if ENABLE_ASSERTIONS

	/* Get max granularity supported by the platform. */
	unsigned int max_granule = xlat_arch_get_max_supported_granule_size();

	VERBOSE("Max translation granule size supported: %u KiB\n",
		max_granule / 1024U);

	unsigned int max_granule_mask = max_granule - 1U;

	/* Base must be aligned to the max granularity */
	assert((sp_boot_info->sp_ns_comm_buf_base & max_granule_mask) == 0);

	/* Size must be a multiple of the max granularity */
	assert((sp_boot_info->sp_ns_comm_buf_size & max_granule_mask) == 0);

#endif /* ENABLE_ASSERTIONS */

	/* This region contains the exception vectors used at S-EL1. */
	const mmap_region_t sel1_exception_vectors =
		MAP_REGION_FLAT(SPM_SHIM_EXCEPTIONS_START,
				SPM_SHIM_EXCEPTIONS_SIZE,
				MT_CODE | MT_SECURE | MT_PRIVILEGED);
	mmap_add_region_ctx(sp_ctx->xlat_ctx_handle,
			    &sel1_exception_vectors);

	mmap_add_ctx(sp_ctx->xlat_ctx_handle,
		     plat_get_secure_partition_mmap(NULL));

	init_xlat_tables_ctx(sp_ctx->xlat_ctx_handle);

	/*
	 * MMU-related registers
	 * ---------------------
	 */
	xlat_ctx_t *xlat_ctx = sp_ctx->xlat_ctx_handle;

	uint64_t mmu_cfg_params[MMU_CFG_PARAM_MAX];

	setup_mmu_cfg((uint64_t *)&mmu_cfg_params, 0, xlat_ctx->base_table,
		      xlat_ctx->pa_max_address, xlat_ctx->va_max_address,
		      EL1_EL0_REGIME);

	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_MAIR_EL1,
		      mmu_cfg_params[MMU_CFG_MAIR]);

	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_TCR_EL1,
		      mmu_cfg_params[MMU_CFG_TCR]);

	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_TTBR0_EL1,
		      mmu_cfg_params[MMU_CFG_TTBR0]);

	/* Setup SCTLR_EL1 */
	u_register_t sctlr_el1 = read_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_SCTLR_EL1);

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
		/* Don't change PSTATE.PAN on taking an exception to EL1 */
		SCTLR_SPAN_BIT							|
		/* Allow cacheable data and instr. accesses to normal memory. */
		SCTLR_C_BIT | SCTLR_I_BIT					|
		/* Enable MMU. */
		SCTLR_M_BIT
	;

	sctlr_el1 &= ~(
		/* Explicit data accesses at EL0 are little-endian. */
		SCTLR_E0E_BIT							|
		/*
		 * Alignment fault checking disabled when at EL1 and EL0 as
		 * the UEFI spec permits unaligned accesses.
		 */
		SCTLR_A_BIT							|
		/* Accesses to DAIF from EL0 are trapped to EL1. */
		SCTLR_UMA_BIT
	);

	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_SCTLR_EL1, sctlr_el1);

	/*
	 * Setup other system registers
	 * ----------------------------
	 */

	/* Shim Exception Vector Base Address */
	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_VBAR_EL1,
			SPM_SHIM_EXCEPTIONS_PTR);

	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_CNTKCTL_EL1,
		      EL0PTEN_BIT | EL0VTEN_BIT | EL0PCTEN_BIT | EL0VCTEN_BIT);

	/*
	 * FPEN: Allow the Secure Partition to access FP/SIMD registers.
	 * Note that SPM will not do any saving/restoring of these registers on
	 * behalf of the SP. This falls under the SP's responsibility.
	 * TTA: Enable access to trace registers.
	 * ZEN (v8.2): Trap SVE instructions and access to SVE registers.
	 */
	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_CPACR_EL1,
			CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_NONE));

	/*
	 * Prepare information in buffer shared between EL3 and S-EL0
	 * ----------------------------------------------------------
	 */

	void *shared_buf_ptr = (void *) sp_boot_info->sp_shared_buf_base;

	/* Copy the boot information into the shared buffer with the SP. */
	assert((uintptr_t)shared_buf_ptr + sizeof(spm_mm_boot_info_t)
	       <= (sp_boot_info->sp_shared_buf_base + sp_boot_info->sp_shared_buf_size));

	assert(sp_boot_info->sp_shared_buf_base <=
				(UINTPTR_MAX - sp_boot_info->sp_shared_buf_size + 1));

	assert(sp_boot_info != NULL);

	memcpy((void *) shared_buf_ptr, (const void *) sp_boot_info,
	       sizeof(spm_mm_boot_info_t));

	/* Pointer to the MP information from the platform port. */
	spm_mm_mp_info_t *sp_mp_info =
		((spm_mm_boot_info_t *) shared_buf_ptr)->mp_info;

	assert(sp_mp_info != NULL);

	/*
	 * Point the shared buffer MP information pointer to where the info will
	 * be populated, just after the boot info.
	 */
	((spm_mm_boot_info_t *) shared_buf_ptr)->mp_info =
		(spm_mm_mp_info_t *) ((uintptr_t)shared_buf_ptr
				+ sizeof(spm_mm_boot_info_t));

	/*
	 * Update the shared buffer pointer to where the MP information for the
	 * payload will be populated
	 */
	shared_buf_ptr = ((spm_mm_boot_info_t *) shared_buf_ptr)->mp_info;

	/*
	 * Copy the cpu information into the shared buffer area after the boot
	 * information.
	 */
	assert(sp_boot_info->num_cpus <= PLATFORM_CORE_COUNT);

	assert((uintptr_t)shared_buf_ptr
	       <= (sp_boot_info->sp_shared_buf_base + sp_boot_info->sp_shared_buf_size -
		       (sp_boot_info->num_cpus * sizeof(*sp_mp_info))));

	memcpy(shared_buf_ptr, (const void *) sp_mp_info,
		sp_boot_info->num_cpus * sizeof(*sp_mp_info));

	/*
	 * Calculate the linear indices of cores in boot information for the
	 * secure partition and flag the primary CPU
	 */
	sp_mp_info = (spm_mm_mp_info_t *) shared_buf_ptr;

	for (unsigned int index = 0; index < sp_boot_info->num_cpus; index++) {
		u_register_t mpidr = sp_mp_info[index].mpidr;

		sp_mp_info[index].linear_id = plat_core_pos_by_mpidr(mpidr);
		if (plat_my_core_pos() == sp_mp_info[index].linear_id)
			sp_mp_info[index].flags |= MP_INFO_FLAG_PRIMARY_CPU;
	}
}

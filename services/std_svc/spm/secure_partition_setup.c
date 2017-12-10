/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <common_def.h>
#include <context.h>
#include <context_mgmt.h>
#include <debug.h>
#include <platform_def.h>
#include <platform.h>
#include <secure_partition.h>
#include <string.h>
#include <types.h>
#include <xlat_tables_v2.h>

#include "spm_private.h"
#include "spm_shim_private.h"

/* Place translation tables by default along with the ones used by BL31. */
#ifndef PLAT_SP_IMAGE_XLAT_SECTION_NAME
#define PLAT_SP_IMAGE_XLAT_SECTION_NAME	"xlat_table"
#endif

/* Allocate and initialise the translation context for the secure partition. */
REGISTER_XLAT_CONTEXT2(secure_partition,
			PLAT_SP_IMAGE_MMAP_REGIONS,
			PLAT_SP_IMAGE_MAX_XLAT_TABLES,
			PLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE,
			EL1_EL0_REGIME, PLAT_SP_IMAGE_XLAT_SECTION_NAME);

/* Export a handle on the secure partition translation context */
xlat_ctx_t *secure_partition_xlat_ctx_handle = &secure_partition_xlat_ctx;

/* Setup context of the Secure Partition */
void secure_partition_setup(void)
{
	VERBOSE("S-EL1/S-EL0 context setup start...\n");

	cpu_context_t *ctx = cm_get_context(SECURE);

	/* Make sure that we got a Secure context. */
	assert(ctx != NULL);

	/* Assert we are in Secure state. */
	assert((read_scr_el3() & SCR_NS_BIT) == 0);

	/* Disable MMU at EL1. */
	disable_mmu_icache_el1();

	/* Invalidate TLBs at EL1. */
	tlbivmalle1();

	/*
	 * General-Purpose registers
	 * -------------------------
	 */

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
	 * X4 to X30 = 0 (already done by cm_init_my_context())
	 */
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0, PLAT_SPM_BUF_BASE);
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X1, PLAT_SPM_BUF_SIZE);
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X2, PLAT_SPM_COOKIE_0);
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X3, PLAT_SPM_COOKIE_1);

	/*
	 * SP_EL0: A non-zero value will indicate to the SP that the SPM has
	 * initialized the stack pointer for the current CPU through
	 * implementation defined means. The value will be 0 otherwise.
	 */
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_SP_EL0,
			PLAT_SP_IMAGE_STACK_BASE + PLAT_SP_IMAGE_STACK_PCPU_SIZE);

	/*
	 * Setup translation tables
	 * ------------------------
	 */

#if ENABLE_ASSERTIONS

	/* Get max granularity supported by the platform. */

	u_register_t id_aa64mmfr0_el1 = read_id_aa64mmfr0_el1();

	int tgran64_supported =
		((id_aa64mmfr0_el1 >> ID_AA64MMFR0_EL1_TGRAN64_SHIFT) &
		 ID_AA64MMFR0_EL1_TGRAN64_MASK) ==
		 ID_AA64MMFR0_EL1_TGRAN64_SUPPORTED;

	int tgran16_supported =
		((id_aa64mmfr0_el1 >> ID_AA64MMFR0_EL1_TGRAN16_SHIFT) &
		 ID_AA64MMFR0_EL1_TGRAN16_MASK) ==
		 ID_AA64MMFR0_EL1_TGRAN16_SUPPORTED;

	int tgran4_supported =
		((id_aa64mmfr0_el1 >> ID_AA64MMFR0_EL1_TGRAN4_SHIFT) &
		 ID_AA64MMFR0_EL1_TGRAN4_MASK) ==
		 ID_AA64MMFR0_EL1_TGRAN4_SUPPORTED;

	uintptr_t max_granule_size;

	if (tgran64_supported) {
		max_granule_size = 64 * 1024;
	} else if (tgran16_supported) {
		max_granule_size = 16 * 1024;
	} else {
		assert(tgran4_supported);
		max_granule_size = 4 * 1024;
	}

	VERBOSE("Max translation granule supported: %lu KiB\n",
		max_granule_size / 1024);

	uintptr_t max_granule_size_mask = max_granule_size - 1;

	/* Base must be aligned to the max granularity */
	assert((ARM_SP_IMAGE_NS_BUF_BASE & max_granule_size_mask) == 0);

	/* Size must be a multiple of the max granularity */
	assert((ARM_SP_IMAGE_NS_BUF_SIZE & max_granule_size_mask) == 0);

#endif /* ENABLE_ASSERTIONS */

	/* This region contains the exception vectors used at S-EL1. */
	const mmap_region_t sel1_exception_vectors =
		MAP_REGION_FLAT(SPM_SHIM_EXCEPTIONS_START,
				SPM_SHIM_EXCEPTIONS_SIZE,
				MT_CODE | MT_SECURE | MT_PRIVILEGED);
	mmap_add_region_ctx(&secure_partition_xlat_ctx,
			    &sel1_exception_vectors);

	mmap_add_ctx(&secure_partition_xlat_ctx,
		     plat_get_secure_partition_mmap(NULL));

	init_xlat_tables_ctx(&secure_partition_xlat_ctx);

	/*
	 * MMU-related registers
	 * ---------------------
	 */

	/* Set attributes in the right indices of the MAIR */
	u_register_t mair_el1 =
		MAIR_ATTR_SET(ATTR_DEVICE, ATTR_DEVICE_INDEX) |
		MAIR_ATTR_SET(ATTR_IWBWA_OWBWA_NTR, ATTR_IWBWA_OWBWA_NTR_INDEX) |
		MAIR_ATTR_SET(ATTR_NON_CACHEABLE, ATTR_NON_CACHEABLE_INDEX);

	write_ctx_reg(get_sysregs_ctx(ctx), CTX_MAIR_EL1, mair_el1);

	/* Setup TCR_EL1. */
	u_register_t tcr_ps_bits = tcr_physical_addr_size_bits(PLAT_PHY_ADDR_SPACE_SIZE);

	u_register_t tcr_el1 =
		/* Size of region addressed by TTBR0_EL1 = 2^(64-T0SZ) bytes. */
		(64 - __builtin_ctzl(PLAT_VIRT_ADDR_SPACE_SIZE))		|
		/* Inner and outer WBWA, shareable. */
		TCR_SH_INNER_SHAREABLE | TCR_RGN_OUTER_WBA | TCR_RGN_INNER_WBA	|
		/* Set the granularity to 4KB. */
		TCR_TG0_4K							|
		/* Limit Intermediate Physical Address Size. */
		tcr_ps_bits << TCR_EL1_IPS_SHIFT				|
		/* Disable translations using TBBR1_EL1. */
		TCR_EPD1_BIT
		/* The remaining fields related to TBBR1_EL1 are left as zero. */
	;

	tcr_el1 &= ~(
		/* Enable translations using TBBR0_EL1 */
		TCR_EPD0_BIT
	);

	write_ctx_reg(get_sysregs_ctx(ctx), CTX_TCR_EL1, tcr_el1);

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

	/* Point TTBR0_EL1 at the tables of the context created for the SP. */
	write_ctx_reg(get_sysregs_ctx(ctx), CTX_TTBR0_EL1,
			(u_register_t)secure_partition_base_xlat_table);

	/*
	 * Setup other system registers
	 * ----------------------------
	 */

	/* Shim Exception Vector Base Address */
	write_ctx_reg(get_sysregs_ctx(ctx), CTX_VBAR_EL1,
			SPM_SHIM_EXCEPTIONS_PTR);

	/*
	 * FPEN: Forbid the Secure Partition to access FP/SIMD registers.
	 * TTA: Enable access to trace registers.
	 * ZEN (v8.2): Trap SVE instructions and access to SVE registers.
	 */
	write_ctx_reg(get_sysregs_ctx(ctx), CTX_CPACR_EL1,
			CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_ALL));

	/*
	 * Prepare information in buffer shared between EL3 and S-EL0
	 * ----------------------------------------------------------
	 */

	void *shared_buf_ptr = (void *) PLAT_SPM_BUF_BASE;

	/* Copy the boot information into the shared buffer with the SP. */
	assert((uintptr_t)shared_buf_ptr + sizeof(secure_partition_boot_info_t)
	       <= (PLAT_SPM_BUF_BASE + PLAT_SPM_BUF_SIZE));

	assert(PLAT_SPM_BUF_BASE <= (UINTPTR_MAX - PLAT_SPM_BUF_SIZE + 1));

	const secure_partition_boot_info_t *sp_boot_info =
			plat_get_secure_partition_boot_info(NULL);

	assert(sp_boot_info != NULL);

	memcpy((void *) shared_buf_ptr, (const void *) sp_boot_info,
	       sizeof(secure_partition_boot_info_t));

	/* Pointer to the MP information from the platform port. */
	secure_partition_mp_info_t *sp_mp_info =
		((secure_partition_boot_info_t *) shared_buf_ptr)->mp_info;

	assert(sp_mp_info != NULL);

	/*
	 * Point the shared buffer MP information pointer to where the info will
	 * be populated, just after the boot info.
	 */
	((secure_partition_boot_info_t *) shared_buf_ptr)->mp_info =
		(secure_partition_mp_info_t *) ((uintptr_t)shared_buf_ptr
				+ sizeof(secure_partition_boot_info_t));

	/*
	 * Update the shared buffer pointer to where the MP information for the
	 * payload will be populated
	 */
	shared_buf_ptr = ((secure_partition_boot_info_t *) shared_buf_ptr)->mp_info;

	/*
	 * Copy the cpu information into the shared buffer area after the boot
	 * information.
	 */
	assert(sp_boot_info->num_cpus <= PLATFORM_CORE_COUNT);

	assert((uintptr_t)shared_buf_ptr
	       <= (PLAT_SPM_BUF_BASE + PLAT_SPM_BUF_SIZE -
		       (sp_boot_info->num_cpus * sizeof(*sp_mp_info))));

	memcpy(shared_buf_ptr, (const void *) sp_mp_info,
		sp_boot_info->num_cpus * sizeof(*sp_mp_info));

	/*
	 * Calculate the linear indices of cores in boot information for the
	 * secure partition and flag the primary CPU
	 */
	sp_mp_info = (secure_partition_mp_info_t *) shared_buf_ptr;

	for (unsigned int index = 0; index < sp_boot_info->num_cpus; index++) {
		u_register_t mpidr = sp_mp_info[index].mpidr;

		sp_mp_info[index].linear_id = plat_core_pos_by_mpidr(mpidr);
		if (plat_my_core_pos() == sp_mp_info[index].linear_id)
			sp_mp_info[index].flags |= MP_INFO_FLAG_PRIMARY_CPU;
	}

	VERBOSE("S-EL1/S-EL0 context setup end.\n");
}

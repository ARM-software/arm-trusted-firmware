/*
 * Copyright (c) 2017-2024, Arm Limited and Contributors. All rights reserved.
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
#if HOB_LIST
#include <lib/hob/hob.h>
#include <lib/hob/hob_guid.h>
#include <lib/hob/mmram.h>
#include <lib/hob/mpinfo.h>
#endif
#if TRANSFER_LIST
#include <transfer_list.h>
#endif
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <services/spm_mm_partition.h>

#include "spm_common.h"
#include "spm_mm_private.h"
#include "spm_shim_private.h"

#if HOB_LIST && TRANSFER_LIST
static struct efi_hob_handoff_info_table *build_sp_boot_hob_list(
		const spm_mm_boot_info_t *sp_boot_info, uint16_t *hob_table_size)
{
	int ret;
	struct efi_hob_handoff_info_table *hob_table;
	struct efi_guid ns_buf_guid = MM_NS_BUFFER_GUID;
	struct efi_guid mmram_resv_guid = MM_PEI_MMRAM_MEMORY_RESERVE_GUID;
	struct efi_mmram_descriptor *mmram_desc_data;
	uint16_t mmram_resv_data_size;
	struct efi_mmram_hob_descriptor_block *mmram_hob_desc_data;
	uint64_t hob_table_offset;

	hob_table_offset = sizeof(struct transfer_list_header) +
		sizeof(struct transfer_list_entry);

	*hob_table_size = 0U;

	hob_table = create_hob_list(sp_boot_info->sp_mem_base,
			sp_boot_info->sp_mem_limit - sp_boot_info->sp_mem_base,
			sp_boot_info->sp_shared_buf_base + hob_table_offset,
			sp_boot_info->sp_shared_buf_size);
	if (hob_table == NULL) {
		return NULL;
	}

	ret = create_fv_hob(hob_table, sp_boot_info->sp_image_base,
			sp_boot_info->sp_image_size);
	if (ret) {
		return NULL;
	}

	ret = create_guid_hob(hob_table, &ns_buf_guid,
			sizeof(struct efi_mmram_descriptor), (void **) &mmram_desc_data);
	if (ret) {
		return NULL;
	}

	mmram_desc_data->physical_start = sp_boot_info->sp_ns_comm_buf_base;
	mmram_desc_data->physical_size = sp_boot_info->sp_ns_comm_buf_size;
	mmram_desc_data->cpu_start = sp_boot_info->sp_ns_comm_buf_base;
	mmram_desc_data->region_state = EFI_CACHEABLE | EFI_ALLOCATED;

	mmram_resv_data_size = sizeof(struct efi_mmram_hob_descriptor_block) +
		sizeof(struct efi_mmram_descriptor) * sp_boot_info->num_sp_mem_regions;

	ret = create_guid_hob(hob_table, &mmram_resv_guid,
			mmram_resv_data_size, (void **) &mmram_hob_desc_data);
	if (ret) {
		return NULL;
	}

	*hob_table_size = hob_table->efi_free_memory_bottom -
		(efi_physical_address_t) hob_table;

	mmram_hob_desc_data->number_of_mm_reserved_regions = 4U;
	mmram_desc_data = &mmram_hob_desc_data->descriptor[0];

	/* First, should be image mm range. */
	mmram_desc_data[0].physical_start = sp_boot_info->sp_image_base;
	mmram_desc_data[0].physical_size = sp_boot_info->sp_image_size;
	mmram_desc_data[0].cpu_start = sp_boot_info->sp_image_base;
	mmram_desc_data[0].region_state = EFI_CACHEABLE | EFI_ALLOCATED;

	/* Second, should be shared buffer mm range. */
	mmram_desc_data[1].physical_start = sp_boot_info->sp_shared_buf_base;
	mmram_desc_data[1].physical_size = sp_boot_info->sp_shared_buf_size;
	mmram_desc_data[1].cpu_start = sp_boot_info->sp_shared_buf_base;
	mmram_desc_data[1].region_state = EFI_CACHEABLE | EFI_ALLOCATED;

	/* Ns Buffer mm range */
	mmram_desc_data[2].physical_start = sp_boot_info->sp_ns_comm_buf_base;
	mmram_desc_data[2].physical_size = sp_boot_info->sp_ns_comm_buf_size;
	mmram_desc_data[2].cpu_start = sp_boot_info->sp_ns_comm_buf_base;
	mmram_desc_data[2].region_state = EFI_CACHEABLE | EFI_ALLOCATED;

	/* Heap mm range */
	mmram_desc_data[3].physical_start = sp_boot_info->sp_heap_base;
	mmram_desc_data[3].physical_size = sp_boot_info->sp_heap_size;
	mmram_desc_data[3].cpu_start = sp_boot_info->sp_heap_base;
	mmram_desc_data[3].region_state = EFI_CACHEABLE;

	return hob_table;
}
#endif

/* Setup context of the Secure Partition */
void spm_sp_setup(sp_context_t *sp_ctx)
{
	cpu_context_t *ctx = &(sp_ctx->cpu_ctx);
	u_register_t sctlr_el1_val;
	/* Pointer to the MP information from the platform port. */
	const spm_mm_boot_info_t *sp_boot_info =
			plat_get_secure_partition_boot_info(NULL);

#if HOB_LIST && TRANSFER_LIST
	struct efi_hob_handoff_info_table *hob_table;
	struct transfer_list_header *sp_boot_tl;
	struct transfer_list_entry *sp_boot_te;
	uint16_t hob_table_size;
#endif

	assert(sp_boot_info != NULL);

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

	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), mair_el1,
		      mmu_cfg_params[MMU_CFG_MAIR]);
	write_ctx_tcr_el1_reg_errata(ctx, mmu_cfg_params[MMU_CFG_TCR]);

	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), ttbr0_el1,
		      mmu_cfg_params[MMU_CFG_TTBR0]);

	/* Setup SCTLR_EL1 */
	sctlr_el1_val = read_ctx_sctlr_el1_reg_errata(ctx);

	sctlr_el1_val |=
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

	sctlr_el1_val &= ~(
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

	/* Store the initialised SCTLR_EL1 value in the cpu_context */
	write_ctx_sctlr_el1_reg_errata(ctx, sctlr_el1_val);

	/*
	 * Setup other system registers
	 * ----------------------------
	 */

	/* Shim Exception Vector Base Address */
	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), vbar_el1,
			SPM_SHIM_EXCEPTIONS_PTR);

	write_el1_ctx_arch_timer(get_el1_sysregs_ctx(ctx), cntkctl_el1,
		      EL0PTEN_BIT | EL0VTEN_BIT | EL0PCTEN_BIT | EL0VCTEN_BIT);

	/*
	 * FPEN: Allow the Secure Partition to access FP/SIMD registers.
	 * Note that SPM will not do any saving/restoring of these registers on
	 * behalf of the SP. This falls under the SP's responsibility.
	 * TTA: Enable access to trace registers.
	 * ZEN (v8.2): Trap SVE instructions and access to SVE registers.
	 */
	write_el1_ctx_common(get_el1_sysregs_ctx(ctx), cpacr_el1,
			CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_NONE));

	/*
	 * Prepare information in buffer shared between EL3 and S-EL0
	 * ----------------------------------------------------------
	 */
#if HOB_LIST && TRANSFER_LIST
	sp_boot_tl = transfer_list_init((void *) sp_boot_info->sp_shared_buf_base,
			sp_boot_info->sp_shared_buf_size);
	assert(sp_boot_tl != NULL);

	hob_table = build_sp_boot_hob_list(sp_boot_info, &hob_table_size);
	assert(hob_table != NULL);

	transfer_list_update_checksum(sp_boot_tl);

	sp_boot_te = transfer_list_add(sp_boot_tl, TL_TAG_HOB_LIST,
			hob_table_size, hob_table);
	if (sp_boot_te == NULL) {
		ERROR("Failed to add HOB list to xfer list\n");
	}

	transfer_list_set_handoff_args(sp_boot_tl, &ep_info);

	transfer_list_dump(sp_boot_tl);

	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X0,
			ep_info.args.arg0);
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X1,
			ep_info.args.arg1);
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X2,
			ep_info.args.arg2);
	write_ctx_reg(get_gpregs_ctx(ctx), CTX_GPREG_X3,
			ep_info.args.arg3);
#else
	void *shared_buf_ptr = (void *) sp_boot_info->sp_shared_buf_base;

	/* Copy the boot information into the shared buffer with the SP. */
	assert((uintptr_t)shared_buf_ptr + sizeof(spm_mm_boot_info_t)
	       <= (sp_boot_info->sp_shared_buf_base + sp_boot_info->sp_shared_buf_size));

	assert(sp_boot_info->sp_shared_buf_base <=
				(UINTPTR_MAX - sp_boot_info->sp_shared_buf_size + 1));


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
#endif
}

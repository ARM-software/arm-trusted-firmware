/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <string.h>

#include <arch.h>
#include <arch_helpers.h>
#include <common/debug.h>
#include <common/fdt_wrappers.h>
#include <context.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <libfdt.h>
#include <plat/common/common_def.h>
#include <plat/common/platform.h>
#include <services/ffa_svc.h>
#include "spm_common.h"
#include "spm_shim_private.h"
#include "spmc.h"
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>

/*
 * Statically allocate a page of memory for passing boot information to an SP.
 */
static uint8_t ffa_boot_info_mem[PAGE_SIZE] __aligned(PAGE_SIZE);

/*
 * We need to choose one execution context from all those available for a S-EL0
 * SP. This execution context will be used subsequently irrespective of which
 * physical CPU the SP runs on.
 */
#define SEL0_SP_EC_INDEX 0

/*
 * This function creates a initialization descriptor in the memory reserved
 * for passing boot information to an SP. It then copies the partition manifest
 * into this region and ensures that its reference in the initialization
 * descriptor is updated.
 */
static void spmc_create_boot_info(entry_point_info_t *ep_info,
				  struct secure_partition_desc *sp)
{
	struct ffa_boot_info_header *boot_header;
	struct ffa_boot_info_desc *boot_descriptor;
	uintptr_t manifest_addr;

	/*
	 * Calculate the maximum size of the manifest that can be accommodated
	 * in the boot information memory region.
	 */
	const unsigned int
	max_manifest_sz = sizeof(ffa_boot_info_mem) -
			  (sizeof(struct ffa_boot_info_header) +
			   sizeof(struct ffa_boot_info_desc));

	/*
	 * The current implementation only supports the FF-A v1.1
	 * implementation of the boot protocol, therefore check
	 * that a v1.0 SP has not requested use of the protocol.
	 */
	if (sp->ffa_version == MAKE_FFA_VERSION(1, 0)) {
		ERROR("FF-A boot protocol not supported for v1.0 clients\n");
		return;
	}

	/*
	 * Check if the manifest will fit into the boot info memory region else
	 * bail.
	 */
	if (ep_info->args.arg1 > max_manifest_sz) {
		WARN("Unable to copy manifest into boot information. ");
		WARN("Max sz = %u bytes. Manifest sz = %lu bytes\n",
		     max_manifest_sz, ep_info->args.arg1);
		return;
	}

	/* Zero the memory region before populating. */
	memset(ffa_boot_info_mem, 0, PAGE_SIZE);

	/*
	 * Populate the ffa_boot_info_header at the start of the boot info
	 * region.
	 */
	boot_header = (struct ffa_boot_info_header *) ffa_boot_info_mem;

	/* Position the ffa_boot_info_desc after the ffa_boot_info_header. */
	boot_header->offset_boot_info_desc =
					sizeof(struct ffa_boot_info_header);
	boot_descriptor = (struct ffa_boot_info_desc *)
			  (ffa_boot_info_mem +
			   boot_header->offset_boot_info_desc);

	/*
	 * We must use the FF-A version corresponding to the version implemented
	 * by the SP. Currently this can only be v1.1.
	 */
	boot_header->version = sp->ffa_version;

	/* Populate the boot information header. */
	boot_header->size_boot_info_desc = sizeof(struct ffa_boot_info_desc);

	/* Set the signature "0xFFA". */
	boot_header->signature = FFA_INIT_DESC_SIGNATURE;

	/* Set the count. Currently 1 since only the manifest is specified. */
	boot_header->count_boot_info_desc = 1;

	/* Populate the boot information descriptor for the manifest. */
	boot_descriptor->type =
		FFA_BOOT_INFO_TYPE(FFA_BOOT_INFO_TYPE_STD) |
		FFA_BOOT_INFO_TYPE_ID(FFA_BOOT_INFO_TYPE_ID_FDT);

	boot_descriptor->flags =
		FFA_BOOT_INFO_FLAG_NAME(FFA_BOOT_INFO_FLAG_NAME_UUID) |
		FFA_BOOT_INFO_FLAG_CONTENT(FFA_BOOT_INFO_FLAG_CONTENT_ADR);

	/*
	 * Copy the manifest into boot info region after the boot information
	 * descriptor.
	 */
	boot_descriptor->size_boot_info = (uint32_t) ep_info->args.arg1;

	manifest_addr = (uintptr_t) (ffa_boot_info_mem +
				     boot_header->offset_boot_info_desc +
				     boot_header->size_boot_info_desc);

	memcpy((void *) manifest_addr, (void *) ep_info->args.arg0,
	       boot_descriptor->size_boot_info);

	boot_descriptor->content = manifest_addr;

	/* Calculate the size of the total boot info blob. */
	boot_header->size_boot_info_blob = boot_header->offset_boot_info_desc +
					   boot_descriptor->size_boot_info +
					   (boot_header->count_boot_info_desc *
					    boot_header->size_boot_info_desc);

	INFO("SP boot info @ 0x%lx, size: %u bytes.\n",
	     (uintptr_t) ffa_boot_info_mem,
	     boot_header->size_boot_info_blob);
	INFO("SP manifest @ 0x%lx, size: %u bytes.\n",
	     boot_descriptor->content,
	     boot_descriptor->size_boot_info);
}

/*
 * S-EL1 partitions can be assigned with multiple execution contexts, each
 * pinned to the physical CPU. Each execution context index corresponds to the
 * respective liner core position.
 * S-EL0 partitions execute in a single execution context (index 0).
 */
unsigned int get_ec_index(struct secure_partition_desc *sp)
{
	return (sp->runtime_el == S_EL0) ?
		SEL0_SP_EC_INDEX : plat_my_core_pos();
}

#if SPMC_AT_EL3_SEL0_SP
/* Setup spsr in entry point info for common context management code to use. */
void spmc_el0_sp_spsr_setup(entry_point_info_t *ep_info)
{
	/* Setup Secure Partition SPSR for S-EL0 SP. */
	ep_info->spsr = SPSR_64(MODE_EL0, MODE_SP_EL0, DISABLE_ALL_EXCEPTIONS);
}

/* Setup context of an EL0 Secure Partition.  */
void spmc_el0_sp_setup(struct secure_partition_desc *sp,
		       int32_t boot_info_reg)
{
	mmap_region_t sel1_exception_vectors =
		MAP_REGION_FLAT(SPM_SHIM_EXCEPTIONS_START,
				SPM_SHIM_EXCEPTIONS_SIZE,
				MT_CODE | MT_SECURE | MT_PRIVILEGED);
	cpu_context_t *ctx;

	ctx = &sp->ec[SEL0_SP_EC_INDEX].cpu_ctx;

	sp->xlat_ctx_handle->xlat_regime = EL1_EL0_REGIME;

	/* This region contains the exception vectors used at S-EL1. */
	mmap_add_region_ctx(sp->xlat_ctx_handle,
			    &sel1_exception_vectors);

	/*
	 * If the SP manifest specified the register to pass the address of the
	 * boot information, then map the memory region to pass boot
	 * information.
	 */
	if (boot_info_reg >= 0) {
		mmap_region_t ffa_boot_info_region = MAP_REGION_FLAT(
			(uintptr_t) ffa_boot_info_mem,
			PAGE_SIZE,
			MT_RO_DATA | MT_SECURE | MT_USER);
		mmap_add_region_ctx(sp->xlat_ctx_handle, &ffa_boot_info_region);
	}

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

	/* Setup other system registers. */

	/* Shim Exception Vector Base Address */
	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_VBAR_EL1,
			SPM_SHIM_EXCEPTIONS_PTR);
#if NS_TIMER_SWITCH
	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_CNTKCTL_EL1,
		      EL0PTEN_BIT | EL0VTEN_BIT | EL0PCTEN_BIT | EL0VCTEN_BIT);
#endif

	/*
	 * FPEN: Allow the Secure Partition to access FP/SIMD registers.
	 * Note that SPM will not do any saving/restoring of these registers on
	 * behalf of the SP. This falls under the SP's responsibility.
	 * TTA: Enable access to trace registers.
	 * ZEN (v8.2): Trap SVE instructions and access to SVE registers.
	 */
	write_ctx_reg(get_el1_sysregs_ctx(ctx), CTX_CPACR_EL1,
			CPACR_EL1_FPEN(CPACR_EL1_FP_TRAP_NONE));
}
#endif /* SPMC_AT_EL3_SEL0_SP */

/* S-EL1 partition specific initialisation. */
void spmc_el1_sp_setup(struct secure_partition_desc *sp,
		       entry_point_info_t *ep_info)
{
	/* Sanity check input arguments. */
	assert(sp != NULL);
	assert(ep_info != NULL);

	/* Initialise the SPSR for S-EL1 SPs. */
	ep_info->spsr =	SPSR_64(MODE_EL1, MODE_SP_ELX,
				DISABLE_ALL_EXCEPTIONS);

	/*
	 * TF-A Implementation defined behaviour to provide the linear
	 * core ID in the x4 register.
	 */
	ep_info->args.arg4 = (uintptr_t) plat_my_core_pos();

	/*
	 * Check whether setup is being performed for the primary or a secondary
	 * execution context. In the latter case, indicate to the SP that this
	 * is a warm boot.
	 * TODO: This check would need to be reworked if the same entry point is
	 * used for both primary and secondary initialisation.
	 */
	if (sp->secondary_ep != 0U) {
		/*
		 * Sanity check that the secondary entry point is still what was
		 * originally set.
		 */
		assert(sp->secondary_ep == ep_info->pc);
		ep_info->args.arg0 = FFA_WB_TYPE_S2RAM;
	}
}

/* Common initialisation for all SPs. */
void spmc_sp_common_setup(struct secure_partition_desc *sp,
			  entry_point_info_t *ep_info,
			  int32_t boot_info_reg)
{
	uint16_t sp_id;

	/* Assign FF-A Partition ID if not already assigned. */
	if (sp->sp_id == INV_SP_ID) {
		sp_id = FFA_SP_ID_BASE + ACTIVE_SP_DESC_INDEX;
		/*
		 * Ensure we don't clash with previously assigned partition
		 * IDs.
		 */
		while (!is_ffa_secure_id_valid(sp_id)) {
			sp_id++;

			if (sp_id == FFA_SWD_ID_LIMIT) {
				ERROR("Unable to determine valid SP ID.\n");
				panic();
			}
		}
		sp->sp_id = sp_id;
	}

	/* Check if the SP wants to use the FF-A boot protocol. */
	if (boot_info_reg >= 0) {
		/*
		 * Create a boot information descriptor and copy the partition
		 * manifest into the reserved memory region for consumption by
		 * the SP.
		 */
		spmc_create_boot_info(ep_info, sp);

		/*
		 * We have consumed what we need from ep args so we can now
		 * zero them before we start populating with new information
		 * specifically for the SP.
		 */
		zeromem(&ep_info->args, sizeof(ep_info->args));

		/*
		 * Pass the address of the boot information in the
		 * boot_info_reg.
		 */
		switch (boot_info_reg) {
		case 0:
			ep_info->args.arg0 = (uintptr_t) ffa_boot_info_mem;
			break;
		case 1:
			ep_info->args.arg1 = (uintptr_t) ffa_boot_info_mem;
			break;
		case 2:
			ep_info->args.arg2 = (uintptr_t) ffa_boot_info_mem;
			break;
		case 3:
			ep_info->args.arg3 = (uintptr_t) ffa_boot_info_mem;
			break;
		default:
			ERROR("Invalid value for \"gp-register-num\" %d.\n",
			      boot_info_reg);
		}
	} else {
		/*
		 * We don't need any of the information that was populated
		 * in ep_args so we can clear them.
		 */
		zeromem(&ep_info->args, sizeof(ep_info->args));
	}
}

/*
 * Initialise the SP context now we have populated the common and EL specific
 * entrypoint information.
 */
void spmc_sp_common_ep_commit(struct secure_partition_desc *sp,
			      entry_point_info_t *ep_info)
{
	cpu_context_t *cpu_ctx;

	cpu_ctx = &(spmc_get_sp_ec(sp)->cpu_ctx);
	print_entry_point_info(ep_info);
	cm_setup_context(cpu_ctx, ep_info);
}

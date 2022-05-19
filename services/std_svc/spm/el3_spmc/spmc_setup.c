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
#include "spmc.h"
#include <tools_share/firmware_image_package.h>

#include <platform_def.h>

/*
 * Statically allocate a page of memory for passing boot information to an SP.
 */
static uint8_t ffa_boot_info_mem[PAGE_SIZE] __aligned(PAGE_SIZE);

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
	 * We must use the FF-A version coresponding to the version implemented
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
 * We are assuming that the index of the execution
 * context used is the linear index of the current physical cpu.
 */
unsigned int get_ec_index(struct secure_partition_desc *sp)
{
	return plat_my_core_pos();
}

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

	/*
	 * We currently only support S-EL1 partitions so ensure this is the
	 * case.
	 */
	assert(sp->runtime_el == S_EL1);

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

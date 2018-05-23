/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <mm_svc.h>
#include <platform.h>
#include <runtime_svc.h>
#include <secure_partition.h>
#include <smccc.h>
#include <smccc_helpers.h>
#include <spinlock.h>
#include <spm_svc.h>
#include <utils.h>
#include <xlat_tables_v2.h>

#include "spm_private.h"
#include "spm_shim_private.h"

/* Place translation tables by default along with the ones used by BL31. */
#ifndef PLAT_SP_IMAGE_XLAT_SECTION_NAME
#define PLAT_SP_IMAGE_XLAT_SECTION_NAME	"xlat_table"
#endif

/* Allocate and initialise the translation context for the secure partitions. */
REGISTER_XLAT_CONTEXT2(sp,
		       PLAT_SP_IMAGE_MMAP_REGIONS,
		       PLAT_SP_IMAGE_MAX_XLAT_TABLES,
		       PLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE,
		       EL1_EL0_REGIME, PLAT_SP_IMAGE_XLAT_SECTION_NAME);

/* Lock used for SP_MEMORY_ATTRIBUTES_GET and SP_MEMORY_ATTRIBUTES_SET */
static spinlock_t mem_attr_smc_lock;

/* Get handle of Secure Partition translation context */
xlat_ctx_t *spm_get_sp_xlat_context(void)
{
	return &sp_xlat_ctx;
};

/*******************************************************************************
 * Secure Partition context information.
 ******************************************************************************/
static secure_partition_context_t sp_ctx;

/*******************************************************************************
 * This function takes an SP context pointer and prepares the CPU to enter.
 ******************************************************************************/
static void spm_sp_prepare_enter(secure_partition_context_t *sp_ctx)
{
	assert(sp_ctx != NULL);

	/* Assign the context of the SP to this CPU */
	cm_set_context(&(sp_ctx->cpu_ctx), SECURE);

	/* Restore the context assigned above */
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	/* Invalidate TLBs at EL1. */
	tlbivmalle1();
	dsbish();
}

/*******************************************************************************
 * Enter SP after preparing it with spm_sp_prepare_enter().
 ******************************************************************************/
static uint64_t spm_sp_enter(secure_partition_context_t *sp_ctx)
{
	/* Enter Secure Partition */
	return spm_secure_partition_enter(&sp_ctx->c_rt_ctx);
}

/*******************************************************************************
 * Jump to each Secure Partition for the first time.
 ******************************************************************************/
static int32_t spm_init(void)
{
	uint64_t rc = 0;
	secure_partition_context_t *ctx;

	INFO("Secure Partition init...\n");

	ctx = &sp_ctx;

	ctx->sp_init_in_progress = 1;

	spm_sp_prepare_enter(ctx);
	rc |= spm_sp_enter(ctx);
	assert(rc == 0);

	ctx->sp_init_in_progress = 0;

	INFO("Secure Partition initialized.\n");

	return rc;
}

/*******************************************************************************
 * Initialize contexts of all Secure Partitions.
 ******************************************************************************/
int32_t spm_setup(void)
{
	secure_partition_context_t *ctx;

	/* Disable MMU at EL1 (initialized by BL2) */
	disable_mmu_icache_el1();

	/* Initialize context of the SP */
	INFO("Secure Partition context setup start...\n");

	ctx = &sp_ctx;

	/* Assign translation tables context. */
	ctx->xlat_ctx_handle = spm_get_sp_xlat_context();

	secure_partition_setup(ctx);

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&spm_init);

	INFO("Secure Partition setup done.\n");

	return 0;
}

/*
 * Attributes are encoded using a different format in the SMC interface than in
 * the Trusted Firmware, where the mmap_attr_t enum type is used. This function
 * converts an attributes value from the SMC format to the mmap_attr_t format by
 * setting MT_RW/MT_RO, MT_USER/MT_PRIVILEGED and MT_EXECUTE/MT_EXECUTE_NEVER.
 * The other fields are left as 0 because they are ignored by the function
 * change_mem_attributes().
 */
static unsigned int smc_attr_to_mmap_attr(unsigned int attributes)
{
	unsigned int tf_attr = 0U;

	unsigned int access = (attributes & SP_MEMORY_ATTRIBUTES_ACCESS_MASK)
			      >> SP_MEMORY_ATTRIBUTES_ACCESS_SHIFT;

	if (access == SP_MEMORY_ATTRIBUTES_ACCESS_RW) {
		tf_attr |= MT_RW | MT_USER;
	} else if (access ==  SP_MEMORY_ATTRIBUTES_ACCESS_RO) {
		tf_attr |= MT_RO | MT_USER;
	} else {
		/* Other values are reserved. */
		assert(access ==  SP_MEMORY_ATTRIBUTES_ACCESS_NOACCESS);
		/* The only requirement is that there's no access from EL0 */
		tf_attr |= MT_RO | MT_PRIVILEGED;
	}

	if ((attributes & SP_MEMORY_ATTRIBUTES_NON_EXEC) == 0) {
		tf_attr |= MT_EXECUTE;
	} else {
		tf_attr |= MT_EXECUTE_NEVER;
	}

	return tf_attr;
}

/*
 * This function converts attributes from the Trusted Firmware format into the
 * SMC interface format.
 */
static unsigned int smc_mmap_to_smc_attr(unsigned int attr)
{
	unsigned int smc_attr = 0U;

	unsigned int data_access;

	if ((attr & MT_USER) == 0) {
		/* No access from EL0. */
		data_access = SP_MEMORY_ATTRIBUTES_ACCESS_NOACCESS;
	} else {
		if ((attr & MT_RW) != 0) {
			assert(MT_TYPE(attr) != MT_DEVICE);
			data_access = SP_MEMORY_ATTRIBUTES_ACCESS_RW;
		} else {
			data_access = SP_MEMORY_ATTRIBUTES_ACCESS_RO;
		}
	}

	smc_attr |= (data_access & SP_MEMORY_ATTRIBUTES_ACCESS_MASK)
		    << SP_MEMORY_ATTRIBUTES_ACCESS_SHIFT;

	if ((attr & MT_EXECUTE_NEVER) != 0U) {
		smc_attr |= SP_MEMORY_ATTRIBUTES_NON_EXEC;
	}

	return smc_attr;
}

static int32_t spm_memory_attributes_get_smc_handler(
					secure_partition_context_t *sp_ctx,
					uintptr_t base_va)
{
	uint32_t attributes;

	spin_lock(&mem_attr_smc_lock);

	int rc = get_mem_attributes(sp_ctx->xlat_ctx_handle,
				     base_va, &attributes);

	spin_unlock(&mem_attr_smc_lock);

	/* Convert error codes of get_mem_attributes() into SPM ones. */
	assert((rc == 0) || (rc == -EINVAL));

	if (rc == 0) {
		return (int32_t) smc_mmap_to_smc_attr(attributes);
	} else {
		return SPM_INVALID_PARAMETER;
	}
}

static int spm_memory_attributes_set_smc_handler(
					secure_partition_context_t *sp_ctx,
					u_register_t page_address,
					u_register_t pages_count,
					u_register_t smc_attributes)
{
	uintptr_t base_va = (uintptr_t) page_address;
	size_t size = (size_t) (pages_count * PAGE_SIZE);
	uint32_t attributes = (uint32_t) smc_attributes;

	INFO("  Start address  : 0x%lx\n", base_va);
	INFO("  Number of pages: %i (%zi bytes)\n", (int) pages_count, size);
	INFO("  Attributes     : 0x%x\n", attributes);

	spin_lock(&mem_attr_smc_lock);

	int ret = change_mem_attributes(sp_ctx->xlat_ctx_handle,
					base_va, size,
					smc_attr_to_mmap_attr(attributes));

	spin_unlock(&mem_attr_smc_lock);

	/* Convert error codes of change_mem_attributes() into SPM ones. */
	assert((ret == 0) || (ret == -EINVAL));

	return (ret == 0) ? SPM_SUCCESS : SPM_INVALID_PARAMETER;
}

/*******************************************************************************
 * Secure Partition Manager SMC handler.
 ******************************************************************************/
uint64_t spm_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	cpu_context_t *ns_cpu_context;
	unsigned int ns;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	if (ns == SMC_FROM_SECURE) {

		/* Handle SMCs from Secure world. */

		assert(handle == cm_get_context(SECURE));

		/* Make next ERET jump to S-EL0 instead of S-EL1. */
		cm_set_elr_spsr_el3(SECURE, read_elr_el1(), read_spsr_el1());

		switch (smc_fid) {

		case SPM_VERSION_AARCH32:
			SMC_RET1(handle, SPM_VERSION_COMPILED);

		case SP_EVENT_COMPLETE_AARCH64:
			/* Save secure state */
			cm_el1_sysregs_context_save(SECURE);

			if (sp_ctx.sp_init_in_progress == 1) {
				/*
				 * SPM reports completion. The SPM must have
				 * initiated the original request through a
				 * synchronous entry into the secure
				 * partition. Jump back to the original C
				 * runtime context.
				 */
				spm_secure_partition_exit(sp_ctx.c_rt_ctx, x1);

				/* spm_secure_partition_exit doesn't return */
			}

			/* Release the Secure Partition context */
			spin_unlock(&(sp_ctx.lock));

			/*
			 * This is the result from the Secure partition of an
			 * earlier request. Copy the result into the non-secure
			 * context and return to the non-secure state.
			 */

			/* Get a reference to the non-secure context */
			ns_cpu_context = cm_get_context(NON_SECURE);
			assert(ns_cpu_context != NULL);

			/* Restore non-secure state */
			cm_el1_sysregs_context_restore(NON_SECURE);
			cm_set_next_eret_context(NON_SECURE);

			/* Return to normal world */
			SMC_RET1(ns_cpu_context, x1);

		case SP_MEMORY_ATTRIBUTES_GET_AARCH64:
			INFO("Received SP_MEMORY_ATTRIBUTES_GET_AARCH64 SMC\n");

			if (sp_ctx.sp_init_in_progress == 0) {
				WARN("SP_MEMORY_ATTRIBUTES_GET_AARCH64 is available at boot time only\n");
				SMC_RET1(handle, SPM_NOT_SUPPORTED);
			}
			SMC_RET1(handle,
				 spm_memory_attributes_get_smc_handler(
					 &sp_ctx, x1));

		case SP_MEMORY_ATTRIBUTES_SET_AARCH64:
			INFO("Received SP_MEMORY_ATTRIBUTES_SET_AARCH64 SMC\n");

			if (sp_ctx.sp_init_in_progress == 0) {
				WARN("SP_MEMORY_ATTRIBUTES_SET_AARCH64 is available at boot time only\n");
				SMC_RET1(handle, SPM_NOT_SUPPORTED);
			}
			SMC_RET1(handle,
				 spm_memory_attributes_set_smc_handler(
					&sp_ctx, x1, x2, x3));
		default:
			break;
		}
	} else {

		/* Handle SMCs from Non-secure world. */

		switch (smc_fid) {

		case MM_VERSION_AARCH32:
			SMC_RET1(handle, MM_VERSION_COMPILED);

		case MM_COMMUNICATE_AARCH32:
		case MM_COMMUNICATE_AARCH64:
		{
			uint64_t mm_cookie = x1;
			uint64_t comm_buffer_address = x2;
			uint64_t comm_size_address = x3;

			/* Cookie. Reserved for future use. It must be zero. */
			if (mm_cookie != 0U) {
				ERROR("MM_COMMUNICATE: cookie is not zero\n");
				SMC_RET1(handle, SPM_INVALID_PARAMETER);
			}

			if (comm_buffer_address == 0U) {
				ERROR("MM_COMMUNICATE: comm_buffer_address is zero\n");
				SMC_RET1(handle, SPM_INVALID_PARAMETER);
			}

			if (comm_size_address != 0U) {
				VERBOSE("MM_COMMUNICATE: comm_size_address is not 0 as recommended.\n");
			}

			/* Save the Normal world context */
			cm_el1_sysregs_context_save(NON_SECURE);

			/* Lock the Secure Partition context. */
			spin_lock(&sp_ctx.lock);

			/* Jump to the Secure Partition. */

			spm_sp_prepare_enter(&sp_ctx);

			SMC_RET4(&(sp_ctx.cpu_ctx), smc_fid,
				 comm_buffer_address, comm_size_address,
				 plat_my_core_pos());
		}

		case SP_MEMORY_ATTRIBUTES_GET_AARCH64:
		case SP_MEMORY_ATTRIBUTES_SET_AARCH64:
			/* SMC interfaces reserved for secure callers. */
			SMC_RET1(handle, SPM_NOT_SUPPORTED);

		default:
			break;
		}
	}

	SMC_RET1(handle, SMC_UNK);
}

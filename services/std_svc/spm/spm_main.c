/*
 * Copyright (c) 2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <bl31.h>
#include <context_mgmt.h>
#include <debug.h>
#include <errno.h>
#include <platform.h>
#include <runtime_svc.h>
#include <secure_partition.h>
#include <smcc.h>
#include <smcc_helpers.h>
#include <spinlock.h>
#include <spm_svc.h>
#include <utils.h>
#include <xlat_tables_v2.h>

#include "spm_private.h"

/* Lock used for SP_MEMORY_ATTRIBUTES_GET and SP_MEMORY_ATTRIBUTES_SET */
static spinlock_t mem_attr_smc_lock;

/*******************************************************************************
 * Secure Partition context information.
 ******************************************************************************/
static secure_partition_context_t sp_ctx;
unsigned int sp_init_in_progress;

/*******************************************************************************
 * Replace the S-EL1 re-entry information with S-EL0 re-entry
 * information
 ******************************************************************************/
void spm_setup_next_eret_into_sel0(cpu_context_t *secure_context)
{
	assert(secure_context == cm_get_context(SECURE));

	cm_set_elr_spsr_el3(SECURE, read_elr_el1(), read_spsr_el1());
}

/*******************************************************************************
 * This function takes an SP context pointer and:
 * 1. Applies the S-EL1 system register context from sp_ctx->cpu_ctx.
 * 2. Saves the current C runtime state (callee-saved registers) on the stack
 *    frame and saves a reference to this state.
 * 3. Calls el3_exit() so that the EL3 system and general purpose registers
 *    from the sp_ctx->cpu_ctx are used to enter the secure payload image.
 ******************************************************************************/
static uint64_t spm_synchronous_sp_entry(secure_partition_context_t *sp_ctx_ptr)
{
	uint64_t rc;

	assert(sp_ctx_ptr != NULL);
	assert(sp_ctx_ptr->c_rt_ctx == 0);
	assert(cm_get_context(SECURE) == &sp_ctx_ptr->cpu_ctx);

	/* Apply the Secure EL1 system register context and switch to it */
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	VERBOSE("%s: We're about to enter the Secure partition...\n", __func__);

	rc = spm_secure_partition_enter(&sp_ctx_ptr->c_rt_ctx);
#if ENABLE_ASSERTIONS
	sp_ctx_ptr->c_rt_ctx = 0;
#endif

	return rc;
}


/*******************************************************************************
 * This function takes a Secure partition context pointer and:
 * 1. Saves the S-EL1 system register context tp sp_ctx->cpu_ctx.
 * 2. Restores the current C runtime state (callee saved registers) from the
 *    stack frame using the reference to this state saved in
 *    spm_secure_partition_enter().
 * 3. It does not need to save any general purpose or EL3 system register state
 *    as the generic smc entry routine should have saved those.
 ******************************************************************************/
static void __dead2 spm_synchronous_sp_exit(
			secure_partition_context_t *sp_ctx_ptr, uint64_t ret)
{
	assert(sp_ctx_ptr != NULL);
	/* Save the Secure EL1 system register context */
	assert(cm_get_context(SECURE) == &sp_ctx_ptr->cpu_ctx);
	cm_el1_sysregs_context_save(SECURE);

	assert(sp_ctx_ptr->c_rt_ctx != 0);
	spm_secure_partition_exit(sp_ctx_ptr->c_rt_ctx, ret);

	/* Should never reach here */
	assert(0);
}

/*******************************************************************************
 * This function passes control to the Secure Partition image (BL32) for the
 * first time on the primary cpu after a cold boot. It assumes that a valid
 * secure context has already been created by spm_setup() which can be directly
 * used. This function performs a synchronous entry into the Secure payload.
 * The SP passes control back to this routine through a SMC.
 ******************************************************************************/
int32_t spm_init(void)
{
	entry_point_info_t *secure_partition_ep_info;
	uint64_t rc;

	VERBOSE("%s entry\n", __func__);

	/*
	 * Get information about the Secure Partition (BL32) image. Its
	 * absence is a critical failure.
	 */
	secure_partition_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	assert(secure_partition_ep_info);

	/*
	 * Initialise the common context and then overlay the S-EL0 specific
	 * context on top of it.
	 */
	cm_init_my_context(secure_partition_ep_info);
	secure_partition_setup();

	/*
	 * Arrange for an entry into the secure payload.
	 */
	sp_init_in_progress = 1;
	rc = spm_synchronous_sp_entry(&sp_ctx);
	assert(rc == 0);
	sp_init_in_progress = 0;
	VERBOSE("SP_MEM_ATTRIBUTES_SET_AARCH64 availability has been revoked\n");

	return rc;
}

/*******************************************************************************
 * Given a secure payload entrypoint info pointer, entry point PC & pointer to
 * a context data structure, this function will initialize the SPM context and
 * entry point info for the secure payload
 ******************************************************************************/
void spm_init_sp_ep_state(struct entry_point_info *sp_ep_info,
			  uint64_t pc,
			  secure_partition_context_t *sp_ctx_ptr)
{
	uint32_t ep_attr;

	assert(sp_ep_info);
	assert(pc);
	assert(sp_ctx_ptr);

	cm_set_context(&sp_ctx_ptr->cpu_ctx, SECURE);

	/* initialise an entrypoint to set up the CPU context */
	ep_attr = SECURE | EP_ST_ENABLE;
	if (read_sctlr_el3() & SCTLR_EE_BIT)
		ep_attr |= EP_EE_BIG;
	SET_PARAM_HEAD(sp_ep_info, PARAM_EP, VERSION_1, ep_attr);

	sp_ep_info->pc = pc;
	/* The SPM payload runs in S-EL0 */
	sp_ep_info->spsr = SPSR_64(MODE_EL0,
				   MODE_SP_EL0,
				   DISABLE_ALL_EXCEPTIONS);

	zeromem(&sp_ep_info->args, sizeof(sp_ep_info->args));
}

/*******************************************************************************
 * Secure Partition Manager setup. The SPM finds out the SP entrypoint if not
 * already known and initialises the context for entry into the SP for its
 * initialisation.
 ******************************************************************************/
int32_t spm_setup(void)
{
	entry_point_info_t *secure_partition_ep_info;

	VERBOSE("%s entry\n", __func__);

	/*
	 * Get information about the Secure Partition (BL32) image. Its
	 * absence is a critical failure.
	 */
	secure_partition_ep_info = bl31_plat_get_next_image_ep_info(SECURE);
	if (!secure_partition_ep_info) {
		WARN("No SPM provided by BL2 boot loader, Booting device"
			" without SPM initialization. SMCs destined for SPM"
			" will return SMC_UNK\n");
		return 1;
	}

	/*
	 * If there's no valid entry point for SP, we return a non-zero value
	 * signalling failure initializing the service. We bail out without
	 * registering any handlers
	 */
	if (!secure_partition_ep_info->pc) {
		return 1;
	}

	spm_init_sp_ep_state(secure_partition_ep_info,
			      secure_partition_ep_info->pc,
			      &sp_ctx);

	/*
	 * All SPM initialization done. Now register our init function with
	 * BL31 for deferred invocation
	 */
	bl31_register_bl32_init(&spm_init);

	VERBOSE("%s exit\n", __func__);

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
static mmap_attr_t smc_attr_to_mmap_attr(unsigned int attributes)
{
	mmap_attr_t tf_attr = 0;

	unsigned int access = (attributes & SP_MEM_ATTR_ACCESS_MASK)
			      >> SP_MEM_ATTR_ACCESS_SHIFT;

	if (access == SP_MEM_ATTR_ACCESS_RW) {
		tf_attr |= MT_RW | MT_USER;
	} else if (access ==  SP_MEM_ATTR_ACCESS_RO) {
		tf_attr |= MT_RO | MT_USER;
	} else {
		/* Other values are reserved. */
		assert(access ==  SP_MEM_ATTR_ACCESS_NOACCESS);
		/* The only requirement is that there's no access from EL0 */
		tf_attr |= MT_RO | MT_PRIVILEGED;
	}

	if ((attributes & SP_MEM_ATTR_NON_EXEC) == 0) {
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
static int smc_mmap_to_smc_attr(mmap_attr_t attr)
{
	int smc_attr = 0;

	int data_access;

	if ((attr & MT_USER) == 0) {
		/* No access from EL0. */
		data_access = SP_MEM_ATTR_ACCESS_NOACCESS;
	} else {
		if ((attr & MT_RW) != 0) {
			assert(MT_TYPE(attr) != MT_DEVICE);
			data_access = SP_MEM_ATTR_ACCESS_RW;
		} else {
			data_access = SP_MEM_ATTR_ACCESS_RO;
		}
	}

	smc_attr |= (data_access & SP_MEM_ATTR_ACCESS_MASK) << SP_MEM_ATTR_ACCESS_SHIFT;

	if (attr & MT_EXECUTE_NEVER) {
		smc_attr |= SP_MEM_ATTR_NON_EXEC;
	}

	return smc_attr;
}

static int spm_memory_attributes_get_smc_handler(uintptr_t base_va)
{
	spin_lock(&mem_attr_smc_lock);

	mmap_attr_t attributes;
	int rc = get_mem_attributes(secure_partition_xlat_ctx_handle,
				     base_va, &attributes);

	spin_unlock(&mem_attr_smc_lock);

	/* Convert error codes of get_mem_attributes() into SPM ones. */
	assert(rc == 0 || rc == -EINVAL);

	if (rc == 0) {
		return smc_mmap_to_smc_attr(attributes);
	} else {
		return SPM_INVALID_PARAMETER;
	}
}

static int spm_memory_attributes_set_smc_handler(u_register_t page_address,
					u_register_t pages_count,
					u_register_t smc_attributes)
{
	uintptr_t base_va = (uintptr_t) page_address;
	size_t size = (size_t) (pages_count * PAGE_SIZE);
	unsigned int attributes = (unsigned int) smc_attributes;

	INFO("  Start address  : 0x%lx\n", base_va);
	INFO("  Number of pages: %i (%zi bytes)\n", (int) pages_count, size);
	INFO("  Attributes     : 0x%x\n", attributes);

	spin_lock(&mem_attr_smc_lock);

	int ret = change_mem_attributes(secure_partition_xlat_ctx_handle,
			base_va, size, smc_attr_to_mmap_attr(attributes));

	spin_unlock(&mem_attr_smc_lock);

	/* Convert error codes of change_mem_attributes() into SPM ones. */
	assert(ret == 0 || ret == -EINVAL);

	return (ret == 0) ? SPM_SUCCESS : SPM_INVALID_PARAMETER;
}


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

		switch (smc_fid) {

		case  SPM_VERSION_AARCH32:
			SMC_RET1(handle, SPM_VERSION_COMPILED);

		case SP_EVENT_COMPLETE_AARCH64:
			assert(handle == cm_get_context(SECURE));
			cm_el1_sysregs_context_save(SECURE);
			spm_setup_next_eret_into_sel0(handle);

			if (sp_init_in_progress) {
				/*
				 * SPM reports completion. The SPM must have
				 * initiated the original request through a
				 * synchronous entry into the secure
				 * partition. Jump back to the original C
				 * runtime context.
				 */
				spm_synchronous_sp_exit(&sp_ctx, x1);
				assert(0);
			}

			/*
			 * This is the result from the Secure partition of an
			 * earlier request. Copy the result into the non-secure
			 * context, save the secure state and return to the
			 * non-secure state.
			 */

			/* Get a reference to the non-secure context */
			ns_cpu_context = cm_get_context(NON_SECURE);
			assert(ns_cpu_context);

			/* Restore non-secure state */
			cm_el1_sysregs_context_restore(NON_SECURE);
			cm_set_next_eret_context(NON_SECURE);

			/* Return to normal world */
			SMC_RET1(ns_cpu_context, x1);

		case SP_MEM_ATTRIBUTES_GET_AARCH64:
			INFO("Received SP_MEM_ATTRIBUTES_GET_AARCH64 SMC\n");

			if (!sp_init_in_progress) {
				WARN("SP_MEM_ATTRIBUTES_GET_AARCH64 is available at boot time only\n");
				SMC_RET1(handle, SPM_NOT_SUPPORTED);
			}
			SMC_RET1(handle, spm_memory_attributes_get_smc_handler(x1));

		case SP_MEM_ATTRIBUTES_SET_AARCH64:
			INFO("Received SP_MEM_ATTRIBUTES_SET_AARCH64 SMC\n");

			if (!sp_init_in_progress) {
				WARN("SP_MEM_ATTRIBUTES_SET_AARCH64 is available at boot time only\n");
				SMC_RET1(handle, SPM_NOT_SUPPORTED);
			}
			SMC_RET1(handle, spm_memory_attributes_set_smc_handler(x1, x2, x3));
		default:
			break;
		}
	} else {

		/* Handle SMCs from Non-secure world. */

		switch (smc_fid) {

		case  SP_VERSION_AARCH64:
		case  SP_VERSION_AARCH32:
			SMC_RET1(handle, SP_VERSION_COMPILED);

		case SP_COMMUNICATE_AARCH32:
		case SP_COMMUNICATE_AARCH64:

			/* Save the Normal world context */
			cm_el1_sysregs_context_save(NON_SECURE);

			/*
			 * Restore the secure world context and prepare for
			 * entry in S-EL0
			 */
			assert(&sp_ctx.cpu_ctx == cm_get_context(SECURE));
			cm_el1_sysregs_context_restore(SECURE);
			cm_set_next_eret_context(SECURE);

			if (x2 != 0) {
				VERBOSE("SP_COMMUNICATE_AARCH32/64: X2 is not 0 as recommended.");
			}

			SMC_RET4(&sp_ctx.cpu_ctx,
				 smc_fid, x2, x3, plat_my_core_pos());

		case SP_MEM_ATTRIBUTES_GET_AARCH64:
		case SP_MEM_ATTRIBUTES_SET_AARCH64:
			/* SMC interfaces reserved for secure callers. */
			SMC_RET1(handle, SPM_NOT_SUPPORTED);

		default:
			break;
		}
	}

	SMC_RET1(handle, SMC_UNK);
}

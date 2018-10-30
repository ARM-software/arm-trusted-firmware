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
#include <ehf.h>
#include <errno.h>
#include <platform.h>
#include <runtime_svc.h>
#include <smccc.h>
#include <smccc_helpers.h>
#include <spinlock.h>
#include <spm_svc.h>
#include <utils.h>
#include <xlat_tables_v2.h>

#include "spm_private.h"

/*******************************************************************************
 * Secure Partition context information.
 ******************************************************************************/
static sp_context_t sp_ctx;

/*******************************************************************************
 * Set state of a Secure Partition context.
 ******************************************************************************/
void sp_state_set(sp_context_t *sp_ptr, sp_state_t state)
{
	spin_lock(&(sp_ptr->state_lock));
	sp_ptr->state = state;
	spin_unlock(&(sp_ptr->state_lock));
}

/*******************************************************************************
 * Wait until the state of a Secure Partition is the specified one and change it
 * to the desired state.
 ******************************************************************************/
void sp_state_wait_switch(sp_context_t *sp_ptr, sp_state_t from, sp_state_t to)
{
	int success = 0;

	while (success == 0) {
		spin_lock(&(sp_ptr->state_lock));

		if (sp_ptr->state == from) {
			sp_ptr->state = to;

			success = 1;
		}

		spin_unlock(&(sp_ptr->state_lock));
	}
}

/*******************************************************************************
 * Check if the state of a Secure Partition is the specified one and, if so,
 * change it to the desired state. Returns 0 on success, -1 on error.
 ******************************************************************************/
int sp_state_try_switch(sp_context_t *sp_ptr, sp_state_t from, sp_state_t to)
{
	int ret = -1;

	spin_lock(&(sp_ptr->state_lock));

	if (sp_ptr->state == from) {
		sp_ptr->state = to;

		ret = 0;
	}

	spin_unlock(&(sp_ptr->state_lock));

	return ret;
}

/*******************************************************************************
 * This function takes an SP context pointer and performs a synchronous entry
 * into it.
 ******************************************************************************/
static uint64_t spm_sp_synchronous_entry(sp_context_t *sp_ctx)
{
	uint64_t rc;

	assert(sp_ctx != NULL);

	/* Assign the context of the SP to this CPU */
	cm_set_context(&(sp_ctx->cpu_ctx), SECURE);

	/* Restore the context assigned above */
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	/* Invalidate TLBs at EL1. */
	tlbivmalle1();
	dsbish();

	/* Enter Secure Partition */
	rc = spm_secure_partition_enter(&sp_ctx->c_rt_ctx);

	/* Save secure state */
	cm_el1_sysregs_context_save(SECURE);

	return rc;
}

/*******************************************************************************
 * This function returns to the place where spm_sp_synchronous_entry() was
 * called originally.
 ******************************************************************************/
__dead2 static void spm_sp_synchronous_exit(uint64_t rc)
{
	sp_context_t *ctx = &sp_ctx;

	/*
	 * The SPM must have initiated the original request through a
	 * synchronous entry into the secure partition. Jump back to the
	 * original C runtime context with the value of rc in x0;
	 */
	spm_secure_partition_exit(ctx->c_rt_ctx, rc);

	panic();
}

/*******************************************************************************
 * Jump to each Secure Partition for the first time.
 ******************************************************************************/
static int32_t spm_init(void)
{
	uint64_t rc;
	sp_context_t *ctx;

	INFO("Secure Partition init...\n");

	ctx = &sp_ctx;

	ctx->state = SP_STATE_RESET;

	rc = spm_sp_synchronous_entry(ctx);
	assert(rc == 0);

	ctx->state = SP_STATE_IDLE;

	INFO("Secure Partition initialized.\n");

	return rc;
}

/*******************************************************************************
 * Initialize contexts of all Secure Partitions.
 ******************************************************************************/
int32_t spm_setup(void)
{
	int rc;
	sp_context_t *ctx;
	void *sp_base, *rd_base;
	size_t sp_size, rd_size;

	/* Disable MMU at EL1 (initialized by BL2) */
	disable_mmu_icache_el1();

	/* Initialize context of the SP */
	INFO("Secure Partition context setup start...\n");

	ctx = &sp_ctx;

	rc = plat_spm_sp_get_next_address(&sp_base, &sp_size,
					  &rd_base, &rd_size);
	if (rc != 0) {
		ERROR("No Secure Partition found.\n");
		panic();
	}

	/* Assign translation tables context. */
	ctx->xlat_ctx_handle = spm_get_sp_xlat_context();

	/* Save location of the image in physical memory */
	ctx->image_base = (uintptr_t)sp_base;
	ctx->image_size = sp_size;

	rc = plat_spm_sp_rd_load(&ctx->rd, rd_base, rd_size);
	if (rc < 0) {
		ERROR("Error while loading RD blob.\n");
		panic();
	}

	spm_sp_setup(ctx);

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&spm_init);

	INFO("Secure Partition setup done.\n");

	return 0;
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

		case SP_MEMORY_ATTRIBUTES_GET_AARCH64:
			INFO("Received SP_MEMORY_ATTRIBUTES_GET_AARCH64 SMC\n");

			if (sp_ctx.state != SP_STATE_RESET) {
				WARN("SP_MEMORY_ATTRIBUTES_GET_AARCH64 is available at boot time only\n");
				SMC_RET1(handle, SPM_NOT_SUPPORTED);
			}
			SMC_RET1(handle,
				 spm_memory_attributes_get_smc_handler(
					 &sp_ctx, x1));

		case SP_MEMORY_ATTRIBUTES_SET_AARCH64:
			INFO("Received SP_MEMORY_ATTRIBUTES_SET_AARCH64 SMC\n");

			if (sp_ctx.state != SP_STATE_RESET) {
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

		assert(handle == cm_get_context(NON_SECURE));

		switch (smc_fid) {

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

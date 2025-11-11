/*
 * Copyright (c) 2017-2025, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <assert.h>
#include <errno.h>

#include <bl31/bl31.h>
#include <bl31/ehf.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/el3_runtime/simd_ctx.h>
#include <lib/smccc.h>
#include <lib/spinlock.h>
#include <lib/utils.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/spm_mm_partition.h>
#include <services/spm_mm_svc.h>
#include <services/ven_el3_svc.h>
#include <smccc_helpers.h>

#include "spm_common.h"
#include "spm_mm_private.h"

/*******************************************************************************
 * Secure Partition context information.
 ******************************************************************************/
static sp_context_t sp_ctx;

/********************************************************************************
 * TPM service UUID: 17b862a4-1806-4faf-86b3-089a58353861 as mentioned in
 * https://developer.arm.com/documentation/den0138/latest/
 *******************************************************************************/
DEFINE_SVC_UUID2(tpm_service_uuid,
		 0x17b862a4, 0x1806, 0x4faf, 0x86, 0xb3,
		 0x08, 0x9a, 0x58, 0x35, 0x38, 0x61);

/*******************************************************************************
 * Set state of a Secure Partition context.
 ******************************************************************************/
static void sp_state_set(sp_context_t *sp_ptr, sp_state_t state)
{
	sp_ptr->state = state;
	spin_unlock(&(sp_ptr->state_lock));
}

/*******************************************************************************
 * Change the state of a Secure Partition to the one specified.
 ******************************************************************************/
static void sp_state_wait_switch(sp_context_t *sp_ptr, sp_state_t from, sp_state_t to)
{
	spin_lock(&(sp_ptr->state_lock));
	sp_ptr->state = to;
}

/*******************************************************************************
 * This function takes an SP context pointer and performs a synchronous entry
 * into it.
 ******************************************************************************/
static uint64_t spm_sp_synchronous_entry(sp_context_t *ctx)
{
	uint64_t rc;

	assert(ctx != NULL);

	/* Assign the context of the SP to this CPU */
	cm_set_context(&(ctx->cpu_ctx), SECURE);

	/* Restore the context assigned above */
	cm_el1_sysregs_context_restore(SECURE);
	cm_set_next_eret_context(SECURE);

	/* Invalidate TLBs at EL1. */
	tlbivmalle1();
	dsbish();

	/* Enter Secure Partition */
	rc = spm_secure_partition_enter(&ctx->c_rt_ctx);

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

	return !rc;
}

/*******************************************************************************
 * Initialize contexts of all Secure Partitions.
 ******************************************************************************/
int32_t spm_mm_setup(void)
{
	sp_context_t *ctx;

	/* Disable MMU at EL1 (initialized by BL2) */
	disable_mmu_icache_el1();

	/* Initialize context of the SP */
	INFO("Secure Partition context setup start...\n");

	ctx = &sp_ctx;

	/* Assign translation tables context. */
	ctx->xlat_ctx_handle = spm_get_sp_xlat_context();

	spm_sp_setup(ctx);

	/* Register init function for deferred init.  */
	bl31_register_bl32_init(&spm_init);

	INFO("Secure Partition setup done.\n");

	return 0;
}

/*******************************************************************************
 * Function to perform a call to a Secure Partition.
 ******************************************************************************/
uint64_t spm_mm_sp_call(uint32_t smc_fid, uint64_t x1, uint64_t x2, uint64_t x3)
{
	uint64_t rc;
	sp_context_t *sp_ptr = &sp_ctx;

#if CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS
	/*
	 * SP runs to completion, no need to restore FP/SVE registers of secure context.
	 * Save FP/SVE registers only for non secure context.
	 */
	simd_ctx_save(NON_SECURE, false);
#endif /* CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS */

	/* Wait until the Secure Partition is idle and set it to busy. */
	sp_state_wait_switch(sp_ptr, SP_STATE_IDLE, SP_STATE_BUSY);

	/* Set values for registers on SP entry */
	cpu_context_t *cpu_ctx = &(sp_ptr->cpu_ctx);

	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X0, smc_fid);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X1, x1);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X2, x2);
	write_ctx_reg(get_gpregs_ctx(cpu_ctx), CTX_GPREG_X3, x3);

	/* Jump to the Secure Partition. */
	rc = spm_sp_synchronous_entry(sp_ptr);

	/* Flag Secure Partition as idle. */
	assert(sp_ptr->state == SP_STATE_BUSY);
	sp_state_set(sp_ptr, SP_STATE_IDLE);

#if CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS
	/*
	 * SP runs to completion, no need to save FP/SVE registers of secure context.
	 * Restore only non secure world FP/SVE registers.
	 */
	simd_ctx_restore(NON_SECURE);
#endif /* CTX_INCLUDE_FPREGS || CTX_INCLUDE_SVE_REGS */

	return rc;
}

/*******************************************************************************
 * MM_COMMUNICATE handler
 ******************************************************************************/
static uint64_t mm_communicate(uint32_t smc_fid, uint64_t mm_cookie,
			       uint64_t comm_buffer_address,
			       uint64_t comm_size_address, void *handle)
{
	uint64_t rc;

	/* Cookie. Reserved for future use. It must be zero. */
	if (mm_cookie != 0U) {
		ERROR("MM_COMMUNICATE: cookie is not zero\n");
		SMC_RET1(handle, SPM_MM_INVALID_PARAMETER);
	}

	if (comm_buffer_address == 0U) {
		ERROR("MM_COMMUNICATE: comm_buffer_address is zero\n");
		SMC_RET1(handle, SPM_MM_INVALID_PARAMETER);
	}

	if (comm_size_address != 0U) {
		VERBOSE("MM_COMMUNICATE: comm_size_address is not 0 as recommended.\n");
	}

	/*
	 * The current secure partition design mandates
	 * - at any point, only a single core can be
	 *   executing in the secure partition.
	 * - a core cannot be preempted by an interrupt
	 *   while executing in secure partition.
	 * Raise the running priority of the core to the
	 * interrupt level configured for secure partition
	 * so as to block any interrupt from preempting this
	 * core.
	 */
	ehf_activate_priority(PLAT_SP_PRI);

	/* Save the Normal world context */
	cm_el1_sysregs_context_save(NON_SECURE);

	rc = spm_mm_sp_call(smc_fid, comm_buffer_address, comm_size_address,
			    plat_my_core_pos());

	/* Restore non-secure state */
	cm_el1_sysregs_context_restore(NON_SECURE);
	cm_set_next_eret_context(NON_SECURE);

	/*
	 * Exited from secure partition. This core can take
	 * interrupts now.
	 */
	ehf_deactivate_priority(PLAT_SP_PRI);

	SMC_RET1(handle, rc);
}

/*******************************************************************************
 * SPM_MM TPM start handler as mentioned in section 3.3.1 of TCG ACPI
 * specification version 1.4
 ******************************************************************************/
uint64_t spm_mm_tpm_start_handler(uint32_t smc_fid,
				  uint64_t x1,
				  uint64_t x2,
				  uint64_t x3,
				  uint64_t x4,
				  void *cookie,
				  void *handle,
				  uint64_t flags)
{
	mm_communicate_header_t *mm_comm_header = (void *)PLAT_SPM_BUF_BASE;
	uint32_t spm_mm_smc_fid;

	if (!is_caller_non_secure(flags)) {
		ERROR("spm_mm TPM START must be requested from normal world only.\n");
		SMC_RET1(handle, SMC_UNK);
	}

	switch (smc_fid) {
	case TPM_START_SMC_32:
		spm_mm_smc_fid = MM_COMMUNICATE_AARCH32;
		break;
	case TPM_START_SMC_64:
		spm_mm_smc_fid = MM_COMMUNICATE_AARCH64;
		break;
	default:
		ERROR("Unexpected SMC FID\n");
		SMC_RET1(handle, SMC_UNK);
		break;
	}

	memset(mm_comm_header, 0U, sizeof(mm_communicate_header_t));
	memcpy(&mm_comm_header->header_guid, &tpm_service_uuid, sizeof(struct efi_guid));

	return mm_communicate(spm_mm_smc_fid, x1,  (uint64_t)mm_comm_header, x3, handle);
}

/*******************************************************************************
 * Secure Partition Manager SMC handler.
 ******************************************************************************/
uint64_t spm_mm_smc_handler(uint32_t smc_fid,
			 uint64_t x1,
			 uint64_t x2,
			 uint64_t x3,
			 uint64_t x4,
			 void *cookie,
			 void *handle,
			 uint64_t flags)
{
	unsigned int ns;
	int32_t ret;
	uint32_t attr;
	uint32_t page_count;

	/* Determine which security state this SMC originated from */
	ns = is_caller_non_secure(flags);

	if (ns == SMC_FROM_SECURE) {

		/* Handle SMCs from Secure world. */

		assert(handle == cm_get_context(SECURE));

		/* Make next ERET jump to S-EL0 instead of S-EL1. */
		cm_set_elr_spsr_el3(SECURE, read_elr_el1(), read_spsr_el1());

		switch (smc_fid) {

		case SPM_MM_VERSION_AARCH32:
			SMC_RET1(handle, SPM_MM_VERSION_COMPILED);

		case MM_SP_EVENT_COMPLETE_AARCH64:
			spm_sp_synchronous_exit(x1);

		case MM_SP_MEMORY_ATTRIBUTES_GET_AARCH64:
			INFO("Received MM_SP_MEMORY_ATTRIBUTES_GET_AARCH64 SMC\n");

			if (sp_ctx.state != SP_STATE_RESET) {
				WARN("MM_SP_MEMORY_ATTRIBUTES_GET_AARCH64 is available at boot time only\n");
				SMC_RET1(handle, SPM_MM_NOT_SUPPORTED);
			}

			/* x2 = page_count - 1 */
			page_count = x2 + 1;

			ret = spm_memory_attributes_get_smc_handler(
					&sp_ctx, x1, &page_count, &attr);
			if (ret != SPM_MM_SUCCESS) {
				SMC_RET1(handle, ret);
			} else {
				SMC_RET2(handle, attr, --page_count);
			}

		case MM_SP_MEMORY_ATTRIBUTES_SET_AARCH64:
			INFO("Received MM_SP_MEMORY_ATTRIBUTES_SET_AARCH64 SMC\n");

			if (sp_ctx.state != SP_STATE_RESET) {
				WARN("MM_SP_MEMORY_ATTRIBUTES_SET_AARCH64 is available at boot time only\n");
				SMC_RET1(handle, SPM_MM_NOT_SUPPORTED);
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

		case MM_VERSION_AARCH32:
			SMC_RET1(handle, MM_VERSION_COMPILED);

		case MM_COMMUNICATE_AARCH32:
		case MM_COMMUNICATE_AARCH64:
			return mm_communicate(smc_fid, x1, x2, x3, handle);

		case MM_SP_MEMORY_ATTRIBUTES_GET_AARCH64:
		case MM_SP_MEMORY_ATTRIBUTES_SET_AARCH64:
			/* SMC interfaces reserved for secure callers. */
			SMC_RET1(handle, SPM_MM_NOT_SUPPORTED);

		default:
			break;
		}
	}

	SMC_RET1(handle, SMC_UNK);
}

bool is_spm_mm_fid(uint32_t smc_fid)
{
	uint32_t fid = smc_fid & MM_FID_MASK;

	if ((fid >= MM_FID_MIN_VALUE) &&
	    (fid <= MM_FID_MAX_VALUE)) {
		return true;
	}

	switch (smc_fid) {
	case SPM_MM_VERSION_AARCH32:
	case MM_SP_EVENT_COMPLETE_AARCH64:
	case MM_SP_MEMORY_ATTRIBUTES_GET_AARCH64:
	case MM_SP_MEMORY_ATTRIBUTES_SET_AARCH64:
		return true;
	default:
		break;
	}

	return false;
}

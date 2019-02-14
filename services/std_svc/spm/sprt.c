/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <limits.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <common/runtime_svc.h>
#include <lib/el3_runtime/context_mgmt.h>
#include <lib/smccc.h>
#include <lib/utils.h>
#include <plat/common/platform.h>
#include <services/sprt_svc.h>
#include <smccc_helpers.h>

#include "spm_private.h"

/*******************************************************************************
 * Functions to manipulate memory regions
 ******************************************************************************/

/*
 * Attributes are encoded using a different format in the SMC interface than in
 * the Trusted Firmware, where the mmap_attr_t enum type is used. This function
 * converts an attributes value from the SMC format to the mmap_attr_t format by
 * setting MT_RW/MT_RO, MT_USER/MT_PRIVILEGED and MT_EXECUTE/MT_EXECUTE_NEVER.
 * The other fields are left as 0 because they are ignored by the function
 * xlat_change_mem_attributes_ctx().
 */
static unsigned int smc_attr_to_mmap_attr(unsigned int attributes)
{
	unsigned int perm = attributes & SPRT_MEMORY_PERM_ATTR_MASK;

	if (perm == SPRT_MEMORY_PERM_ATTR_RW) {
		return MT_RW | MT_EXECUTE_NEVER | MT_USER;
	} else if (perm ==  SPRT_MEMORY_PERM_ATTR_RO) {
		return MT_RO | MT_EXECUTE_NEVER | MT_USER;
	} else if (perm == SPRT_MEMORY_PERM_ATTR_RO_EXEC) {
		return MT_RO | MT_USER;
	} else {
		return UINT_MAX;
	}
}

/*
 * This function converts attributes from the Trusted Firmware format into the
 * SMC interface format.
 */
static unsigned int mmap_attr_to_smc_attr(unsigned int attr)
{
	unsigned int perm;

	/* No access from EL0. */
	if ((attr & MT_USER) == 0U)
		return UINT_MAX;

	if ((attr & MT_RW) != 0) {
		assert(MT_TYPE(attr) != MT_DEVICE);
		perm = SPRT_MEMORY_PERM_ATTR_RW;
	} else {
		if ((attr & MT_EXECUTE_NEVER) != 0U) {
			perm = SPRT_MEMORY_PERM_ATTR_RO;
		} else {
			perm = SPRT_MEMORY_PERM_ATTR_RO_EXEC;
		}
	}

	return perm << SPRT_MEMORY_PERM_ATTR_SHIFT;
}

static int32_t sprt_memory_perm_attr_get(sp_context_t *sp_ctx, uintptr_t base_va)
{
	uint32_t attributes;

	spin_lock(&(sp_ctx->xlat_ctx_lock));

	int ret = xlat_get_mem_attributes_ctx(sp_ctx->xlat_ctx_handle,
				     base_va, &attributes);

	spin_unlock(&(sp_ctx->xlat_ctx_lock));

	/* Convert error codes of xlat_get_mem_attributes_ctx() into SPM. */
	assert((ret == 0) || (ret == -EINVAL));

	if (ret != 0)
		return SPRT_INVALID_PARAMETER;

	unsigned int perm = mmap_attr_to_smc_attr(attributes);

	if (perm == UINT_MAX)
		return SPRT_INVALID_PARAMETER;

	return SPRT_SUCCESS | perm;
}

static int32_t sprt_memory_perm_attr_set(sp_context_t *sp_ctx,
		u_register_t page_address, u_register_t pages_count,
		u_register_t smc_attributes)
{
	int ret;
	uintptr_t base_va = (uintptr_t) page_address;
	size_t size = pages_count * PAGE_SIZE;

	VERBOSE("  Start address  : 0x%lx\n", base_va);
	VERBOSE("  Number of pages: %i (%zi bytes)\n", (int) pages_count, size);
	VERBOSE("  Attributes     : 0x%lx\n", smc_attributes);

	uint32_t mmap_attr = smc_attr_to_mmap_attr(smc_attributes);

	if (mmap_attr == UINT_MAX) {
		WARN("%s: Invalid memory attributes: 0x%lx\n", __func__,
		     smc_attributes);
		return SPRT_INVALID_PARAMETER;
	}

	/*
	 * Perform some checks before actually trying to change the memory
	 * attributes.
	 */

	spin_lock(&(sp_ctx->xlat_ctx_lock));

	uint32_t attributes;

	ret = xlat_get_mem_attributes_ctx(sp_ctx->xlat_ctx_handle,
				     base_va, &attributes);

	if (ret != 0) {
		spin_unlock(&(sp_ctx->xlat_ctx_lock));
		return SPRT_INVALID_PARAMETER;
	}

	if ((attributes & MT_USER) == 0U) {
		/* Prohibit changing attributes of S-EL1 regions */
		spin_unlock(&(sp_ctx->xlat_ctx_lock));
		return SPRT_INVALID_PARAMETER;
	}

	ret = xlat_change_mem_attributes_ctx(sp_ctx->xlat_ctx_handle,
					base_va, size, mmap_attr);

	spin_unlock(&(sp_ctx->xlat_ctx_lock));

	/* Convert error codes of xlat_change_mem_attributes_ctx() into SPM. */
	assert((ret == 0) || (ret == -EINVAL));

	return (ret == 0) ? SPRT_SUCCESS : SPRT_INVALID_PARAMETER;
}

/*******************************************************************************
 * This function handles all SMCs in the range reserved for SPRT.
 ******************************************************************************/
static uintptr_t sprt_smc_handler(uint32_t smc_fid, u_register_t x1,
				  u_register_t x2, u_register_t x3,
				  u_register_t x4, void *cookie, void *handle,
				  u_register_t flags)
{
	/* SPRT only supported from the Secure world */
	if (is_caller_non_secure(flags) == SMC_FROM_NON_SECURE) {
		SMC_RET1(handle, SMC_UNK);
	}

	assert(handle == cm_get_context(SECURE));

	/*
	 * Only S-EL0 partitions are supported for now. Make the next ERET into
	 * the partition jump directly to S-EL0 instead of S-EL1.
	 */
	cm_set_elr_spsr_el3(SECURE, read_elr_el1(), read_spsr_el1());

	switch (smc_fid) {
	case SPRT_VERSION:
		SMC_RET1(handle, SPRT_VERSION_COMPILED);

	case SPRT_PUT_RESPONSE_AARCH64:
		spm_sp_synchronous_exit(SPRT_PUT_RESPONSE_AARCH64);

	case SPRT_YIELD_AARCH64:
		spm_sp_synchronous_exit(SPRT_YIELD_AARCH64);

	case SPRT_MEMORY_PERM_ATTR_GET_AARCH64:
	{
		/* Get context of the SP in use by this CPU. */
		unsigned int linear_id = plat_my_core_pos();
		sp_context_t *sp_ctx = spm_cpu_get_sp_ctx(linear_id);

		SMC_RET1(handle, sprt_memory_perm_attr_get(sp_ctx, x1));
	}

	case SPRT_MEMORY_PERM_ATTR_SET_AARCH64:
	{
		/* Get context of the SP in use by this CPU. */
		unsigned int linear_id = plat_my_core_pos();
		sp_context_t *sp_ctx = spm_cpu_get_sp_ctx(linear_id);

		SMC_RET1(handle, sprt_memory_perm_attr_set(sp_ctx, x1, x2, x3));
	}

	default:
		break;
	}

	WARN("SPRT: Unsupported call 0x%08x\n", smc_fid);
	SMC_RET1(handle, SPRT_NOT_SUPPORTED);
}

DECLARE_RT_SVC(
	sprt_handler,
	OEN_SPRT_START,
	OEN_SPRT_END,
	SMC_TYPE_FAST,
	NULL,
	sprt_smc_handler
);

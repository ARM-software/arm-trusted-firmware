/*
 * Copyright (c) 2018-2025, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <errno.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <platform_def.h>
#include <plat/common/platform.h>
#include <services/spm_mm_partition.h>
#include <services/spm_mm_svc.h>

#include "spm_mm_private.h"
#include "spm_shim_private.h"

/* Lock used for SP_MEMORY_ATTRIBUTES_GET and SP_MEMORY_ATTRIBUTES_SET */
static spinlock_t mem_attr_smc_lock;

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
	unsigned int tf_attr = 0U;

	unsigned int access = (attributes & MM_SP_MEMORY_ATTRIBUTES_ACCESS_MASK)
			      >> MM_SP_MEMORY_ATTRIBUTES_ACCESS_SHIFT;

	if (access == MM_SP_MEMORY_ATTRIBUTES_ACCESS_RW) {
		tf_attr |= MT_RW | MT_USER;
	} else if (access ==  MM_SP_MEMORY_ATTRIBUTES_ACCESS_RO) {
		tf_attr |= MT_RO | MT_USER;
	} else {
		/* Other values are reserved. */
		assert(access == MM_SP_MEMORY_ATTRIBUTES_ACCESS_NOACCESS);
		/* The only requirement is that there's no access from EL0 */
		tf_attr |= MT_RO | MT_PRIVILEGED;
	}

	if ((attributes & MM_SP_MEMORY_ATTRIBUTES_NON_EXEC) == 0) {
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
		data_access = MM_SP_MEMORY_ATTRIBUTES_ACCESS_NOACCESS;
	} else {
		if ((attr & MT_RW) != 0) {
			assert(MT_TYPE(attr) != MT_DEVICE);
			data_access = MM_SP_MEMORY_ATTRIBUTES_ACCESS_RW;
		} else {
			data_access = MM_SP_MEMORY_ATTRIBUTES_ACCESS_RO;
		}
	}

	smc_attr |= (data_access & MM_SP_MEMORY_ATTRIBUTES_ACCESS_MASK)
		    << MM_SP_MEMORY_ATTRIBUTES_ACCESS_SHIFT;

	if ((attr & MT_EXECUTE_NEVER) != 0U) {
		smc_attr |= MM_SP_MEMORY_ATTRIBUTES_NON_EXEC;
	}

	return smc_attr;
}

int32_t spm_memory_attributes_get_smc_handler(sp_context_t *sp_ctx,
					      uintptr_t base_va,
					      uint32_t *page_count,
					      uint32_t *attr)
{
	uint32_t cur_attr;
	uint32_t table_level;
	uint32_t count;
	int rc;

	assert((page_count != NULL) && (*page_count > 0));
	assert(attr != NULL);

	base_va &= ~(PAGE_SIZE_MASK);

	spin_lock(&mem_attr_smc_lock);

	rc = xlat_get_mem_attributes_ctx(sp_ctx->xlat_ctx_handle,
				     base_va, attr, &table_level);
	if (rc != 0) {
		goto err_out;
	}

	/*
	 * Caculate how many pages in this block entry from base_va including
	 * its page.
	 */
	count = ((XLAT_BLOCK_SIZE(table_level) -
				(base_va & XLAT_BLOCK_MASK(table_level))) >> PAGE_SIZE_SHIFT);
	base_va += XLAT_BLOCK_SIZE(table_level);

	while ((count < *page_count) && (base_va != 0x00)) {
		rc = xlat_get_mem_attributes_ctx(sp_ctx->xlat_ctx_handle,
					     base_va, &cur_attr, &table_level);
		if (rc != 0) {
			goto err_out;
		}

		if (*attr != cur_attr) {
			*page_count = count;
			break;
		}

		base_va += XLAT_BLOCK_SIZE(table_level);
		count += (XLAT_BLOCK_SIZE(table_level) >> PAGE_SIZE_SHIFT);
	}

	*attr = smc_mmap_to_smc_attr(*attr);

err_out:
	spin_unlock(&mem_attr_smc_lock);
	/* Convert error codes of xlat_get_mem_attributes_ctx() into SPM. */
	assert((rc == 0) || (rc == -EINVAL));

	if (rc == 0) {
		return SPM_MM_SUCCESS;
	} else {
		return SPM_MM_INVALID_PARAMETER;
	}
}

int spm_memory_attributes_set_smc_handler(sp_context_t *sp_ctx,
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

	int ret = xlat_change_mem_attributes_ctx(sp_ctx->xlat_ctx_handle,
					base_va, size,
					smc_attr_to_mmap_attr(attributes));

	spin_unlock(&mem_attr_smc_lock);

	/* Convert error codes of xlat_change_mem_attributes_ctx() into SPM. */
	assert((ret == 0) || (ret == -EINVAL));

	return (ret == 0) ? SPM_MM_SUCCESS : SPM_MM_INVALID_PARAMETER;
}

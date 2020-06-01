/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
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
#include "spm_mm_shim_private.h"

/* Place translation tables by default along with the ones used by BL31. */
#ifndef PLAT_SP_IMAGE_XLAT_SECTION_NAME
#define PLAT_SP_IMAGE_XLAT_SECTION_NAME	"xlat_table"
#endif
#ifndef PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME
#define PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME	".bss"
#endif

/* Allocate and initialise the translation context for the secure partitions. */
REGISTER_XLAT_CONTEXT2(sp,
		       PLAT_SP_IMAGE_MMAP_REGIONS,
		       PLAT_SP_IMAGE_MAX_XLAT_TABLES,
		       PLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE,
		       EL1_EL0_REGIME, PLAT_SP_IMAGE_XLAT_SECTION_NAME,
		       PLAT_SP_IMAGE_BASE_XLAT_SECTION_NAME);

/* Lock used for SP_MEMORY_ATTRIBUTES_GET and SP_MEMORY_ATTRIBUTES_SET */
static spinlock_t mem_attr_smc_lock;

/* Get handle of Secure Partition translation context */
xlat_ctx_t *spm_get_sp_xlat_context(void)
{
	return &sp_xlat_ctx;
};

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
					      uintptr_t base_va)
{
	uint32_t attributes;

	spin_lock(&mem_attr_smc_lock);

	int rc = xlat_get_mem_attributes_ctx(sp_ctx->xlat_ctx_handle,
				     base_va, &attributes);

	spin_unlock(&mem_attr_smc_lock);

	/* Convert error codes of xlat_get_mem_attributes_ctx() into SPM. */
	assert((rc == 0) || (rc == -EINVAL));

	if (rc == 0) {
		return (int32_t) smc_mmap_to_smc_attr(attributes);
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

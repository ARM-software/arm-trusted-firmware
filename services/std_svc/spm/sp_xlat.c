/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_helpers.h>
#include <assert.h>
#include <errno.h>
#include <object_pool.h>
#include <platform_def.h>
#include <platform.h>
#include <sp_res_desc.h>
#include <spm_svc.h>
#include <string.h>
#include <utils.h>
#include <xlat_tables_v2.h>

#include "spm_private.h"
#include "spm_shim_private.h"

/*******************************************************************************
 * Instantiation of translation table context
 ******************************************************************************/

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
xlat_ctx_t *spm_sp_xlat_context_alloc(void)
{
	return &sp_xlat_ctx;
};

/*******************************************************************************
 * Functions to allocate memory for regions.
 ******************************************************************************/

/*
 * The region with base PLAT_SPM_HEAP_BASE and size PLAT_SPM_HEAP_SIZE is
 * reserved for SPM to use as heap to allocate memory regions of Secure
 * Partitions. This is only done at boot.
 */
static OBJECT_POOL(spm_heap_mem, (void *)PLAT_SPM_HEAP_BASE, 1U,
		   PLAT_SPM_HEAP_SIZE);

static uintptr_t spm_alloc_heap(size_t size)
{
	return (uintptr_t)pool_alloc_n(&spm_heap_mem, size);
}

/*******************************************************************************
 * Functions to map memory regions described in the resource description.
 ******************************************************************************/
static unsigned int rdmem_attr_to_mmap_attr(uint32_t attr)
{
	unsigned int index = attr & RD_MEM_MASK;

	const unsigned int mmap_attr_arr[8] = {
		MT_DEVICE | MT_RW | MT_SECURE,	/* RD_MEM_DEVICE */
		MT_CODE | MT_SECURE,		/* RD_MEM_NORMAL_CODE */
		MT_MEMORY | MT_RW | MT_SECURE,	/* RD_MEM_NORMAL_DATA */
		MT_MEMORY | MT_RW | MT_SECURE,	/* RD_MEM_NORMAL_BSS */
		MT_RO_DATA | MT_SECURE,		/* RD_MEM_NORMAL_RODATA */
		MT_MEMORY | MT_RW | MT_SECURE,	/* RD_MEM_NORMAL_SPM_SP_SHARED_MEM */
		MT_MEMORY | MT_RW | MT_SECURE,	/* RD_MEM_NORMAL_CLIENT_SHARED_MEM */
		MT_MEMORY | MT_RW | MT_SECURE	/* RD_MEM_NORMAL_MISCELLANEOUS */
	};

	if (index >= ARRAY_SIZE(mmap_attr_arr)) {
		ERROR("Unsupported RD memory attributes 0x%x\n", attr);
		panic();
	}

	return mmap_attr_arr[index];
}

/*
 * The data provided in the resource description structure is not directly
 * compatible with a mmap_region structure. This function handles the conversion
 * and maps it.
 */
static void map_rdmem(sp_context_t *sp_ctx, struct sp_rd_sect_mem_region *rdmem)
{
	int rc;
	mmap_region_t mmap;

	/* Location of the SP image */
	uintptr_t sp_size = sp_ctx->image_size;
	uintptr_t sp_base_va = sp_ctx->rd.attribute.load_address;
	unsigned long long sp_base_pa = sp_ctx->image_base;

	/* Location of the memory region to map */
	size_t rd_size = rdmem->size;
	uintptr_t rd_base_va = rdmem->base;
	unsigned long long rd_base_pa;

	unsigned int memtype = rdmem->attr & RD_MEM_MASK;

	VERBOSE("Adding memory region '%s'\n", rdmem->name);

	mmap.granularity = REGION_DEFAULT_GRANULARITY;

	/* Check if the RD region is inside of the SP image or not */
	int is_outside = (rd_base_va + rd_size <= sp_base_va) ||
			 (sp_base_va + sp_size <= rd_base_va);

	/* Set to 1 if it is needed to zero this region */
	int zero_region = 0;

	switch (memtype) {
	case RD_MEM_DEVICE:
		/* Device regions are mapped 1:1 */
		rd_base_pa = rd_base_va;
		break;

	case RD_MEM_NORMAL_CODE:
	case RD_MEM_NORMAL_RODATA:
	{
		if (is_outside == 1) {
			ERROR("Code and rodata sections must be fully contained in the image.");
			panic();
		}

		/* Get offset into the image */
		rd_base_pa = sp_base_pa + rd_base_va - sp_base_va;
		break;
	}
	case RD_MEM_NORMAL_DATA:
	{
		if (is_outside == 1) {
			ERROR("Data sections must be fully contained in the image.");
			panic();
		}

		rd_base_pa = spm_alloc_heap(rd_size);

		/* Get offset into the image */
		void *img_pa = (void *)(sp_base_pa + rd_base_va - sp_base_va);

		VERBOSE("  Copying data from %p to 0x%llx\n", img_pa, rd_base_pa);

		/* Map destination */
		rc = mmap_add_dynamic_region(rd_base_pa, rd_base_pa,
				rd_size, MT_MEMORY | MT_RW | MT_SECURE);
		if (rc != 0) {
			ERROR("Unable to map data region at EL3: %d\n", rc);
			panic();
		}

		/* Copy original data to destination */
		memcpy((void *)rd_base_pa, img_pa, rd_size);

		/* Unmap destination region */
		rc = mmap_remove_dynamic_region(rd_base_pa, rd_size);
		if (rc != 0) {
			ERROR("Unable to remove data region at EL3: %d\n", rc);
			panic();
		}

		break;
	}
	case RD_MEM_NORMAL_MISCELLANEOUS:
		/* Allow SPM to change the attributes of the region. */
		mmap.granularity = PAGE_SIZE;
		rd_base_pa = spm_alloc_heap(rd_size);
		zero_region = 1;
		break;

	case RD_MEM_NORMAL_SPM_SP_SHARED_MEM:
		if ((sp_ctx->spm_sp_buffer_base != 0) ||
		    (sp_ctx->spm_sp_buffer_size != 0)) {
			ERROR("A partition must have only one SPM<->SP buffer.\n");
			panic();
		}
		rd_base_pa = spm_alloc_heap(rd_size);
		zero_region = 1;
		/* Save location of this buffer, it is needed by SPM */
		sp_ctx->spm_sp_buffer_base = rd_base_pa;
		sp_ctx->spm_sp_buffer_size = rd_size;
		break;

	case RD_MEM_NORMAL_CLIENT_SHARED_MEM:
		/* Fallthrough */
	case RD_MEM_NORMAL_BSS:
		rd_base_pa = spm_alloc_heap(rd_size);
		zero_region = 1;
		break;

	default:
		panic();
	}

	mmap.base_pa = rd_base_pa;
	mmap.base_va = rd_base_va;
	mmap.size = rd_size;

	/* Only S-EL0 mappings supported for now */
	mmap.attr = rdmem_attr_to_mmap_attr(rdmem->attr) | MT_USER;

	VERBOSE("  VA: 0x%lx PA: 0x%llx (0x%lx, attr: 0x%x)\n",
		mmap.base_va, mmap.base_pa, mmap.size, mmap.attr);

	/* Map region in the context of the Secure Partition */
	mmap_add_region_ctx(sp_ctx->xlat_ctx_handle, &mmap);

	if (zero_region == 1) {
		VERBOSE("  Zeroing region...\n");

		rc = mmap_add_dynamic_region(mmap.base_pa, mmap.base_pa,
				mmap.size, MT_MEMORY | MT_RW | MT_SECURE);
		if (rc != 0) {
			ERROR("Unable to map memory at EL3 to zero: %d\n",
			      rc);
			panic();
		}

		zeromem((void *)mmap.base_pa, mmap.size);

		/*
		 * Unmap destination region unless it is the SPM<->SP buffer,
		 * which must be used by SPM.
		 */
		if (memtype != RD_MEM_NORMAL_SPM_SP_SHARED_MEM) {
			rc = mmap_remove_dynamic_region(rd_base_pa, rd_size);
			if (rc != 0) {
				ERROR("Unable to remove region at EL3: %d\n", rc);
				panic();
			}
		}
	}
}

void sp_map_memory_regions(sp_context_t *sp_ctx)
{
	/* This region contains the exception vectors used at S-EL1. */
	const mmap_region_t sel1_exception_vectors =
		MAP_REGION_FLAT(SPM_SHIM_EXCEPTIONS_START,
				SPM_SHIM_EXCEPTIONS_SIZE,
				MT_CODE | MT_SECURE | MT_PRIVILEGED);

	mmap_add_region_ctx(sp_ctx->xlat_ctx_handle,
			    &sel1_exception_vectors);

	struct sp_rd_sect_mem_region *rdmem;

	for (rdmem = sp_ctx->rd.mem_region; rdmem != NULL; rdmem = rdmem->next) {
		map_rdmem(sp_ctx, rdmem);
	}

	init_xlat_tables_ctx(sp_ctx->xlat_ctx_handle);
}

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
		return SPM_INVALID_PARAMETER;
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

	return (ret == 0) ? SPM_SUCCESS : SPM_INVALID_PARAMETER;
}

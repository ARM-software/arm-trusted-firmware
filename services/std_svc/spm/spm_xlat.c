/*
 * Copyright (c) 2018-2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch.h>
#include <arch_features.h>
#include <arch_helpers.h>
#include <assert.h>
#include <errno.h>
#include <string.h>

#include <platform_def.h>

#include <lib/object_pool.h>
#include <lib/utils.h>
#include <lib/utils_def.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <plat/common/platform.h>
#include <services/sp_res_desc.h>

#include "spm_private.h"
#include "spm_shim_private.h"

/*******************************************************************************
 * Instantiation of translation table context
 ******************************************************************************/

/* Place translation tables by default along with the ones used by BL31. */
#ifndef PLAT_SP_IMAGE_XLAT_SECTION_NAME
#define PLAT_SP_IMAGE_XLAT_SECTION_NAME	"xlat_table"
#endif

/*
 * Allocate elements of the translation contexts for the Secure Partitions.
 */

/* Allocate an array of mmap_region per partition. */
static struct mmap_region sp_mmap_regions[PLAT_SP_IMAGE_MMAP_REGIONS + 1]
	[PLAT_SPM_MAX_PARTITIONS];
static OBJECT_POOL(sp_mmap_regions_pool, sp_mmap_regions,
	sizeof(mmap_region_t) * (PLAT_SP_IMAGE_MMAP_REGIONS + 1),
	PLAT_SPM_MAX_PARTITIONS);

/* Allocate individual translation tables. */
static uint64_t sp_xlat_tables[XLAT_TABLE_ENTRIES]
	[(PLAT_SP_IMAGE_MAX_XLAT_TABLES + 1) * PLAT_SPM_MAX_PARTITIONS]
	__aligned(XLAT_TABLE_SIZE) __section(PLAT_SP_IMAGE_XLAT_SECTION_NAME);
static OBJECT_POOL(sp_xlat_tables_pool, sp_xlat_tables,
	XLAT_TABLE_ENTRIES * sizeof(uint64_t),
	(PLAT_SP_IMAGE_MAX_XLAT_TABLES + 1) * PLAT_SPM_MAX_PARTITIONS);

/* Allocate arrays. */
static int sp_xlat_mapped_regions[PLAT_SP_IMAGE_MAX_XLAT_TABLES]
	[PLAT_SPM_MAX_PARTITIONS];
static OBJECT_POOL(sp_xlat_mapped_regions_pool, sp_xlat_mapped_regions,
	sizeof(int) * PLAT_SP_IMAGE_MAX_XLAT_TABLES, PLAT_SPM_MAX_PARTITIONS);

/* Allocate individual contexts. */
static xlat_ctx_t sp_xlat_ctx[PLAT_SPM_MAX_PARTITIONS];
static OBJECT_POOL(sp_xlat_ctx_pool, sp_xlat_ctx, sizeof(xlat_ctx_t),
	PLAT_SPM_MAX_PARTITIONS);

/* Get handle of Secure Partition translation context */
void spm_sp_xlat_context_alloc(sp_context_t *sp_ctx)
{
	/* Allocate xlat context elements */

	xlat_ctx_t *ctx = pool_alloc(&sp_xlat_ctx_pool);

	struct mmap_region *mmap = pool_alloc(&sp_mmap_regions_pool);

	uint64_t *base_table = pool_alloc(&sp_xlat_tables_pool);
	uint64_t **tables = pool_alloc_n(&sp_xlat_tables_pool,
					PLAT_SP_IMAGE_MAX_XLAT_TABLES);

	int *mapped_regions = pool_alloc(&sp_xlat_mapped_regions_pool);

	/* Calculate the size of the virtual address space needed */

	uintptr_t va_size = 0U;
	struct sp_rd_sect_mem_region *rdmem;

	for (rdmem = sp_ctx->rd.mem_region; rdmem != NULL; rdmem = rdmem->next) {
		uintptr_t end_va = (uintptr_t)rdmem->base +
				   (uintptr_t)rdmem->size;

		if (end_va > va_size)
			va_size = end_va;
	}

	if (va_size == 0U) {
		ERROR("No regions in resource description.\n");
		panic();
	}

	/*
	 * Get the power of two that is greater or equal to the top VA. The
	 * values of base and size in the resource description are 32-bit wide
	 * so the values will never overflow when using a uintptr_t.
	 */
	if (!IS_POWER_OF_TWO(va_size)) {
		va_size = 1ULL <<
			((sizeof(va_size) * 8) - __builtin_clzll(va_size));
	}

	if (va_size > PLAT_VIRT_ADDR_SPACE_SIZE) {
		ERROR("Resource description requested too much virtual memory.\n");
		panic();
	}

	uintptr_t min_va_size;

	/* The following sizes are only valid for 4KB pages */
	assert(PAGE_SIZE == (4U * 1024U));

	if (is_armv8_4_ttst_present()) {
		VERBOSE("Using ARMv8.4-TTST\n");
		min_va_size = 1ULL << (64 - TCR_TxSZ_MAX_TTST);
	} else {
		min_va_size = 1ULL << (64 - TCR_TxSZ_MAX);
	}

	if (va_size < min_va_size) {
		va_size = min_va_size;
	}

	/* Initialize xlat context */

	xlat_setup_dynamic_ctx(ctx, PLAT_PHY_ADDR_SPACE_SIZE - 1ULL,
			       va_size - 1ULL, mmap,
			       PLAT_SP_IMAGE_MMAP_REGIONS, tables,
			       PLAT_SP_IMAGE_MAX_XLAT_TABLES, base_table,
			       EL1_EL0_REGIME, mapped_regions);

	sp_ctx->xlat_ctx_handle = ctx;
};

/*******************************************************************************
 * Translation table context used for S-EL1 exception vectors
 ******************************************************************************/

REGISTER_XLAT_CONTEXT2(spm_sel1, SPM_SHIM_MMAP_REGIONS, SPM_SHIM_XLAT_TABLES,
		SPM_SHIM_XLAT_VIRT_ADDR_SPACE_SIZE, PLAT_PHY_ADDR_SPACE_SIZE,
		EL1_EL0_REGIME, PLAT_SP_IMAGE_XLAT_SECTION_NAME);

void spm_exceptions_xlat_init_context(void)
{
	/* This region contains the exception vectors used at S-EL1. */
	mmap_region_t sel1_exception_vectors =
		MAP_REGION(SPM_SHIM_EXCEPTIONS_PTR,
			   0x0UL,
			   SPM_SHIM_EXCEPTIONS_SIZE,
			   MT_CODE | MT_SECURE | MT_PRIVILEGED);

	mmap_add_region_ctx(&spm_sel1_xlat_ctx,
			    &sel1_exception_vectors);

	init_xlat_tables_ctx(&spm_sel1_xlat_ctx);
}

uint64_t *spm_exceptions_xlat_get_base_table(void)
{
	return spm_sel1_xlat_ctx.base_table;
}

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

	if (rd_size == 0U) {
		VERBOSE("Memory region '%s' is empty. Ignored.\n", rdmem->name);
		return;
	}

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
	struct sp_rd_sect_mem_region *rdmem;

	for (rdmem = sp_ctx->rd.mem_region; rdmem != NULL; rdmem = rdmem->next) {
		map_rdmem(sp_ctx, rdmem);
	}

	init_xlat_tables_ctx(sp_ctx->xlat_ctx_handle);
}

/*
 * Copyright (c) 2018-2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>

#include <platform_def.h>

#include <common/debug.h>
#include <plat/common/platform.h>
#include <tools_share/sptool.h>

static unsigned int sp_next;

/*******************************************************************************
 * Platform handler get the address of a Secure Partition and its resource
 * description blob. It iterates through all SPs detected by the platform. If
 * there is information for another SP, it returns 0. If there are no more SPs,
 * it returns -1.
 ******************************************************************************/
int plat_spm_sp_get_next_address(void **sp_base, size_t *sp_size,
				 void **rd_base, size_t *rd_size)
{
	assert((sp_base != NULL) && (sp_size != NULL));
	assert((rd_base != NULL) && (rd_base != NULL));

	const uint64_t *pkg_base = (uint64_t *)PLAT_SP_PACKAGE_BASE;

	struct sp_pkg_header *pkg_header = (struct sp_pkg_header *)pkg_base;

	if (sp_next == 0) {
		if (pkg_header->version != 0x1) {
			ERROR("SP package has an unsupported version 0x%llx\n",
			      pkg_header->version);
			panic();
		}
	}

	if (sp_next >= pkg_header->number_of_sp) {
		/* No more partitions in the package */
		return -1;
	}

	const struct sp_pkg_entry *entry_list =
		(const struct sp_pkg_entry *)((uintptr_t)pkg_base
					       + sizeof(struct sp_pkg_header));

	const struct sp_pkg_entry *entry = &(entry_list[sp_next]);

	uint64_t sp_offset = entry->sp_offset;
	uint64_t rd_offset = entry->rd_offset;

	uintptr_t pkg_sp_base = ((uintptr_t)PLAT_SP_PACKAGE_BASE + sp_offset);
	uintptr_t pkg_rd_base = ((uintptr_t)PLAT_SP_PACKAGE_BASE + rd_offset);

	uint64_t pkg_sp_size = entry->sp_size;
	uint64_t pkg_rd_size = entry->rd_size;

	uintptr_t pkg_end = (uintptr_t)PLAT_SP_PACKAGE_BASE
			  + (uintptr_t)PLAT_SP_PACKAGE_SIZE - 1U;

	/*
	 * Check for overflows. The package header isn't trusted, so assert()
	 * can't be used here.
	 */

	uintptr_t pkg_sp_end = pkg_sp_base + pkg_sp_size - 1U;
	uintptr_t pkg_rd_end = pkg_rd_base + pkg_rd_size - 1U;

	if ((pkg_sp_end > pkg_end) || (pkg_sp_end < pkg_sp_base)) {
		ERROR("Invalid Secure Partition size (0x%llx)\n", pkg_sp_size);
		panic();
	}

	if ((pkg_rd_end > pkg_end) || (pkg_rd_end < pkg_rd_base)) {
		ERROR("Invalid Resource Description blob size (0x%llx)\n",
		      pkg_rd_size);
		panic();
	}

	/* Return location of the binaries. */

	*sp_base = (void *)pkg_sp_base;
	*sp_size = pkg_sp_size;
	*rd_base = (void *)pkg_rd_base;
	*rd_size = pkg_rd_size;

	sp_next++;

	return 0;
}

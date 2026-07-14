/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_TOC_H
#define SMEM_TOC_H

#include <stdint.h>

#include "smem_type.h"

/* Value placed in ToC identifier field */
#define SMEM_TOC_IDENTIFIER	0x434F5424 /* $TOC */

/*
 * Define the size of the Table of Contents partition.
 * Like all partitions, it must be a 4kB multiple.
 */
#define SMEM_TOC_SIZE	(4 * 1024)

/* Hosts bitmap size */
#define SMEM_TOC_HOSTS_BITMAP_ARRAY_SIZE	2

/* Max number of exclusions per ToC entry */
#define SMEM_TOC_MAX_EXCLUSIONS	4

/* Maximum allowed number of partitions */
#define SMEM_TOC_MAX_SMEM_PARTITIONS	60

/*
 * Entry in the SMEM Table of Contents
 */
struct smem_toc_entry {
	/* Offset in bytes from SMEM base of the region */
	uint32_t offset;
	/* Size in bytes of the region */
	uint32_t size;
	/* Flags for this region */
	uint32_t flags;
	/*
	 * IDs for the 2 subsystems which have access to this partition.
	 * Order does not matter.
	 * For the entry which describes the TOC itself, these are both set to
	 * SMEM_INVALID_HOST.
	 * Use uint16_t, rather than enum type, to ensure size.
	 */
	uint16_t host0;
	uint16_t host1;
	/*
	 * Lowest common multiple of cacheline sizes for both endpoints. For example,
	 * if Proc0 has cacheline size of 32 and Proc1 has cacheline size of 64,
	 * this value is set to 64
	 */
	uint32_t size_cacheline;
	/* Subsystems bitmap */
	uint32_t hosts_bitmap[SMEM_TOC_HOSTS_BITMAP_ARRAY_SIZE];
	uint32_t reserved[1];
	/*
	 * Sizes of sub ranges that are part of the region, but are excluded from
	 * the SMEM heap. These are allocated from the end of the region starting
	 * with sizes[0]. Set to 0 when not used.
	 */
	uint32_t exclusion_sizes[SMEM_TOC_MAX_EXCLUSIONS];
};

/*
 * Header of the SMEM Table of Contents
 */
struct smem_toc {
	/*
	 * Identifier is a constant for use in debugging and identifying this struct
	 * in a binary capture. Set to SMEM_TOC_IDENTIFIER.
	 */
	uint32_t identifier;
	/* Version number */
	uint32_t version;
	/* Number of entries in the table */
	uint32_t num_entries;
	/* Minor version number */
	uint32_t minor_version;
	uint32_t reserved[4];
	/* Zero or more entries follow */
	struct smem_toc_entry entry[];
};

#endif /* SMEM_TOC_H */

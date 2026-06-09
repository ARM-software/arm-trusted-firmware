/*
 * Copyright (c) 2026, Qualcomm Technologies, Inc. and/or its subsidiaries.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef SMEM_PARTITION_H
#define SMEM_PARTITION_H

#include "smem.h"
#include "smem_internal.h"
#include "smem_toc.h"

/* Partition Identifier: "$PRT" */
#define SMEM_PARTITION_HEADER_ID	0x54525024

/* Return the nearest multiple of y at or above x */
#define ROUND_UP(x, y)	((((x) + (y) - 1) / (y)) * (y))

/* Number of bytes of padding based on cacheline sizes */
#define SMEM_PARTITION_ITEM_PADDING(size_cl)                                  \
	(ROUND_UP(sizeof(struct smem_partition_allocation_header), (size_cl)) - \
	 sizeof(struct smem_partition_allocation_header))

/* Canary placed in the allocation header for each item */
#define SMEM_ALLOC_HDR_CANARY	0xa5a5

/*
 * Header for the SMEM partition, sits at the base of every partition in SMEM
 */
struct smem_partition_header {
	uint32_t identifier;
	/*
	 * IDs for the 2 subsystems which have access to this partition.
	 * Order does not matter.
	 * Use uint16_t, rather than enum type, to ensure size.
	 */
	uint16_t host0;
	uint16_t host1;
	/* Size of the partition, in bytes, not including the exclusion ranges */
	uint32_t size;
	/* Offset of the byte following the last uncached allocation */
	uint32_t offset_free_uncached;
	/* Offset of the byte following the last cached allocation */
	uint32_t offset_free_cached;
	/* Subsystems bitmap */
	uint32_t hosts_bitmap[SMEM_TOC_HOSTS_BITMAP_ARRAY_SIZE];
	uint32_t reserved[1];
};

/*
 * This structure is the header for each SMEM allocation in the
 * protected partition.
 */
struct smem_partition_allocation_header {
	/*
	 * Canary value.  Can be used to detect corruption.
	 * Set to 0xa5a5
	 */
	uint16_t canary;
	/*
	 * SMEM item ID.
	 * Use uint16_t here, rather than enum, to ensure size.
	 */
	uint16_t smem_type;
	/*
	 * Size of the allocated item in SMEM, in bytes.
	 * Includes padding_data bytes.
	 */
	uint32_t size;
	/*
	 * Size of data padding, in bytes.
	 * If the requested size was padded (rounded up), this is the
	 * amount it was rounded.
	 */
	uint16_t padding_data;
	/* Size of header padding, in bytes. */
	uint16_t padding_header;
	/* SMEM Hosts */
	uint16_t host0;
	uint16_t host1;
};

/*
 * State of the SMEM partition (resides in local memory)
 */
struct smem_partition_info {
	/* Pointer to the header for the partition which sits at the base */
	struct smem_partition_header *header;
	/* IDs for the 2 subsystems which have access to this partition. */
	uint16_t host0;
	uint16_t host1;
	/* Size of the partition, in bytes, not including the exclusion ranges */
	uint32_t size;
	/* Lowest common multiple of the cacheline sizes for this edge */
	uint32_t size_cacheline;
	/*
	 * Offset of the byte following the last allocation from the uncached side.
	 * Compared against the value in SMEM to determine if the remote endpoint
	 * has allocated from the heap.
	 */
	uint32_t offset_free_uncached;
	/*
	 * Offset of the byte following the last allocation from the cached side.
	 * Compared against the value in SMEM to determine if the remote endpoint
	 * has allocated from the heap.
	 */
	uint32_t offset_free_cached;
};

extern const struct smem_funcs smem_part_funcs;

/*
 * Initialize the SMEM partitioning structures.
 *
 * This function checks for the existence of the SMEM table of contents and
 * then reads it to determine partition locations and size. Data structures
 * are initialized to enable SMEM allocation from protected partitions.
 */
void smem_part_init(void);

/*
 * Request the address and size of an allocated buffer in shared memory.
 *
 * If found, sets the buffer and size fields of the params struct.
 * params.smem_type must be set to the ID to search for. params.remote_host
 * must be set to the ID of the remote host of the partition.
 *
 * This function updates the size_remaining field for the partition.
 */
int32_t smem_part_get_addr_ex(struct smem_alloc_params *params);

/*
 * Request a pointer to an already allocated buffer in shared memory.
 *
 * Returns the address and size of the allocated buffer. Newly-allocated
 * shared memory buffers, which have never been allocated on any processor,
 * are guaranteed to be zeroed.
 */
void *smem_part_get_addr(enum smem_mem_type smem_type, uint32_t *buf_size);

#endif /* SMEM_PARTITION_H */

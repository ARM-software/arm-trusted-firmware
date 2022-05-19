/*
 * Copyright (c) 2022, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef EL3_SPMC_FFA_MEM_H
#define EL3_SPMC_FFA_MEM_H

#include <assert.h>

/*
 * Subset of Arm Firmware Framework for Armv8-A
 * (https://developer.arm.com/docs/den0077/a) needed for shared memory.
 */

/**
 * typedef ffa_endpoint_id16_t - Endpoint ID
 *
 * Current implementation only supports VM IDs. FF-A spec also support stream
 * endpoint ids.
 */
typedef uint16_t ffa_endpoint_id16_t;

/**
 * struct ffa_cons_mrd - Constituent memory region descriptor
 * @address:
 *         Start address of contiguous memory region. Must be 4K page aligned.
 * @page_count:
 *         Number of 4K pages in region.
 * @reserved_12_15:
 *         Reserve bytes 12-15 to pad struct size to 16 bytes.
 */
struct ffa_cons_mrd {
	uint64_t address;
	uint32_t page_count;
	uint32_t reserved_12_15;
};
CASSERT(sizeof(struct ffa_cons_mrd) == 16, assert_ffa_cons_mrd_size_mismatch);

/**
 * struct ffa_comp_mrd - Composite memory region descriptor
 * @total_page_count:
 *         Number of 4k pages in memory region. Must match sum of
 *         @address_range_array[].page_count.
 * @address_range_count:
 *         Number of entries in @address_range_array.
 * @reserved_8_15:
 *         Reserve bytes 8-15 to pad struct size to 16 byte alignment and
 *         make @address_range_array 16 byte aligned.
 * @address_range_array:
 *         Array of &struct ffa_cons_mrd entries.
 */
struct ffa_comp_mrd {
	uint32_t total_page_count;
	uint32_t address_range_count;
	uint64_t reserved_8_15;
	struct ffa_cons_mrd address_range_array[];
};
CASSERT(sizeof(struct ffa_comp_mrd) == 16, assert_ffa_comp_mrd_size_mismatch);

/**
 * typedef ffa_mem_attr8_t - Memory region attributes v1.0.
 * typedef ffa_mem_attr16_t - Memory region attributes v1.1.
 *
 * * @FFA_MEM_ATTR_NS_BIT:
 *     Memory security state.
 * * @FFA_MEM_ATTR_DEVICE_NGNRNE:
 *     Device-nGnRnE.
 * * @FFA_MEM_ATTR_DEVICE_NGNRE:
 *     Device-nGnRE.
 * * @FFA_MEM_ATTR_DEVICE_NGRE:
 *     Device-nGRE.
 * * @FFA_MEM_ATTR_DEVICE_GRE:
 *     Device-GRE.
 * * @FFA_MEM_ATTR_NORMAL_MEMORY_UNCACHED
 *     Normal memory. Non-cacheable.
 * * @FFA_MEM_ATTR_NORMAL_MEMORY_CACHED_WB
 *     Normal memory. Write-back cached.
 * * @FFA_MEM_ATTR_NON_SHAREABLE
 *     Non-shareable. Combine with FFA_MEM_ATTR_NORMAL_MEMORY_*.
 * * @FFA_MEM_ATTR_OUTER_SHAREABLE
 *     Outer Shareable. Combine with FFA_MEM_ATTR_NORMAL_MEMORY_*.
 * * @FFA_MEM_ATTR_INNER_SHAREABLE
 *     Inner Shareable. Combine with FFA_MEM_ATTR_NORMAL_MEMORY_*.
 */
typedef uint8_t ffa_mem_attr8_t;
typedef uint16_t ffa_mem_attr16_t;
#define FFA_MEM_ATTR_NS_BIT			(0x1U << 6)
#define FFA_MEM_ATTR_DEVICE_NGNRNE		((1U << 4) | (0x0U << 2))
#define FFA_MEM_ATTR_DEVICE_NGNRE		((1U << 4) | (0x1U << 2))
#define FFA_MEM_ATTR_DEVICE_NGRE		((1U << 4) | (0x2U << 2))
#define FFA_MEM_ATTR_DEVICE_GRE			((1U << 4) | (0x3U << 2))
#define FFA_MEM_ATTR_NORMAL_MEMORY_UNCACHED	((2U << 4) | (0x1U << 2))
#define FFA_MEM_ATTR_NORMAL_MEMORY_CACHED_WB	((2U << 4) | (0x3U << 2))
#define FFA_MEM_ATTR_NON_SHAREABLE		(0x0U << 0)
#define FFA_MEM_ATTR_OUTER_SHAREABLE		(0x2U << 0)
#define FFA_MEM_ATTR_INNER_SHAREABLE		(0x3U << 0)

/**
 * typedef ffa_mem_perm8_t - Memory access permissions
 *
 * * @FFA_MEM_ATTR_RO
 *     Request or specify read-only mapping.
 * * @FFA_MEM_ATTR_RW
 *     Request or allow read-write mapping.
 * * @FFA_MEM_PERM_NX
 *     Deny executable mapping.
 * * @FFA_MEM_PERM_X
 *     Request executable mapping.
 */
typedef uint8_t ffa_mem_perm8_t;
#define FFA_MEM_PERM_RO		(1U << 0)
#define FFA_MEM_PERM_RW		(1U << 1)
#define FFA_MEM_PERM_NX		(1U << 2)
#define FFA_MEM_PERM_X		(1U << 3)

/**
 * typedef ffa_mem_flag8_t - Endpoint memory flags
 *
 * * @FFA_MEM_FLAG_NON_RETRIEVAL_BORROWER
 *     Non-retrieval Borrower. Memory region must not be or was not retrieved on
 *     behalf of this endpoint.
 */
typedef uint8_t ffa_mem_flag8_t;
#define FFA_MEM_FLAG_NON_RETRIEVAL_BORROWER	(1U << 0)

/**
 * typedef ffa_mtd_flag32_t - Memory transaction descriptor flags
 *
 * * @FFA_MTD_FLAG_ZERO_MEMORY
 *     Zero memory after unmapping from sender (must be 0 for share).
 * * @FFA_MTD_FLAG_TIME_SLICING
 *     Not supported by this implementation.
 * * @FFA_MTD_FLAG_ZERO_MEMORY_AFTER_RELINQUISH
 *     Zero memory after unmapping from borrowers (must be 0 for share).
 * * @FFA_MTD_FLAG_TYPE_MASK
 *     Bit-mask to extract memory management transaction type from flags.
 * * @FFA_MTD_FLAG_TYPE_SHARE_MEMORY
 *     Share memory transaction flag.
 *     Used by @SMC_FC_FFA_MEM_RETRIEVE_RESP to indicate that memory came from
 *     @SMC_FC_FFA_MEM_SHARE and by @SMC_FC_FFA_MEM_RETRIEVE_REQ to specify that
 *     it must have.
 * * @FFA_MTD_FLAG_ADDRESS_RANGE_ALIGNMENT_HINT_MASK
 *     Not supported by this implementation.
 */
typedef uint32_t ffa_mtd_flag32_t;
#define FFA_MTD_FLAG_ZERO_MEMORY			(1U << 0)
#define FFA_MTD_FLAG_TIME_SLICING			(1U << 1)
#define FFA_MTD_FLAG_ZERO_MEMORY_AFTER_RELINQUISH	(1U << 2)
#define FFA_MTD_FLAG_TYPE_MASK				(3U << 3)
#define FFA_MTD_FLAG_TYPE_SHARE_MEMORY			(1U << 3)
#define FFA_MTD_FLAG_TYPE_LEND_MEMORY			(1U << 4)
#define FFA_MTD_FLAG_ADDRESS_RANGE_ALIGNMENT_HINT_MASK	(0x1FU << 5)

/**
 * struct ffa_mapd - Memory access permissions descriptor
 * @endpoint_id:
 *         Endpoint id that @memory_access_permissions and @flags apply to.
 *         (&typedef ffa_endpoint_id16_t).
 * @memory_access_permissions:
 *         FFA_MEM_PERM_* values or'ed together (&typedef ffa_mem_perm8_t).
 * @flags:
 *         FFA_MEM_FLAG_* values or'ed together (&typedef ffa_mem_flag8_t).
 */
struct ffa_mapd {
	ffa_endpoint_id16_t endpoint_id;
	ffa_mem_perm8_t memory_access_permissions;
	ffa_mem_flag8_t flags;
};
CASSERT(sizeof(struct ffa_mapd) == 4, assert_ffa_mapd_size_mismatch);

/**
 * struct ffa_emad_v1_0 - Endpoint memory access descriptor.
 * @mapd:  &struct ffa_mapd.
 * @comp_mrd_offset:
 *         Offset of &struct ffa_comp_mrd from start of &struct ffa_mtd_v1_0.
 * @reserved_8_15:
 *         Reserved bytes 8-15. Must be 0.
 */
struct ffa_emad_v1_0 {
	struct ffa_mapd mapd;
	uint32_t comp_mrd_offset;
	uint64_t reserved_8_15;
};
CASSERT(sizeof(struct ffa_emad_v1_0) == 16, assert_ffa_emad_v1_0_size_mismatch);

/**
 * struct ffa_mtd_v1_0 - Memory transaction descriptor.
 * @sender_id:
 *         Sender endpoint id.
 * @memory_region_attributes:
 *         FFA_MEM_ATTR_* values or'ed together (&typedef ffa_mem_attr8_t).
 * @reserved_3:
 *         Reserved bytes 3. Must be 0.
 * @flags:
 *         FFA_MTD_FLAG_* values or'ed together (&typedef ffa_mtd_flag32_t).
 * @handle:
 *         Id of shared memory object. Must be 0 for MEM_SHARE or MEM_LEND.
 * @tag:   Client allocated tag. Must match original value.
 * @reserved_24_27:
 *         Reserved bytes 24-27. Must be 0.
 * @emad_count:
 *         Number of entries in @emad.
 * @emad:
 *         Endpoint memory access descriptor array (see @struct ffa_emad_v1_0).
 */
struct ffa_mtd_v1_0 {
	ffa_endpoint_id16_t sender_id;
	ffa_mem_attr8_t memory_region_attributes;
	uint8_t reserved_3;
	ffa_mtd_flag32_t flags;
	uint64_t handle;
	uint64_t tag;
	uint32_t reserved_24_27;
	uint32_t emad_count;
	struct ffa_emad_v1_0 emad[];
};
CASSERT(sizeof(struct ffa_mtd_v1_0) == 32, assert_ffa_mtd_size_v1_0_mismatch);

/**
 * struct ffa_mtd - Memory transaction descriptor for FF-A v1.1.
 * @sender_id:
 *         Sender endpoint id.
 * @memory_region_attributes:
 *         FFA_MEM_ATTR_* values or'ed together (&typedef ffa_mem_attr16_t).
 * @flags:
 *         FFA_MTD_FLAG_* values or'ed together (&typedef ffa_mtd_flag32_t).
 * @handle:
 *         Id of shared memory object. Must be 0 for MEM_SHARE or MEM_LEND.
 * @tag:   Client allocated tag. Must match original value.
 * @emad_size:
 *         Size of the emad descriptor.
 * @emad_count:
 *         Number of entries in the emad array.
 * @emad_offset:
 *         Offset from the beginning of the descriptor to the location of the
 *         memory access descriptor array (see @struct ffa_emad_v1_0).
 * @reserved_36_39:
 *         Reserved bytes 36-39. Must be 0.
 * @reserved_40_47:
 *         Reserved bytes 44-47. Must be 0.
 */
struct ffa_mtd {
	ffa_endpoint_id16_t sender_id;
	ffa_mem_attr16_t memory_region_attributes;
	ffa_mtd_flag32_t flags;
	uint64_t handle;
	uint64_t tag;
	uint32_t emad_size;
	uint32_t emad_count;
	uint32_t emad_offset;
	uint32_t reserved_36_39;
	uint64_t reserved_40_47;
};
CASSERT(sizeof(struct ffa_mtd) == 48, assert_ffa_mtd_size_mismatch);

#endif /* EL3_SPMC_FFA_MEM_H */

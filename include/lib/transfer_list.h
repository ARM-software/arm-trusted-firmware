/*
 * Copyright (c) 2023-2024, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TRANSFER_LIST_H
#define __TRANSFER_LIST_H

#include <stdbool.h>
#include <stdint.h>

#include <common/ep_info.h>
#include <lib/utils_def.h>

#define TRANSFER_LIST_SIGNATURE U(0x4a0fb10b)
#define TRANSFER_LIST_VERSION U(0x0001)

/*
 * Init value of maximum alignment required by any TE data in the TL
 * specified as a power of two
 */
#define TRANSFER_LIST_INIT_MAX_ALIGN U(3)

/* Alignment required by TE header start address, in bytes */
#define TRANSFER_LIST_GRANULE U(8)

/*
 * Version of the register convention used.
 * Set to 1 for both AArch64 and AArch32 according to fw handoff spec v0.9
 */
#define REGISTER_CONVENTION_VERSION_SHIFT_64	UL(32)
#define REGISTER_CONVENTION_VERSION_SHIFT_32	UL(24)
#define REGISTER_CONVENTION_VERSION_MASK	UL(0xff)
#define REGISTER_CONVENTION_VERSION 	UL(1)

#define TRANSFER_LIST_HANDOFF_X1_VALUE(__version) 	\
	((TRANSFER_LIST_SIGNATURE &	\
	((1UL << REGISTER_CONVENTION_VERSION_SHIFT_64) - 1)) | 	\
	(((__version) & REGISTER_CONVENTION_VERSION_MASK) <<	\
	 REGISTER_CONVENTION_VERSION_SHIFT_64))

#define TRANSFER_LIST_HANDOFF_R1_VALUE(__version) 	\
	((TRANSFER_LIST_SIGNATURE &	\
	((1UL << REGISTER_CONVENTION_VERSION_SHIFT_32) - 1)) | 	\
	(((__version) & REGISTER_CONVENTION_VERSION_MASK) <<	\
	 REGISTER_CONVENTION_VERSION_SHIFT_32))

#ifndef __ASSEMBLER__

#define TL_FLAGS_HAS_CHECKSUM BIT(0)

enum transfer_list_tag_id {
	TL_TAG_EMPTY = 0,
	TL_TAG_FDT = 1,
	TL_TAG_HOB_BLOCK = 2,
	TL_TAG_HOB_LIST = 3,
	TL_TAG_ACPI_TABLE_AGGREGATE = 4,
	TL_TAG_OPTEE_PAGABLE_PART = 0x100,
	TL_TAG_DT_SPMC_MANIFEST = 0x101,
	TL_TAG_EXEC_EP_INFO64 = 0x102,
	TL_TAG_TB_FW_CONFIG = 0x103,
	TL_TAG_SRAM_LAYOUT64 = 0x104,
};

enum transfer_list_ops {
	TL_OPS_NON, /* invalid for any operation */
	TL_OPS_ALL, /* valid for all operations */
	TL_OPS_RO, /* valid for read only */
	TL_OPS_CUS, /* abort or switch to special code to interpret */
};

struct transfer_list_header {
	uint32_t signature;
	uint8_t checksum;
	uint8_t version;
	uint8_t hdr_size;
	uint8_t alignment; /* max alignment of TE data */
	uint32_t size; /* TL header + all TEs */
	uint32_t max_size;
	uint32_t flags;
	uint32_t reserved; /* spare bytes */
	/*
	 * Commented out element used to visualize dynamic part of the
	 * data structure.
	 *
	 * Note that struct transfer_list_entry also is dynamic in size
	 * so the elements can't be indexed directly but instead must be
	 * traversed in order
	 *
	 * struct transfer_list_entry entries[];
	 */
};

struct __attribute__((packed)) transfer_list_entry {
	uint32_t tag_id : 24;
	uint8_t hdr_size;
	uint32_t data_size;
	/*
	 * Commented out element used to visualize dynamic part of the
	 * data structure.
	 *
	 * Note that padding is added at the end of @data to make to reach
	 * a 8-byte boundary.
	 *
	 * uint8_t	data[ROUNDUP(data_size, 8)];
	 */
};

CASSERT(sizeof(struct transfer_list_entry) == U(0x8), assert_transfer_list_entry_size);

void transfer_list_dump(struct transfer_list_header *tl);
entry_point_info_t *
transfer_list_set_handoff_args(struct transfer_list_header *tl,
			       entry_point_info_t *ep_info);
struct transfer_list_header *transfer_list_init(void *addr, size_t max_size);

struct transfer_list_header *
transfer_list_relocate(struct transfer_list_header *tl, void *addr,
		       size_t max_size);
enum transfer_list_ops
transfer_list_check_header(const struct transfer_list_header *tl);

void transfer_list_update_checksum(struct transfer_list_header *tl);
bool transfer_list_verify_checksum(const struct transfer_list_header *tl);

bool transfer_list_set_data_size(struct transfer_list_header *tl,
				 struct transfer_list_entry *entry,
				 uint32_t new_data_size);

void *transfer_list_entry_data(struct transfer_list_entry *entry);
bool transfer_list_rem(struct transfer_list_header *tl,
		       struct transfer_list_entry *entry);

struct transfer_list_entry *transfer_list_add(struct transfer_list_header *tl,
					      uint32_t tag_id,
					      uint32_t data_size,
					      const void *data);

struct transfer_list_entry *
transfer_list_add_with_align(struct transfer_list_header *tl, uint32_t tag_id,
			     uint32_t data_size, const void *data,
			     uint8_t alignment);

struct transfer_list_entry *
transfer_list_next(struct transfer_list_header *tl,
		   struct transfer_list_entry *last);

struct transfer_list_entry *transfer_list_find(struct transfer_list_header *tl,
					       uint32_t tag_id);

#endif /*__ASSEMBLER__*/
#endif /*__TRANSFER_LIST_H*/

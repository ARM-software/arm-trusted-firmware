/*
 * Copyright (c) 2023, Linaro Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __TRANSFER_LIST_H
#define __TRANSFER_LIST_H

#include <stdbool.h>
#include <stdint.h>

#include <lib/utils_def.h>

#define	TRANSFER_LIST_SIGNATURE		U(0x006ed0ff)
#define TRANSFER_LIST_VERSION		U(0x0001)

// Init value of maximum alignment required by any TE data in the TL
// specified as a power of two
#define TRANSFER_LIST_INIT_MAX_ALIGN	U(3)

// alignment required by TE header start address, in bytes
#define TRANSFER_LIST_GRANULE		U(8)

// version of the register convention used.
// Set to 1 for both AArch64 and AArch32 according to fw handoff spec v0.9
#define REGISTER_CONVENTION_VERSION_MASK (1 << 24)

#ifndef __ASSEMBLER__

enum transfer_list_tag_id {
	TL_TAG_EMPTY = 0,
	TL_TAG_FDT = 1,
	TL_TAG_HOB_BLOCK = 2,
	TL_TAG_HOB_LIST = 3,
	TL_TAG_ACPI_TABLE_AGGREGATE = 4,
};

enum transfer_list_ops {
	TL_OPS_NON,	// invalid for any operation
	TL_OPS_ALL,	// valid for all operations
	TL_OPS_RO,	// valid for read only
	TL_OPS_CUS,	// either abort or switch to special code to interpret
};

struct transfer_list_header {
	uint32_t	signature;
	uint8_t		checksum;
	uint8_t		version;
	uint8_t		hdr_size;
	uint8_t		alignment;	// max alignment of TE data
	uint32_t	size;		// TL header + all TEs
	uint32_t	max_size;
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

struct transfer_list_entry {
	uint16_t	tag_id;
	uint8_t		reserved0;	// place holder
	uint8_t		hdr_size;
	uint32_t	data_size;
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

void transfer_list_dump(struct transfer_list_header *tl);
struct transfer_list_header *transfer_list_init(void *addr, size_t max_size);

struct transfer_list_header *transfer_list_relocate(struct transfer_list_header *tl,
						    void *addr, size_t max_size);
enum transfer_list_ops transfer_list_check_header(const struct transfer_list_header *tl);

void transfer_list_update_checksum(struct transfer_list_header *tl);
bool transfer_list_verify_checksum(const struct transfer_list_header *tl);

bool transfer_list_set_data_size(struct transfer_list_header *tl,
				 struct transfer_list_entry *entry,
				 uint32_t new_data_size);

void *transfer_list_entry_data(struct transfer_list_entry *entry);
bool transfer_list_rem(struct transfer_list_header *tl, struct transfer_list_entry *entry);

struct transfer_list_entry *transfer_list_add(struct transfer_list_header *tl,
					      uint16_t tag_id, uint32_t data_size,
					      const void *data);

struct transfer_list_entry *transfer_list_add_with_align(struct transfer_list_header *tl,
							 uint16_t tag_id, uint32_t data_size,
							 const void *data, uint8_t alignment);

struct transfer_list_entry *transfer_list_next(struct transfer_list_header *tl,
					       struct transfer_list_entry *last);

struct transfer_list_entry *transfer_list_find(struct transfer_list_header *tl,
					       uint16_t tag_id);

#endif /*__ASSEMBLER__*/
#endif /*__TRANSFER_LIST_H*/

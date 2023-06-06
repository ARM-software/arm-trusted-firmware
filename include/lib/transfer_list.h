/* SPDX-License-Identifier: BSD-2-Clause */
/*
 * Copyright (c) 2022, Linaro Limited
 */

#ifndef __KERNEL_TRANSFER_LIST_H
#define __KERNEL_TRANSFER_LIST_H

#include <export/lib/utils_def_exp.h>
#include <stdint.h>
#include <stdbool.h>

#define	TRANSFER_LIST_SIGNATURE		U(0x006ed0ff)
#define TRANSFER_LIST_VERSION		U(0x0001)

// Init value of maximum alignment required by any TE in the TL
// specified as a power of two
#define TRANSFER_LIST_INIT_MAX_ALIGN	U(3)

//#define TRANSFER_LIST_GRANULE		U(16)

#ifndef __ASSEMBLER__

enum transfer_list_tag_id {
	TL_TAG_EMPTY = 0,
	TL_TAG_FDT = 1,
	TL_TAG_HOB_BLOCK = 2,
	TL_TAG_HOB_LIST = 3,
	TL_TAG_ACPI_TABLE_AGGREGATE = 4,
};

struct transfer_list_header {
	uint32_t	signature;
	uint8_t		checksum;
	uint8_t		version;
	uint8_t		hdr_size;
	uint8_t		alignment;
	uint32_t	size;		// TL header + all TEs
	uint32_t	max_size;
	/*
	 * Commented out element used to visualze dynamic part of the
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
	uint8_t		tag_id;
	uint8_t		reserved0[2];
	uint8_t		hdr_size;
	uint32_t	data_size;
	/*
	 * Commented out element used to visualze dynamic part of the
	 * data structure.
	 *
	 * Note that padding is added at the end of @data to make to reach
	 * a 16-byte boundary.
	 *
	 * uint8_t	data[ROUNDUP(data_size, 16)];
	 */
};

struct transfer_list_header *transfer_list_init(void *p, size_t max_size);

bool transfer_list_relocate(struct transfer_list_header *tl, void **p, size_t max_size);

struct transfer_list_header *transfer_list_check_header(struct transfer_list_header *tl);

void transfer_list_update_checksum(struct transfer_list_header *tl);
bool transfer_list_verify_checksum(struct transfer_list_header *tl);

bool transfer_list_set_data_size(struct transfer_list_header *tl,
				 struct transfer_list_entry *entry,
				 uint32_t new_data_size);

bool transfer_list_grow_to_max_data_size(struct transfer_list_header *tl,
					 struct transfer_list_entry *entry);

static inline void *transfer_list_data(struct transfer_list_entry *entry)
{
	return (uint8_t *)entry + entry->hdr_size;
}

bool transfer_list_rem(struct transfer_list_header *tl, struct transfer_list_entry *entry);

struct transfer_list_entry *transfer_list_add(struct transfer_list_header *tl,
					 uint8_t tag_id, uint32_t data_size,
					 const void *data);

struct transfer_list_entry *transfer_list_add_with_align(struct transfer_list_header *tl,
					 uint8_t tag_id, uint32_t data_size, const void *data,
					 uint8_t alignment);

struct transfer_list_entry *transfer_list_find(struct transfer_list_header *tl,
					  uint8_t tag_id);

#endif /*__ASSEMBLER__*/
#endif /*__KERNEL_TRANSFER_LIST_H*/

/*
 * Copyright (c) 2016-2026, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <stdbool.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/partition/efi.h>
#include <drivers/partition/gpt.h>
#include <lib/utils.h>

static int unicode_to_ascii(const unsigned short *str_in, unsigned char *str_out)
{
	const uint8_t *name;
	int i;

	assert((str_in != NULL) && (str_out != NULL));

	name = (uint8_t *)str_in;

	assert(name[0] != '\0');

	/* check whether the unicode string is valid */
	for (i = 1; i < (EFI_NAMELEN << 1); i += 2) {
		if (name[i] != '\0') {
			return -EINVAL;
		}
	}
	/* convert the unicode string to ascii string */
	for (i = 0; i < (EFI_NAMELEN << 1); i += 2) {
		str_out[i >> 1] = name[i];
		if (name[i] == '\0') {
			break;
		}
	}
	return 0;
}

static bool is_zero_guid(const struct efi_guid *guid)
{
	static const struct efi_guid null_guid = NULL_GUID;

	return guidcmp(guid, &null_guid) == 0;
}

static bool gpt_entry_name_is_empty(const unsigned short *name)
{
	int i;

	for (i = 0; i < EFI_NAMELEN; i++) {
		if (name[i] != 0U) {
			return false;
		}
	}

	return true;
}

int parse_gpt_entry(const gpt_header_t *header, const gpt_entry_t *gpt_entry,
		    partition_entry_t *entry)
{
	int result;

	assert((header != NULL) && (gpt_entry != NULL) && (entry != NULL));

	if (is_zero_guid(&gpt_entry->type_uuid) &&
	    is_zero_guid(&gpt_entry->unique_uuid) &&
	    (gpt_entry->first_lba == 0U) &&
	    (gpt_entry->last_lba == 0U) &&
	    (gpt_entry->attr == 0U) &&
	    gpt_entry_name_is_empty(gpt_entry->name)) {
		return -ENOENT;
	}

	if (is_zero_guid(&gpt_entry->type_uuid) ||
	    is_zero_guid(&gpt_entry->unique_uuid) ||
	    (gpt_entry->first_lba == 0U) ||
	    (gpt_entry->last_lba == 0U) ||
	    (gpt_entry->first_lba > gpt_entry->last_lba) ||
	    (gpt_entry->first_lba < header->first_lba) ||
	    (gpt_entry->last_lba > header->last_lba)) {
		return -EINVAL;
	}

	zeromem(entry, sizeof(partition_entry_t));
	result = unicode_to_ascii(gpt_entry->name, (uint8_t *)entry->name);
	if (result != 0) {
		return result;
	}
	entry->start = (uint64_t)gpt_entry->first_lba *
		       PLAT_PARTITION_BLOCK_SIZE;
	entry->length = (uint64_t)(gpt_entry->last_lba -
				   gpt_entry->first_lba + 1) *
			PLAT_PARTITION_BLOCK_SIZE;
	guidcpy(&entry->part_guid, &gpt_entry->unique_uuid);
	guidcpy(&entry->type_guid, &gpt_entry->type_uuid);

	return 0;
}

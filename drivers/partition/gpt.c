/*
 * Copyright (c) 2016-2017, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>
#include <string.h>

#include <common/debug.h>
#include <drivers/partition/gpt.h>
#include <lib/utils.h>

static int unicode_to_ascii(unsigned short *str_in, unsigned char *str_out)
{
	uint8_t *name;
	int i;

	assert((str_in != NULL) && (str_out != NULL));

	name = (uint8_t *)str_in;

	assert(name[0] != '\0');

	/* check whether the unicode string is valid */
	for (i = 1; i < (EFI_NAMELEN << 1); i += 2) {
		if (name[i] != '\0')
			return -EINVAL;
	}
	/* convert the unicode string to ascii string */
	for (i = 0; i < (EFI_NAMELEN << 1); i += 2) {
		str_out[i >> 1] = name[i];
		if (name[i] == '\0')
			break;
	}
	return 0;
}

int parse_gpt_entry(gpt_entry_t *gpt_entry, partition_entry_t *entry)
{
	int result;

	assert((gpt_entry != NULL) && (entry != NULL));

	if ((gpt_entry->first_lba == 0) && (gpt_entry->last_lba == 0)) {
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
	return 0;
}

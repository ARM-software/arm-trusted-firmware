/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef GPT_H
#define GPT_H

#include <drivers/partition/partition.h>

#define PARTITION_TYPE_GPT		0xee
#define GPT_HEADER_OFFSET		PARTITION_BLOCK_SIZE
#define GPT_ENTRY_OFFSET		(GPT_HEADER_OFFSET +		\
					 PARTITION_BLOCK_SIZE)
#define GUID_LEN			16

#define GPT_SIGNATURE			"EFI PART"

typedef struct gpt_entry {
	unsigned char		type_uuid[GUID_LEN];
	unsigned char		unique_uuid[GUID_LEN];
	unsigned long long	first_lba;
	unsigned long long	last_lba;
	unsigned long long	attr;
	unsigned short		name[EFI_NAMELEN];
} gpt_entry_t;

typedef struct gpt_header {
	unsigned char		signature[8];
	unsigned int		revision;
	unsigned int		size;
	unsigned int		header_crc;
	unsigned int		reserved;
	unsigned long long	current_lba;
	unsigned long long	backup_lba;
	unsigned long long	first_lba;
	unsigned long long	last_lba;
	unsigned char		disk_uuid[16];
	/* starting LBA of array of partition entries */
	unsigned long long	part_lba;
	/* number of partition entries in array */
	unsigned int		list_num;
	/* size of a single partition entry (usually 128) */
	unsigned int		part_size;
	unsigned int		part_crc;
} gpt_header_t;

int parse_gpt_entry(gpt_entry_t *gpt_entry, partition_entry_t *entry);

#endif /* GPT_H */

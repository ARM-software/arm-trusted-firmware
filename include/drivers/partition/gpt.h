/*
 * Copyright (c) 2016, ARM Limited and Contributors. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of ARM nor the names of its contributors may be used
 * to endorse or promote products derived from this software without specific
 * prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __GPT_H__
#define __GPT_H__

#include <partition.h>

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

#endif	/* __GPT_H__ */

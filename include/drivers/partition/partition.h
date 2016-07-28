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

#ifndef __PARTITION_H__
#define __PARTITION_H__

#include <cassert.h>
#include <types.h>

#if !PLAT_PARTITION_MAX_ENTRIES
# define PLAT_PARTITION_MAX_ENTRIES	128
#endif	/* PLAT_PARTITION_MAX_ENTRIES */

CASSERT(PLAT_PARTITION_MAX_ENTRIES <= 128, assert_plat_partition_max_entries);

#define PARTITION_BLOCK_SIZE		512

#define EFI_NAMELEN			36

typedef struct partition_entry {
	uint64_t		start;
	uint64_t		length;
	char			name[EFI_NAMELEN];
} partition_entry_t;

typedef struct partition_entry_list {
	partition_entry_t	list[PLAT_PARTITION_MAX_ENTRIES];
	int			entry_count;
} partition_entry_list_t;

int load_partition_table(unsigned int image_id);
const partition_entry_t *get_partition_entry(const char *name);
const partition_entry_list_t *get_partition_entry_list(void);
void partition_init(unsigned int image_id);

#endif	/* __PARTITION_H__ */

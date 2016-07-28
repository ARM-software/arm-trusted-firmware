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

#include <assert.h>
#include <debug.h>
#include <errno.h>
#include <gpt.h>
#include <string.h>

static int unicode_to_ascii(unsigned short *str_in, unsigned char *str_out)
{
	uint8_t *name = (uint8_t *)str_in;
	int i;

	assert((str_in != NULL) && (str_out != NULL) && (name[0] != '\0'));

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

	assert((gpt_entry != 0) && (entry != 0));

	if ((gpt_entry->first_lba == 0) && (gpt_entry->last_lba == 0)) {
		return -EINVAL;
	}

	memset(entry, 0, sizeof(partition_entry_t));
	result = unicode_to_ascii(gpt_entry->name, (uint8_t *)entry->name);
	if (result != 0) {
		return result;
	}
	entry->start = (uint64_t)gpt_entry->first_lba * PARTITION_BLOCK_SIZE;
	entry->length = (uint64_t)(gpt_entry->last_lba -
				   gpt_entry->first_lba + 1) *
			PARTITION_BLOCK_SIZE;
	return 0;
}

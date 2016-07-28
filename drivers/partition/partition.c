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
#include <io_storage.h>
#include <gpt.h>
#include <mbr.h>
#include <partition.h>
#include <platform.h>
#include <string.h>

static uint8_t mbr_sector[PARTITION_BLOCK_SIZE];
partition_entry_list_t list;

#if LOG_LEVEL >= LOG_LEVEL_VERBOSE
static void dump_entries(int num)
{
	char name[EFI_NAMELEN];
	int i, j, len;

	VERBOSE("Partition table with %d entries:\n", num);
	for (i = 0; i < num; i++) {
		len = snprintf(name, EFI_NAMELEN, "%s", list.list[i].name);
		for (j = 0; j < EFI_NAMELEN - len - 1; j++) {
			name[len + j] = ' ';
		}
		name[EFI_NAMELEN - 1] = '\0';
		VERBOSE("%d: %s %lx-%lx\n", i + 1, name, list.list[i].start,
			list.list[i].start + list.list[i].length - 4);
	}
}
#else
#define dump_entries(num)	((void)num)
#endif

/*
 * Load the first sector that carries MBR header.
 * The MBR boot signature should be always valid whether it's MBR or GPT.
 */
static int load_mbr_header(uintptr_t image_handle, mbr_entry_t *mbr_entry)
{
	size_t bytes_read;
	uintptr_t offset;
	int result;

	assert(mbr_entry != NULL);
	/* MBR partition table is in LBA0. */
	result = io_seek(image_handle, IO_SEEK_SET, MBR_OFFSET);
	if (result != 0) {
		WARN("Failed to seek (%i)\n", result);
		return result;
	}
	result = io_read(image_handle, (uintptr_t)&mbr_sector,
			 PARTITION_BLOCK_SIZE, &bytes_read);
	if (result != 0) {
		WARN("Failed to read data (%i)\n", result);
		return result;
	}

	/* Check MBR boot signature. */
	if ((mbr_sector[PARTITION_BLOCK_SIZE - 2] != MBR_SIGNATURE_FIRST) ||
	    (mbr_sector[PARTITION_BLOCK_SIZE - 1] != MBR_SIGNATURE_SECOND)) {
		return -ENOENT;
	}
	offset = (uintptr_t)&mbr_sector + MBR_PRIMARY_ENTRY_OFFSET;
	memcpy(mbr_entry, (void *)offset, sizeof(mbr_entry_t));
	return 0;
}

/*
 * Load GPT header and check the GPT signature.
 * If partiton numbers could be found, check & update it.
 */
static int load_gpt_header(uintptr_t image_handle)
{
	gpt_header_t header;
	size_t bytes_read;
	int result;

	result = io_seek(image_handle, IO_SEEK_SET, GPT_HEADER_OFFSET);
	if (result != 0) {
		return result;
	}
	result = io_read(image_handle, (uintptr_t)&header,
			 sizeof(gpt_header_t), &bytes_read);
	if ((result != 0) || (sizeof(gpt_header_t) != bytes_read)) {
		return result;
	}
	if (memcmp(header.signature, GPT_SIGNATURE,
		   sizeof(header.signature)) != 0) {
		return -EINVAL;
	}

	/* partition numbers can't exceed PLAT_PARTITION_MAX_ENTRIES */
	list.entry_count = header.list_num;
	if (list.entry_count > PLAT_PARTITION_MAX_ENTRIES) {
		list.entry_count = PLAT_PARTITION_MAX_ENTRIES;
	}
	return 0;
}

static int load_gpt_entry(uintptr_t image_handle, gpt_entry_t *entry)
{
	size_t bytes_read;
	int result;

	assert(entry != NULL);
	result = io_read(image_handle, (uintptr_t)entry, sizeof(gpt_entry_t),
			 &bytes_read);
	if (sizeof(gpt_entry_t) != bytes_read)
		return -EINVAL;
	return result;
}

static int verify_partition_gpt(uintptr_t image_handle)
{
	gpt_entry_t entry;
	int result, i;

	for (i = 0; i < list.entry_count; i++) {
		result = load_gpt_entry(image_handle, &entry);
		assert(result == 0);
		result = parse_gpt_entry(&entry, &list.list[i]);
		if (result != 0) {
			break;
		}
	}
	if (i == 0) {
		return -EINVAL;
	}
	/*
	 * Only records the valid partition number that is loaded from
	 * partition table.
	 */
	list.entry_count = i;
	dump_entries(list.entry_count);

	return 0;
}

int load_partition_table(unsigned int image_id)
{
	uintptr_t dev_handle, image_handle, image_spec = 0;
	mbr_entry_t mbr_entry;
	int result;

	result = plat_get_image_source(image_id, &dev_handle, &image_spec);
	if (result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
			image_id, result);
		return result;
	}

	result = io_open(dev_handle, image_spec, &image_handle);
	if (result != 0) {
		WARN("Failed to access image id=%u (%i)\n", image_id, result);
		return result;
	}

	result = load_mbr_header(image_handle, &mbr_entry);
	if (result != 0) {
		WARN("Failed to access image id=%u (%i)\n", image_id, result);
		return result;
	}
	if (mbr_entry.type == PARTITION_TYPE_GPT) {
		result = load_gpt_header(image_handle);
		assert(result == 0);
		result = io_seek(image_handle, IO_SEEK_SET, GPT_ENTRY_OFFSET);
		assert(result == 0);
		result = verify_partition_gpt(image_handle);
	} else {
		/* MBR type isn't supported yet. */
		result = -EINVAL;
		goto exit;
	}
exit:
	io_close(image_handle);
	return result;
}

const partition_entry_t *get_partition_entry(const char *name)
{
	int i;

	for (i = 0; i < list.entry_count; i++) {
		if (strcmp(name, list.list[i].name) == 0) {
			return &list.list[i];
		}
	}
	return NULL;
}

const partition_entry_list_t *get_partition_entry_list(void)
{
	return &list;
}

void partition_init(unsigned int image_id)
{
	load_partition_table(image_id);
}

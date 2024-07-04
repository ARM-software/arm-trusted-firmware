/*
 * Copyright (c) 2016-2024, Arm Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <inttypes.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <common/tf_crc32.h>
#include <drivers/io/io_storage.h>
#include <drivers/partition/efi.h>
#include <drivers/partition/partition.h>
#include <drivers/partition/gpt.h>
#include <drivers/partition/mbr.h>
#include <plat/common/platform.h>

static uint8_t mbr_sector[PLAT_PARTITION_BLOCK_SIZE];
static partition_entry_list_t list;

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
		VERBOSE("%d: %s %" PRIx64 "-%" PRIx64 "\n", i + 1, name, list.list[i].start,
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
	int result;
	mbr_entry_t tmp;

	assert(mbr_entry != NULL);
	/* MBR partition table is in LBA0. */
	result = io_seek(image_handle, IO_SEEK_SET, MBR_OFFSET);
	if (result != 0) {
		VERBOSE("Failed to seek (%i)\n", result);
		return result;
	}
	result = io_read(image_handle, (uintptr_t)&mbr_sector,
			 PLAT_PARTITION_BLOCK_SIZE, &bytes_read);
	if ((result != 0) || (bytes_read != PLAT_PARTITION_BLOCK_SIZE)) {
		VERBOSE("Failed to read data (%i)\n", result);
		return result;
	}

	/* Check MBR boot signature. */
	if ((mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 2] != MBR_SIGNATURE_FIRST) ||
	    (mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 1] != MBR_SIGNATURE_SECOND)) {
		VERBOSE("MBR boot signature failure\n");
		return -ENOENT;
	}

	memcpy(&tmp, mbr_sector + MBR_PRIMARY_ENTRY_OFFSET, sizeof(tmp));

	if (tmp.first_lba != 1) {
		VERBOSE("MBR header may have an invalid first LBA\n");
		return -EINVAL;
	}

	if ((tmp.sector_nums == 0) || (tmp.sector_nums == UINT32_MAX)) {
		VERBOSE("MBR header entry has an invalid number of sectors\n");
		return -EINVAL;
	}

	memcpy(mbr_entry, &tmp, sizeof(mbr_entry_t));
	return 0;
}

/*
 * Load GPT header and check the GPT signature and header CRC.
 * If partition numbers could be found, check & update it.
 */
static int load_gpt_header(uintptr_t image_handle, size_t header_offset,
			   gpt_header_t *header)
{
	size_t bytes_read;
	int result;
	uint32_t header_crc, calc_crc;

	result = io_seek(image_handle, IO_SEEK_SET, header_offset);
	if (result != 0) {
		VERBOSE("Failed to seek into the GPT image at offset (%zu)\n",
			header_offset);
		return result;
	}
	result = io_read(image_handle, (uintptr_t)header,
			 sizeof(gpt_header_t), &bytes_read);
	if ((result != 0) || (sizeof(gpt_header_t) != bytes_read)) {
		VERBOSE("GPT header read error(%i) or read mismatch occurred,"
			"expected(%zu) and actual(%zu)\n", result,
			sizeof(gpt_header_t), bytes_read);
		return result;
	}
	if (memcmp(header->signature, GPT_SIGNATURE,
			   sizeof(header->signature)) != 0) {
		VERBOSE("GPT header signature failure\n");
		return -EINVAL;
	}

	/*
	 * UEFI Spec 2.8 March 2019 Page 119: HeaderCRC32 value is
	 * computed by setting this field to 0, and computing the
	 * 32-bit CRC for HeaderSize bytes.
	 */
	header_crc = header->header_crc;
	header->header_crc = 0U;

	calc_crc = tf_crc32(0U, (uint8_t *)header, sizeof(gpt_header_t));
	if (header_crc != calc_crc) {
		ERROR("Invalid GPT Header CRC: Expected 0x%x but got 0x%x.\n",
		      header_crc, calc_crc);
		return -EINVAL;
	}

	header->header_crc = header_crc;

	/* partition numbers can't exceed PLAT_PARTITION_MAX_ENTRIES */
	list.entry_count = header->list_num;
	if (list.entry_count > PLAT_PARTITION_MAX_ENTRIES) {
		list.entry_count = PLAT_PARTITION_MAX_ENTRIES;
	}

	return 0;
}

/*
 * Load a single MBR entry based on details from MBR header.
 */
static int load_mbr_entry(uintptr_t image_handle, mbr_entry_t *mbr_entry,
			  int part_number)
{
	size_t bytes_read;
	uintptr_t offset;
	int result;

	assert(mbr_entry != NULL);
	/* MBR partition table is in LBA0. */
	result = io_seek(image_handle, IO_SEEK_SET, MBR_OFFSET);
	if (result != 0) {
		VERBOSE("Failed to seek (%i)\n", result);
		return result;
	}
	result = io_read(image_handle, (uintptr_t)&mbr_sector,
			 PLAT_PARTITION_BLOCK_SIZE, &bytes_read);
	if (result != 0) {
		VERBOSE("Failed to read data (%i)\n", result);
		return result;
	}

	/* Check MBR boot signature. */
	if ((mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 2] != MBR_SIGNATURE_FIRST) ||
	    (mbr_sector[LEGACY_PARTITION_BLOCK_SIZE - 1] != MBR_SIGNATURE_SECOND)) {
		VERBOSE("MBR Entry boot signature failure\n");
		return -ENOENT;
	}
	offset = (uintptr_t)&mbr_sector +
		MBR_PRIMARY_ENTRY_OFFSET +
		MBR_PRIMARY_ENTRY_SIZE * part_number;
	memcpy(mbr_entry, (void *)offset, sizeof(mbr_entry_t));

	return 0;
}

/*
 * Load MBR entries based on max number of partition entries.
 */
static int load_mbr_entries(uintptr_t image_handle)
{
	mbr_entry_t mbr_entry;
	unsigned int i;

	list.entry_count = MBR_PRIMARY_ENTRY_NUMBER;

	for (i = 0U; i < list.entry_count; i++) {
		load_mbr_entry(image_handle, &mbr_entry, i);
		list.list[i].start = mbr_entry.first_lba * 512;
		list.list[i].length = mbr_entry.sector_nums * 512;
		list.list[i].name[0] = mbr_entry.type;
	}

	return 0;
}

/*
 * Try to read and load a single GPT entry.
 */
static int load_gpt_entry(uintptr_t image_handle, gpt_entry_t *entry)
{
	size_t bytes_read = 0U;
	int result;

	assert(entry != NULL);
	result = io_read(image_handle, (uintptr_t)entry, sizeof(gpt_entry_t),
			&bytes_read);
	if ((result != 0) || (sizeof(gpt_entry_t) != bytes_read)) {
		VERBOSE("GPT Entry read error(%i) or read mismatch occurred,"
			"expected(%zu) and actual(%zu)\n", result,
			sizeof(gpt_entry_t), bytes_read);
		return -EINVAL;
	}

	return result;
}

/*
 * Retrieve each entry in the partition table, parse the data from each
 * entry and store them in the list of partition table entries.
 */
static int load_partition_gpt(uintptr_t image_handle, gpt_header_t header)
{
	const signed long long gpt_entry_offset = LBA(header.part_lba);
	gpt_entry_t entry;
	int result;
	unsigned int i;
	uint32_t calc_crc = 0U;

	result = io_seek(image_handle, IO_SEEK_SET, gpt_entry_offset);
	if (result != 0) {
		VERBOSE("Failed to seek (%i), Failed loading GPT partition"
			"table entries\n", result);
		return result;
	}

	for (i = 0U; i < list.entry_count; i++) {
		result = load_gpt_entry(image_handle, &entry);
		if (result != 0) {
			VERBOSE("Failed to load gpt entry data(%u) error is (%i)\n",
				i, result);
			return result;
		}

		result = parse_gpt_entry(&entry, &list.list[i]);
		if (result != 0) {
			result = io_seek(image_handle, IO_SEEK_SET,
					(gpt_entry_offset + (i * sizeof(gpt_entry_t))));
			if (result != 0) {
				VERBOSE("Failed to seek (%i)\n", result);
				return result;
			}
			break;
		}

		/*
		 * Calculate CRC of Partition entry array to compare with CRC
		 * value in header
		 */
		calc_crc = tf_crc32(calc_crc, (uint8_t *)&entry, sizeof(gpt_entry_t));
	}
	if (i == 0) {
		VERBOSE("No Valid GPT Entries found\n");
		return -EINVAL;
	}

	/*
	 * Only records the valid partition number that is loaded from
	 * partition table.
	 */
	list.entry_count = i;
	dump_entries(list.entry_count);

	/*
	 * If there are less valid entries than the possible number of entries
	 * from the header, continue to load the partition entry table to
	 * calculate the full CRC in order to check against the partition CRC
	 * from the header for validation.
	 */
	for (; i < header.list_num; i++) {
		result = load_gpt_entry(image_handle, &entry);
		if (result != 0) {
			VERBOSE("Failed to load gpt entry data(%u) error is (%i)\n",
				i, result);
			return result;
		}

		calc_crc = tf_crc32(calc_crc, (uint8_t *)&entry, sizeof(gpt_entry_t));
	}

	if (header.part_crc != calc_crc) {
		ERROR("Invalid GPT Partition Array Entry CRC: Expected 0x%x"
				" but got 0x%x.\n", header.part_crc, calc_crc);
		return -EINVAL;
	}

	return 0;
}

/*
 * Try retrieving and parsing the backup-GPT header and backup GPT entries.
 * Last 33 blocks contains the backup-GPT entries and header.
 */
static int load_backup_gpt(unsigned int image_id, unsigned int sector_nums)
{
	int result;
	gpt_header_t header;
	size_t gpt_header_offset;
	uintptr_t dev_handle, image_spec, image_handle;
	io_block_spec_t *block_spec;
	int part_num_entries;

	result = plat_get_image_source(image_id, &dev_handle, &image_spec);
	if (result != 0) {
		VERBOSE("Failed to obtain reference to image id=%u (%i)\n",
			image_id, result);
		return result;
	}

	block_spec = (io_block_spec_t *)image_spec;
	/*
	 * We need to read 32 blocks of GPT entries and one block of GPT header
	 * try mapping only last 33 last blocks from the image to read the
	 * Backup-GPT header and its entries.
	 */
	part_num_entries = (PLAT_PARTITION_MAX_ENTRIES / 4);
	/* Move the offset base to LBA-33 */
	block_spec->offset += LBA(sector_nums - part_num_entries);
	/*
	 * Set length as LBA-33, 32 blocks of backup-GPT entries and one
	 * block of backup-GPT header.
	 */
	block_spec->length = LBA(part_num_entries + 1);

	result = io_open(dev_handle, image_spec, &image_handle);
	if (result != 0) {
		VERBOSE("Failed to access image id (%i)\n", result);
		return result;
	}

	INFO("Trying to retrieve back-up GPT header\n");
	/* Last block is backup-GPT header, after the end of GPT entries */
	gpt_header_offset = LBA(part_num_entries);
	result = load_gpt_header(image_handle, gpt_header_offset, &header);
	if ((result != 0) || (header.part_lba == 0)) {
		ERROR("Failed to retrieve Backup GPT header,"
		      "Partition maybe corrupted\n");
		goto out;
	}

	/*
	 * Note we mapped last 33 blocks(LBA-33), first block here starts with
	 * entries while last block was header.
	 */
	header.part_lba = 0;
	result = load_partition_gpt(image_handle, header);

out:
	io_close(image_handle);
	return result;
}

/*
 * Load a GPT partition, Try retrieving and parsing the primary GPT header,
 * if its corrupted try loading backup GPT header and then retrieve list
 * of partition table entries found from the GPT.
 */
static int load_primary_gpt(uintptr_t image_handle, unsigned int first_lba)
{
	int result;
	size_t gpt_header_offset;
	gpt_header_t header;

	/* Try to load Primary GPT header from LBA1 */
	gpt_header_offset = LBA(first_lba);
	result = load_gpt_header(image_handle, gpt_header_offset, &header);
	if ((result != 0) || (header.part_lba == 0)) {
		VERBOSE("Failed to retrieve Primary GPT header,"
			"trying to retrieve back-up GPT header\n");
		return result;
	}

	return load_partition_gpt(image_handle, header);
}

/*
 * Load the partition table info based on the image id provided.
 */
int load_partition_table(unsigned int image_id)
{
	uintptr_t dev_handle, image_handle, image_spec = 0;
	mbr_entry_t mbr_entry;
	int result;

	result = plat_get_image_source(image_id, &dev_handle, &image_spec);
	if (result != 0) {
		VERBOSE("Failed to obtain reference to image id=%u (%i)\n",
			image_id, result);
		return result;
	}

	result = io_open(dev_handle, image_spec, &image_handle);
	if (result != 0) {
		VERBOSE("Failed to access image id=%u (%i)\n", image_id, result);
		return result;
	}

	result = load_mbr_header(image_handle, &mbr_entry);
	if (result != 0) {
		VERBOSE("Failed to access image id=%u (%i)\n", image_id, result);
		goto out;
	}
	if (mbr_entry.type == PARTITION_TYPE_GPT) {
		result = load_primary_gpt(image_handle, mbr_entry.first_lba);
		if (result != 0) {
			io_close(image_handle);
			return load_backup_gpt(BKUP_GPT_IMAGE_ID,
					       mbr_entry.sector_nums);
		}
	} else {
		result = load_mbr_entries(image_handle);
	}

out:
	io_close(image_handle);
	return result;
}

/*
 * Try retrieving a partition table entry based on the name of the partition.
 */
const partition_entry_t *get_partition_entry(const char *name)
{
	unsigned int i;

	for (i = 0U; i < list.entry_count; i++) {
		if (strcmp(name, list.list[i].name) == 0) {
			return &list.list[i];
		}
	}
	return NULL;
}

/*
 * Try retrieving a partition table entry based on the partition type GUID.
 */
const partition_entry_t *get_partition_entry_by_type(
	const struct efi_guid *type_guid)
{
	unsigned int i;

	for (i = 0U; i < list.entry_count; i++) {
		if (guidcmp(type_guid, &list.list[i].type_guid) == 0) {
			return &list.list[i];
		}
	}

	return NULL;
}

/*
 * Try retrieving a partition table entry based on the unique partition GUID.
 */
const partition_entry_t *get_partition_entry_by_guid(
	const struct efi_guid *part_guid)
{
	unsigned int i;

	for (i = 0U; i < list.entry_count; i++) {
		if (guidcmp(part_guid, &list.list[i].part_guid) == 0) {
			return &list.list[i];
		}
	}

	return NULL;
}

/*
 * Return entry to the list of partition table entries.
 */
const partition_entry_list_t *get_partition_entry_list(void)
{
	return &list;
}

/*
 * Try loading partition table info for the given image ID.
 */
void partition_init(unsigned int image_id)
{
	int ret;

	ret = load_partition_table(image_id);
	if (ret != 0) {
		ERROR("Failed to parse partition with image id = %u\n",
		      image_id);
	}
}

/*
 * Load a GPT based image.
 */
int gpt_partition_init(void)
{
	return load_partition_table(GPT_IMAGE_ID);
}

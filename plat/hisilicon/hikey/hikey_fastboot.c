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

#include <arch_helpers.h>
#include <assert.h>
#include <debug.h>
#include <ctype.h>
#include <emmc.h>
#include <errno.h>
#include <fastboot/fastboot.h>
#include <gpio.h>
#include <hi6220.h>
#include <mmio.h>
#include <partition/partition.h>
#include <platform.h>
#include <string.h>
#include <tbbr/tbbr_img_def.h>

#include "hikey_def.h"
#include "hikey_private.h"

#define HIKEY_LED_GPIO_BASE			32

static struct random_serial_num hikey_sn;

static inline char hex2str(unsigned int data)
{
	data &= 0xf;
	if ((data >= 0) && (data <= 9))
		return (char)(data + 0x30);
	return (char)(data - 10 + 0x41);
}

static uint64_t hikey_rand(unsigned int data)
{
	int64_t quotient, remainder, t;

	quotient = data / 127773;
	remainder = data % 127773;
	t = 16807 * remainder - 2836 * quotient;
	if (t <= 0)
		t += RANDOM_MAX;
	return (t % ((uint64_t)RANDOM_MAX + 1));
}

void hikey_assign_serialno(const char *cmdbuf, struct random_serial_num *sn)
{
	int offset, i;

	assert(sn != NULL);
	offset = 0;
	while (*(cmdbuf + offset) == ' ')
		offset++;
	for (i = 0; i < 16; i++) {
		if (isxdigit(*(cmdbuf + offset + i)))
			continue;
		/* invalid character */
		assert(0);
	}
	memcpy(sn->serialno, cmdbuf + offset, 16);
	sn->serialno[16] = '\0';
	sn->magic = RANDOM_MAGIC;
}

void hikey_generate_serialno(struct random_serial_num *sn)
{
	unsigned int data, t;
	int i;

	assert(sn != NULL);
	data = mmio_read_32(AO_SC_SYSTEST_SLICER_CNT0);
	t = hikey_rand(data);
	sn->data = ((uint64_t)t << 32) | data;
	for (i = 0; i < 8; i++) {
		sn->serialno[i] = hex2str((t >> ((7 - i) << 2)) & 0xf);
	}
	for (i = 0; i < 8; i++) {
		sn->serialno[i + 8] = hex2str((data >> ((7 - i) << 2)) & 0xf);
	}
	sn->serialno[16] = '\0';
	sn->magic = RANDOM_MAGIC;
}

const char *hikey_init_serialno(void)
{
	int result;

	result = hikey_read_serialno(&hikey_sn);
	if (result < 0) {
		hikey_generate_serialno(&hikey_sn);
		result = hikey_write_serialno(&hikey_sn);
		assert(result == 0);
	}
	fastboot_set_var(FASTBOOT_VAR_SERIALNO, hikey_sn.serialno, NULL);
	return hikey_sn.serialno;
}

/*
 * Execution of fastboot_var.
 */
int hikey_get_partition_type(const char *arg, int left, char *response)
{
	const partition_entry_list_t *entry_list;
	const partition_entry_t *entry;
	int i, count = 0, len;

	assert((left > 0) && (response != NULL));
	if (arg != NULL) {
		/* Access one partition entry. */
		entry = get_partition_entry(arg);
		if (entry == NULL)
			goto exit;
		if ((strcmp(arg, "system") == 0) ||
		    (strcmp(arg, "userdata") == 0)) {
			len = snprintf(response, left, "ext4");
		} else if (strcmp(arg, "cache") == 0) {
			len = snprintf(response, left, "%s", "fat");
		} else {
			len = snprintf(response, left, "%s", "raw");
		}
		if (len < left) {
			count = len;
		}
	} else {
		/* Get all partition entries. */
		entry_list = get_partition_entry_list();
		assert((entry_list != NULL) && (entry_list->entry_count > 0));
		for (i = 0, count = 0; i < entry_list->entry_count; i++) {
			len = snprintf(response + count, left, "\n%s%s: ",
				       FASTBOOT_VAR_PARTITION_TYPE,
				       entry_list->list[i].name);
			if (len >= left) {
				break;
			}
			if (!strcmp(entry_list->list[i].name, "system") ||
			    !strcmp(entry_list->list[i].name, "userdata")) {
				len += snprintf(response + count + len,
						left - len, "ext4");
			} else if (!strcmp(entry_list->list[i].name, "cache")) {
				len += snprintf(response + count + len,
						left - len, "fat");
			} else {
				len += snprintf(response + count + len,
						left - len, "raw");
			}
			if (len >= left) {
				break;
			} else {
				count += len;
				left = left - len;
			}
		}
	}
	if (count > 0) {
		response[count] = '\0';
		return count;
	}
exit:
	snprintf(response, left, "%s", "invalid partition");
	return -EINVAL;
}

int hikey_get_partition_size(const char *arg, int left, char *response)
{
	const partition_entry_list_t *entry_list;
	const partition_entry_t *entry;
	int i, count = 0, len;

	assert((left > 0) && (response != NULL));
	if (arg != NULL) {
		entry = get_partition_entry(arg);
		if (entry == NULL)
			goto exit;
		len = snprintf(response, left, "0x%lx", entry->length);
		if (len < left) {
			count = len;
		}
	} else {
		/* Get all partition entries. */
		entry_list = get_partition_entry_list();
		assert((entry_list != NULL) && (entry_list->entry_count > 0));
		for (i = 0, count = 0; i < entry_list->entry_count; i++) {
			len = snprintf(response + count, left, "\n%s%s: ",
				       FASTBOOT_VAR_PARTITION_SIZE,
				       entry_list->list[i].name);
			if (len >= left) {
				break;
			}
			len += snprintf(response + count + len,
					left - len, "0x%lx",
					entry_list->list[i].length);
			if (len >= left) {
				break;
			} else {
				count += len;
				left = left - len;
			}
		}
	}
	if (count > 0) {
		response[count] = '\0';
		return count;
	}
exit:
	snprintf(response, left, "%s", "invalid partition");
	return -EINVAL;
}

/*
 * Callback of fastboot reboot command.
 */
int hikey_reboot(const char *arg)
{
	/* Send the system reset request */
	mmio_write_32(AO_SC_SYS_STAT0, 0x48698284);

	wfi();
	panic();
	return 0;
}

/*
 * Callback of fastboot oem command.
 */
int hikey_oem(const char *arg)
{
	int result, offset, led;

	assert(arg != NULL);

	if (!memcmp(arg, "serialno", strlen("serialno"))) {
		offset = strlen("serialno");
		if (arg[offset] == '\0') {
			/* generate new serial number */
			hikey_generate_serialno(&hikey_sn);
		} else if (!memcmp(arg + offset, " set ", strlen(" set "))) {
			/* set serial number */
			offset += strlen(" set ");
			hikey_assign_serialno(arg + offset, &hikey_sn);
		} else {
			return -EINVAL;
		}
		result = hikey_write_serialno(&hikey_sn);
		assert(result == 0);
		fastboot_set_var(FASTBOOT_VAR_SERIALNO, hikey_sn.serialno,
				 NULL);
	} else if (!memcmp(arg, "led", strlen("led"))) {
		offset = strlen("led");
		if ((arg[offset] >= '1') && (arg[offset] <= '4')) {
			led = arg[offset] - '1';
			/* skip led index and space */
			offset += 2;
			if (!memcmp(arg + offset, (void *)"on", strlen("on"))) {
				gpio_set_value(HIKEY_LED_GPIO_BASE + led, 1);
			} else if (!memcmp(arg + offset, (void *)"off",
					   strlen("off"))) {
				gpio_set_value(HIKEY_LED_GPIO_BASE + led, 0);
			} else {
				return -EINVAL;
			}
		} else {
			return -EINVAL;
		}
	} else {
		assert(0);
	}
	return 0;
}

/*
 * Callback of fastboot erase command.
 */
int hikey_erase(const char *arg)
{
	const partition_entry_t *entry;
	int lba;

	entry = get_partition_entry(arg);
	if ((entry == NULL) || ((entry->start % EMMC_BLOCK_SIZE) != 0)) {
		return -EINVAL;
	}
	lba = entry->start / EMMC_BLOCK_SIZE;
	emmc_erase_blocks(lba, entry->length);
	return 0;
}

/*
 * Callback of partition driver.
 * Create a pseudo partition entry for partition table.
 */
int hikey_partition_table_handler(const char *name, partition_entry_t *entry)
{
	assert((name != NULL) && (entry != NULL));

	if (strcmp(name, "ptable") == 0) {
		memset(entry, 0, sizeof(partition_entry_t));
		snprintf(entry->name, EFI_NAMELEN, "%s", "ptable");
		entry->start = 0;
		/*
		 * 1 block: header of serial downloading protocol.
		 * 1 block: MBR header
		 * 1 block: GPT header
		 * 4 entries share one partition block
		 */
		entry->length = (((PLAT_PARTITION_MAX_ENTRIES + 3) / 4) + 3) *
				PARTITION_BLOCK_SIZE;
	} else {
		return -EINVAL;
	}
	return 0;
}

/*
 * Callback of fastboot flash command.
 */
int hikey_flash(const char *arg)
{
	uintptr_t dev_handle, image_handle, image_spec = 0;
	uint64_t buffer;
	const partition_entry_t *entry;
	partition_entry_t pseudo_entry;
	size_t bytes_written;
	int result;

	entry = get_partition_entry(arg);
	if (entry == NULL) {
		result = hikey_partition_table_handler(arg, &pseudo_entry);
		if (result != 0)
			goto exit;
		entry = &pseudo_entry;
	}
	result = plat_get_image_source(BL2U_IMAGE_ID, &dev_handle,
				       &image_spec);
	if (result != 0) {
		WARN("Failed to obtain reference to image id=%u (%i)\n",
		     BL2U_IMAGE_ID, result);
		goto exit;
	}
	result = io_open(dev_handle, image_spec, &image_handle);
	if (result != 0) {
		WARN("Failed to access image id=%u (%i)\n",
		     BL2U_IMAGE_ID, result);
		goto exit;
	}

	result = io_seek(image_handle, IO_SEEK_SET, entry->start);
	if (result != 0) {
		WARN("Failed to seek (%i)\n", result);
		goto exit_io;
	}
	if (strcmp(arg, "ptable") == 0) {
		/* Skip the hack header of partition table. */
		buffer = HIKEY_FB_DOWNLOAD_BASE + EMMC_BLOCK_SIZE;
	} else {
		buffer = HIKEY_FB_DOWNLOAD_BASE;
	}
	result = io_write(image_handle, buffer, entry->length, &bytes_written);
	if (result != 0) {
		goto exit_io;
	}
	io_close(image_handle);

	/* Update the partition table. */
	if (strcmp(arg, "ptable") == 0) {
		result = load_partition_table(BL2U_IMAGE_ID);
		assert(result == 0);
	}
	return result;
exit_io:
	io_close(image_handle);
exit:
	return result;
}

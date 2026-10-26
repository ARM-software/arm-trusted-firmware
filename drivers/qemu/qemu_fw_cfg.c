/*
 * Copyright (c) 2026, Kuan-Wei Chiu <visitorckw@gmail.com>
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <cdefs.h>
#include <endian.h>
#include <errno.h>
#include <string.h>

#include <drivers/qemu/qemu_fw_cfg.h>
#include <lib/mmio.h>

/* QEMU fw_cfg MMIO register offsets */
#define FW_CFG_DATA_OFF		0x00U
#define FW_CFG_CTL_OFF		0x08U

/* Standard fw_cfg selector keys */
#define FW_CFG_SIGNATURE	0x0000U
#define FW_CFG_FILE_DIR		0x0019U

/* "QEMU" signature in little-endian 32-bit word */
#define FW_CFG_SIG_QEMU		0x554d4551U

#define FW_CFG_MAX_FILE_PATH	56U

struct fw_cfg_file {
	uint32_t size;
	uint16_t select;
	uint16_t reserved;
	char name[FW_CFG_MAX_FILE_PATH];
} __packed;

static uintptr_t fw_cfg_base;

void qemu_fw_cfg_select(uint16_t key)
{
	assert(fw_cfg_base != 0U);
	mmio_write_16(fw_cfg_base + FW_CFG_CTL_OFF, htobe16(key));
}

void qemu_fw_cfg_read_bytes(uint8_t *buf, size_t len)
{
	assert(fw_cfg_base != 0U);

	while ((len > 0U) && (((uintptr_t)buf & 7U) != 0U)) {
		*buf++ = mmio_read_8(fw_cfg_base + FW_CFG_DATA_OFF);
		len--;
	}

	while (len >= sizeof(uint64_t)) {
		*(uint64_t *)buf = mmio_read_64(fw_cfg_base + FW_CFG_DATA_OFF);
		buf += sizeof(uint64_t);
		len -= sizeof(uint64_t);
	}

	while (len > 0U) {
		*buf++ = mmio_read_8(fw_cfg_base + FW_CFG_DATA_OFF);
		len--;
	}
}

void qemu_fw_cfg_skip_bytes(size_t len)
{
	assert(fw_cfg_base != 0U);

	while (len >= sizeof(uint64_t)) {
		(void)mmio_read_64(fw_cfg_base + FW_CFG_DATA_OFF);
		len -= sizeof(uint64_t);
	}

	while (len > 0U) {
		(void)mmio_read_8(fw_cfg_base + FW_CFG_DATA_OFF);
		len--;
	}
}

int qemu_fw_cfg_init(uintptr_t base)
{
	assert(base != 0U);

	fw_cfg_base = base;
	qemu_fw_cfg_select(FW_CFG_SIGNATURE);
	if (mmio_read_32(fw_cfg_base + FW_CFG_DATA_OFF) != FW_CFG_SIG_QEMU) {
		fw_cfg_base = 0U;
		return -ENODEV;
	}

	return 0;
}

int qemu_fw_cfg_find_file(const char *name, uint16_t *select, size_t *size)
{
	uint32_t count, i;

	assert(name != NULL);
	assert(select != NULL);
	assert(size != NULL);

	if (fw_cfg_base == 0U) {
		return -ENODEV;
	}

	qemu_fw_cfg_select(FW_CFG_FILE_DIR);
	qemu_fw_cfg_read_bytes((uint8_t *)&count, sizeof(count));
	count = be32toh(count);

	for (i = 0U; i < count; i++) {
		struct fw_cfg_file file;

		qemu_fw_cfg_read_bytes((uint8_t *)&file, sizeof(file));
		if (strncmp(file.name, name, FW_CFG_MAX_FILE_PATH) == 0) {
			*select = be16toh(file.select);
			*size = be32toh(file.size);
			return 0;
		}
	}

	return -ENOENT;
}

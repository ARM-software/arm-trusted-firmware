/*
 * Copyright (c) 2023, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

#include "ethosn_big_fw.h"

/* Magic (FourCC) number to identify the big firmware binary */
#define ETHOSN_BIG_FW_MAGIC	('E' | ('N' << 8) | ('F' << 16) | ('W' << 24))

/* Supported big firmware version */
#define ETHOSN_BIG_FW_VERSION_MAJOR	15

#define ETHOSN_ARCH_VER_MAJOR_MASK	U(0xF000)
#define ETHOSN_ARCH_VER_MAJOR_SHIFT	U(0xC)
#define ETHOSN_ARCH_VER_MINOR_MASK	U(0xF00)
#define ETHOSN_ARCH_VER_MINOR_SHIFT	U(0x8)
#define ETHOSN_ARCH_VER_REV_MASK	U(0xFF)

/* Convert Arm(R) Ethos(TM)-N NPU architecture version to big firmware format */
#define ETHOSN_BIG_FW_FORMAT_ARCH_VER(arch_ver)					 \
	(arch_ver & ETHOSN_ARCH_VER_MAJOR_MASK) << ETHOSN_ARCH_VER_MAJOR_SHIFT | \
	(arch_ver & ETHOSN_ARCH_VER_MINOR_MASK) << ETHOSN_ARCH_VER_MINOR_SHIFT | \
	(arch_ver & ETHOSN_ARCH_VER_REV_MASK)


bool ethosn_big_fw_verify_header(const struct ethosn_big_fw *big_fw,
				 uint32_t npu_arch_ver)
{
	const uint32_t arch_ver = ETHOSN_BIG_FW_FORMAT_ARCH_VER(npu_arch_ver);

	if (big_fw->fw_magic != ETHOSN_BIG_FW_MAGIC) {
		ERROR("ETHOSN: Unable to find firmware. Invalid magic value: 0x%02x\n",
		      big_fw->fw_magic);

		return false;
	}

	if (big_fw->fw_ver_major != ETHOSN_BIG_FW_VERSION_MAJOR) {
		ERROR("ETHOSN: Unsupported firmware version: %u.%u.%u. Expected Version %u.x.x.\n",
		      big_fw->fw_ver_major, big_fw->fw_ver_minor,
		      big_fw->fw_ver_patch, ETHOSN_BIG_FW_VERSION_MAJOR);

		return false;
	}

	if (big_fw->arch_min > arch_ver || arch_ver > big_fw->arch_max) {
		ERROR("ETHOSN: Firmware is not compatbile with architecture version: 0x%02x\n",
		      npu_arch_ver);
		return false;
	}

	return true;
}

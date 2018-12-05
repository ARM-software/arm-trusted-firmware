/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdint.h>

#include "gxl_private.h"

#define EFUSE_BASE	0x140
#define EFUSE_SIZE	0xC0

uint64_t gxbb_efuse_read(void *dst, uint32_t offset, uint32_t size)
{
	if ((uint64_t)(offset + size) > (uint64_t)EFUSE_SIZE)
		return 0;

	return scpi_efuse_read(dst, offset + EFUSE_BASE, size);
}

uint64_t gxbb_efuse_user_max(void)
{
	return EFUSE_SIZE;
}

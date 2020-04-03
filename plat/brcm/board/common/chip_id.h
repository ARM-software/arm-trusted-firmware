/*
 * Copyright (c) 2017 - 2020, Broadcom
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef CHIP_ID_H
#define CHIP_ID_H

#include <lib/mmio.h>

#include <platform_def.h>

#define CHIP_REV_MAJOR_MASK	0xF0
#define CHIP_REV_MAJOR_AX	0x00
#define CHIP_REV_MAJOR_BX	0x10
#define CHIP_REV_MAJOR_CX	0x20
#define CHIP_REV_MAJOR_DX	0x30

/* Get Chip ID (product number) of the chip */
static inline unsigned int chip_get_product_id(void)
{
	return PLAT_CHIP_ID_GET;
}

/* Get Revision ID (major and minor) number of the chip */
static inline unsigned int chip_get_rev_id(void)
{
	return PLAT_CHIP_REV_GET;
}

static inline unsigned int chip_get_rev_id_major(void)
{
	return (chip_get_rev_id() & CHIP_REV_MAJOR_MASK);
}

#endif

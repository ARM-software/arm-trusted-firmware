/*
 * Copyright (C) 2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <plat_marvell.h>

#define DEVICE_BASE		0xF0000000
#define DDR_PHY_OFFSET		0x1000000
#define DDR_PHY_BASE_ADDR	(DEVICE_BASE + DDR_PHY_OFFSET)

int mvebu_ddr_phy_write(uintptr_t offset, uint16_t data);
int mvebu_ddr_phy_read(uintptr_t offset, uint16_t *read);

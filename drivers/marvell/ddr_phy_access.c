/*
 * Copyright (C) 2021 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include "ddr_phy_access.h"
#include <lib/mmio.h>
#include <drivers/marvell/ccu.h>
#include <errno.h>

#define DDR_PHY_END_ADDRESS	0x100000

#ifdef DDR_PHY_DEBUG
#define debug_printf(...) printf(__VA_ARGS__)
#else
#define debug_printf(...)
#endif


/*
 * This routine writes 'data' to specified 'address' offset,
 * with optional debug print support
 */
int snps_fw_write(uintptr_t offset, uint16_t data)
{
	debug_printf("In %s\n", __func__);

	if (offset < DDR_PHY_END_ADDRESS) {
		mmio_write_16(DDR_PHY_BASE_ADDR + (2 * offset), data);
		return 0;
	}
	debug_printf("%s: illegal offset value: 0x%x\n", __func__, offset);
	return -EINVAL;
}

int snps_fw_read(uintptr_t offset, uint16_t *read)
{
	debug_printf("In %s\n", __func__);

	if (offset < DDR_PHY_END_ADDRESS) {
		*read = mmio_read_16(DDR_PHY_BASE_ADDR + (2 * offset));
		return 0;
	}
	debug_printf("%s: illegal offset value: 0x%x\n", __func__, offset);
	return -EINVAL;
}

int mvebu_ddr_phy_write(uintptr_t offset, uint16_t data)
{
	return snps_fw_write(offset, data);
}

int mvebu_ddr_phy_read(uintptr_t offset, uint16_t *read)
{
	return snps_fw_read(offset, read);
}

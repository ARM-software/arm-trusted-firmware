/*
 * Copyright (c) 2024-2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <stm32mp2_private.h>

/*
 * SYSCFG register offsets (base relative)
 */
#define SYSCFG_OCTOSPIAMCR		0x2C00U
#define SYSCFG_DEVICEID			0x6400U

/* SYSCFG_OCTOSPIAMCR Register */
#define SYSCFG_OCTOSPIAMCR_OAM_MASK	GENMASK_32(2, 0)

/* SYSCFG_DEVICEID Register */
#define SYSCFG_DEVICEID_DEV_ID_MASK	GENMASK_32(11, 0)

/*
 * @brief  Get device ID from SYSCFG registers.
 * @retval device ID (DEV_ID).
 */
uint32_t stm32mp_syscfg_get_chip_dev_id(void)
{
	return mmio_read_32(SYSCFG_BASE + SYSCFG_DEVICEID) & SYSCFG_DEVICEID_DEV_ID_MASK;
}

size_t stm32mp_syscfg_get_mm_size(uint8_t bank)
{
	uint32_t amcr = mmio_read_32(SYSCFG_BASE + SYSCFG_OCTOSPIAMCR);
	uint32_t oam = amcr & SYSCFG_OCTOSPIAMCR_OAM_MASK;
	size_t addr_mapping1;
	size_t addr_mapping2;

	if (oam > 3U) {
		addr_mapping2 = SZ_256M;
	} else {
		addr_mapping2 = oam * SZ_64M;
	}

	addr_mapping1 = SZ_256M - addr_mapping2;

	return (bank == 0U) ? addr_mapping1 : addr_mapping2;
}

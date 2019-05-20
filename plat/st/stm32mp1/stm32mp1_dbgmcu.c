/*
 * Copyright (c) 2016-2022, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <errno.h>

#include <common/debug.h>
#include <drivers/st/bsec.h>
#include <drivers/st/bsec2_reg.h>
#include <drivers/st/stm32mp1_rcc.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <platform_def.h>
#include <stm32mp1_dbgmcu.h>

#define DBGMCU_IDC		U(0x00)

#define DBGMCU_IDC_DEV_ID_MASK	GENMASK(11, 0)
#define DBGMCU_IDC_REV_ID_MASK	GENMASK(31, 16)
#define DBGMCU_IDC_REV_ID_SHIFT	16

static int stm32mp1_dbgmcu_init(void)
{
	if ((bsec_read_debug_conf() & BSEC_DBGSWGEN) == 0U) {
		INFO("Software access to all debug components is disabled\n");
		return -1;
	}

	mmio_setbits_32(RCC_BASE + RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);

	return 0;
}

/*
 * @brief  Get silicon revision from DBGMCU registers.
 * @param  chip_version: pointer to the read value.
 * @retval 0 on success, negative value on failure.
 */
int stm32mp1_dbgmcu_get_chip_version(uint32_t *chip_version)
{
	assert(chip_version != NULL);

	if (stm32mp1_dbgmcu_init() != 0) {
		return -EPERM;
	}

	*chip_version = (mmio_read_32(DBGMCU_BASE + DBGMCU_IDC) &
			 DBGMCU_IDC_REV_ID_MASK) >> DBGMCU_IDC_REV_ID_SHIFT;

	return 0;
}

/*
 * @brief  Get device ID from DBGMCU registers.
 * @param  chip_dev_id: pointer to the read value.
 * @retval 0 on success, negative value on failure.
 */
int stm32mp1_dbgmcu_get_chip_dev_id(uint32_t *chip_dev_id)
{
	assert(chip_dev_id != NULL);

	if (stm32mp1_dbgmcu_init() != 0) {
		return -EPERM;
	}

	*chip_dev_id = mmio_read_32(DBGMCU_BASE + DBGMCU_IDC) &
		       DBGMCU_IDC_DEV_ID_MASK;

	return 0;
}

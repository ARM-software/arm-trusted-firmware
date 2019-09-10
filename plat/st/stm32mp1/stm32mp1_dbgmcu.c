/*
 * Copyright (c) 2016-2019, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <platform_def.h>

#include <common/debug.h>
#include <drivers/st/bsec.h>
#include <drivers/st/stm32mp1_rcc.h>
#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <stm32mp1_dbgmcu.h>

#define DBGMCU_IDC		U(0x00)
#define DBGMCU_APB4FZ1		U(0x2C)

#define DBGMCU_IDC_DEV_ID_MASK	GENMASK(11, 0)
#define DBGMCU_IDC_REV_ID_MASK	GENMASK(31, 16)
#define DBGMCU_IDC_REV_ID_SHIFT	16

#define DBGMCU_APB4FZ1_IWDG2	BIT(2)

static uintptr_t get_rcc_base(void)
{
	/* This is called before stm32mp_rcc_base() is available */
	return RCC_BASE;
}

static int stm32mp1_dbgmcu_init(void)
{
	uint32_t dbg_conf;
	uintptr_t rcc_base = get_rcc_base();

	dbg_conf = bsec_read_debug_conf();

	if ((dbg_conf & BSEC_DBGSWGEN) == 0U) {
		uint32_t result = bsec_write_debug_conf(dbg_conf |
							BSEC_DBGSWGEN);

		if (result != BSEC_OK) {
			ERROR("Error enabling DBGSWGEN\n");
			return -1;
		}
	}

	mmio_setbits_32(rcc_base + RCC_DBGCFGR, RCC_DBGCFGR_DBGCKEN);

	return 0;
}

int stm32mp1_dbgmcu_get_chip_version(uint32_t *chip_version)
{
	if (stm32mp1_dbgmcu_init() != 0) {
		return -EPERM;
	}

	*chip_version = (mmio_read_32(DBGMCU_BASE + DBGMCU_IDC) &
			 DBGMCU_IDC_REV_ID_MASK) >> DBGMCU_IDC_REV_ID_SHIFT;

	return 0;
}

int stm32mp1_dbgmcu_get_chip_dev_id(uint32_t *chip_dev_id)
{
	if (stm32mp1_dbgmcu_init() != 0) {
		return -EPERM;
	}

	*chip_dev_id = mmio_read_32(DBGMCU_BASE + DBGMCU_IDC) &
		DBGMCU_IDC_DEV_ID_MASK;

	return 0;
}

int stm32mp1_dbgmcu_freeze_iwdg2(void)
{
	uint32_t dbg_conf;

	if (stm32mp1_dbgmcu_init() != 0) {
		return -EPERM;
	}

	dbg_conf = bsec_read_debug_conf();

	if ((dbg_conf & (BSEC_SPIDEN | BSEC_SPINDEN)) != 0U) {
		mmio_setbits_32(DBGMCU_BASE + DBGMCU_APB4FZ1,
				DBGMCU_APB4FZ1_IWDG2);
	}

	return 0;
}

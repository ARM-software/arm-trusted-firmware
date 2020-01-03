/*
 * Copyright (c) 2019, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <assert.h>
#include <drivers/arm/scu.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>
#include <stdint.h>

/*******************************************************************************
 * Turn ON snoop control unit. This is needed to synchronize the data between
 * CPU's.
 ******************************************************************************/
void enable_snoop_ctrl_unit(uintptr_t base)
{
	uint32_t scu_ctrl;

	INFO("[SCU]: enabling snoop control unit ... \n");

	assert(base != 0U);
	scu_ctrl = mmio_read_32(base + SCU_CTRL_REG);

	/* already enabled? */
	if ((scu_ctrl & SCU_ENABLE_BIT) != 0) {
		return;
	}

	scu_ctrl |= SCU_ENABLE_BIT;
	mmio_write_32(base + SCU_CTRL_REG, scu_ctrl);
}

/*******************************************************************************
 * Snoop Control Unit configuration register. This is read-only register and
 * contains information such as
 * - number of CPUs present
 * - is a particular CPU operating in SMP mode or AMP mode
 * - data cache size of a particular CPU
 * - does SCU has ACP port
 * - is L2CPRESENT
 * NOTE: user of this API should interpert the bits in this register according
 * to the TRM
 ******************************************************************************/
uint32_t read_snoop_ctrl_unit_cfg(uintptr_t base)
{
	assert(base != 0U);

	return mmio_read_32(base + SCU_CFG_REG);
}

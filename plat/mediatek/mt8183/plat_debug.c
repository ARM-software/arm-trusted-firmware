/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <plat_debug.h>
#include <platform_def.h>
#include <spm.h>

void circular_buffer_setup(void)
{
	/* Clear DBG_CONTROL.lastpc_disable to enable circular buffer */
	sync_writel(CA15M_DBG_CONTROL,
		    mmio_read_32(CA15M_DBG_CONTROL) & ~(BIT_CA15M_LASTPC_DIS));
}

void circular_buffer_unlock(void)
{
	unsigned int i;

	/* Disable big vproc external off (set CPU_EXT_BUCK_ISO to 0x0) */
	sync_writel(VPROC_EXT_CTL, mmio_read_32(VPROC_EXT_CTL) & ~(0x1 << 1));

	/* Release vproc apb mask (set 0x0C53_2008[1] to 0x0) */
	sync_writel(CA15M_PWR_RST_CTL, mmio_read_32(CA15M_PWR_RST_CTL) & ~(0x1 << 1));

	for (i = 1; i <= 4; ++i)
		sync_writel(MP1_CPUTOP_PWR_CON + i * 4,
			    (mmio_read_32(MP1_CPUTOP_PWR_CON + i * 4) & ~(0x4))|(0x4));

	/* Set DFD.en */
	sync_writel(DFD_INTERNAL_CTL, 0x1);
}

void circular_buffer_lock(void)
{
	/* Clear DFD.en */
	sync_writel(DFD_INTERNAL_CTL, 0x0);
}

void clear_all_on_mux(void)
{
	sync_writel(MCU_ALL_PWR_ON_CTRL,
		    mmio_read_32(MCU_ALL_PWR_ON_CTRL) & ~(1 << 2));
	sync_writel(MCU_ALL_PWR_ON_CTRL,
		    mmio_read_32(MCU_ALL_PWR_ON_CTRL) & ~(1 << 1));
}

void l2c_parity_check_setup(void)
{
	/* Enable DBG_CONTROL.l2parity_en */
	sync_writel(CA15M_DBG_CONTROL,
		    mmio_read_32(CA15M_DBG_CONTROL) | BIT_CA15M_L2PARITY_EN);
}

/*
 * Copyright (c) 2017-2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>

#include <sunxi_mmap.h>
#include <sunxi_private.h>

#ifdef SUNXI_SPC_BASE
#define SPC_DECPORT_STA_REG(p)	(SUNXI_SPC_BASE + ((p) * 0x0c) + 0x4)
#define SPC_DECPORT_SET_REG(p)	(SUNXI_SPC_BASE + ((p) * 0x0c) + 0x8)
#define SPC_DECPORT_CLR_REG(p)	(SUNXI_SPC_BASE + ((p) * 0x0c) + 0xc)
#endif

#define R_PRCM_SEC_SWITCH_REG	0x1d0
#define DMA_SEC_REG		0x20

/*
 * Setup the peripherals to be accessible by non-secure world.
 * This will not work for the Secure Peripherals Controller (SPC) unless
 * a fuse it burnt (seems to be an erratum), but we do it nevertheless,
 * to allow booting on boards using secure boot.
 */
void sunxi_security_setup(void)
{
#ifdef SUNXI_SPC_BASE
	int i;

	INFO("Configuring SPC Controller\n");
	/* SPC setup: set all devices to non-secure */
	for (i = 0; i < 6; i++)
		mmio_write_32(SPC_DECPORT_SET_REG(i), 0xff);
#endif

	/* set MBUS clocks, bus clocks (AXI/AHB/APB) and PLLs to non-secure */
	mmio_write_32(SUNXI_CCU_SEC_SWITCH_REG, 0x7);

	/* set R_PRCM clocks to non-secure */
	mmio_write_32(SUNXI_R_PRCM_BASE + R_PRCM_SEC_SWITCH_REG, 0x7);

	/* Set all DMA channels (16 max.) to non-secure */
	mmio_write_32(SUNXI_DMA_BASE + DMA_SEC_REG, 0xffff);
}

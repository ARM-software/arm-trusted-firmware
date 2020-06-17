/*
 * Copyright (C) 2018 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <lib/mmio.h>

#include <plat_marvell.h>

void marvell_bl1_setup_mpps(void)
{
	/* Enable UART MPPs.
	 ** In a normal system, this is done by Bootrom.
	 */
	mmio_write_32(MVEBU_AP_MPP_REGS(1), 0x3000);
	mmio_write_32(MVEBU_AP_MPP_REGS(2), 0x3000);
}

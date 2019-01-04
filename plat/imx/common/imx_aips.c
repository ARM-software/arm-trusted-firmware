/*
 * Copyright (c) 2018, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/mmio.h>
#include <lib/utils_def.h>

#include <imx_aips.h>
#include <imx_regs.h>

static void imx_aips_set_default_access(struct aipstz_regs *aips_regs)
{
	int i;
	uintptr_t addr;

	/*
	 * See section 4.7.7.1 AIPSTZ_MPR field descriptions
	 * i.MX 7Solo Applications Processor Reference Manual, Rev. 0.1, 08/2016
	 * 0111 ->
	 *	0: Write Access from master not buffered
	 *	1: Master is trusted for read access
	 *	1: Master is trsuted for write access
	 *	1: Access from master is not forced to user mode
	 */
	addr = (uintptr_t)&aips_regs->aipstz_mpr;
	mmio_write_32(addr, 0x77777777);

	/*
	 * Helpfully the OPACR registers have the logical inversion of the above
	 * See section 4.7.7.1 AIPSTZ_MPR field descriptions
	 * i.MX 7Solo Applications Processor Reference Manual, Rev. 0.1, 08/2016
	 * 0000 ->
	 *	0: Write Access to the peripheral is not buffered by AIPSTZ
	 *	0: The peripheral does not require supervisor priv to access
	 *	0: Master is trsuted for write access
	 *	0: Access from master is not forced to user mode
	 */
	for (i = 0; i < AIPSTZ_OAPCR_COUNT; i++) {
		addr = (uintptr_t)&aips_regs->aipstz_opacr[i];
		mmio_write_32(addr, 0x00000000);
	}
}

void imx_aips_init(void)
{
	int i;
	struct aipstz_regs *aips_regs[] = {
		(struct aipstz_regs *)(AIPS1_BASE + AIPSTZ_CONFIG_OFFSET),
		(struct aipstz_regs *)(AIPS2_BASE + AIPSTZ_CONFIG_OFFSET),
		(struct aipstz_regs *)(AIPS3_BASE + AIPSTZ_CONFIG_OFFSET),
	};

	for (i = 0; i < ARRAY_SIZE(aips_regs); i++)
		imx_aips_set_default_access(aips_regs[i]);
}

/*
 * Copyright (C) 2019 Marvell International Ltd.
 *
 * SPDX-License-Identifier:     BSD-3-Clause
 * https://spdx.org/licenses
 */

#include <a8k_plat_def.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <mvebu.h>

/* CONFI REGISTERS */
#define MG_CM3_CONFI_BASE(CP)		(MVEBU_CP_REGS_BASE(CP) + 0x100000)
#define MG_CM3_CONFI_GLOB_CFG_REG(CP)	(MG_CM3_CONFI_BASE(CP) + 0x2B500)
#define CM3_CPU_EN_BIT			BIT(28)
#define MG_CM3_MG_INT_MFX_REG(CP)	(MG_CM3_CONFI_BASE(CP) + 0x2B054)
#define CM3_SYS_RESET_BIT		BIT(0)

void mg_start_ap_fw(int cp_nr)
{
	if (mmio_read_32(MG_CM3_CONFI_GLOB_CFG_REG(cp_nr)) & CM3_CPU_EN_BIT) {
		VERBOSE("cm3 already running\n");
		return;  /* cm3 already running */
	}

	mmio_setbits_32(MG_CM3_CONFI_GLOB_CFG_REG(cp_nr), CM3_CPU_EN_BIT);
	mmio_setbits_32(MG_CM3_MG_INT_MFX_REG(cp_nr), CM3_SYS_RESET_BIT);

	/* TODO: add some routine for checking if ap process is running, if not
	 * disable cm3.
	 */
}

/*
 * Copyright (c) 2026, STMicroelectronics - All Rights Reserved
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <drivers/st/stm32mp_tamp_nvram.h>
#include <lib/mmio.h>

#include <platform_def.h>

#define BKPREG_ZONE_1 U(0)
#define BKPREG_ZONE_2 U(1)
#define BKPREG_ZONE_3 U(2)

static const struct stm32_tamp_nvram_reg_field
	mp1_zone_cfg_fields[MAX_TAMP_BACKUP_REGS_ZONES - 1] = {
		[BKPREG_ZONE_1] = REG_FIELD(TAMP_SECCFGR, 0, 7),
		[BKPREG_ZONE_2] = REG_FIELD(TAMP_SECCFGR, 16, 23),
	};

static const struct stm32_tamp_nvram_cdata stm32mp1_tamp_nvram = {
	.nb_zones = MAX_TAMP_BACKUP_REGS_ZONES,
	.zone_cfg = mp1_zone_cfg_fields,
};

static enum stm32_tamp_bkpreg_access mp1_bkpreg_access[MAX_TAMP_BACKUP_REGS_ZONES] = {
	[BKPREG_ZONE_1] = RSWS,
	[BKPREG_ZONE_2] = RNSWS,
	[BKPREG_ZONE_3] = RNSWNS,
};

const struct stm32_tamp_nvram_cdata *stm32_tamp_nvram_get_cdata(void)
{
	return &stm32mp1_tamp_nvram;
}

enum stm32_tamp_bkpreg_access *
stm32_tamp_nvram_get_access_rights(struct stm32_tamp_nvram_drv_data *drv_data)
{
	return &mp1_bkpreg_access[0];
}

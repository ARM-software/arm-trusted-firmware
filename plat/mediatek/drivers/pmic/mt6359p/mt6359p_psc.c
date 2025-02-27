/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/pmic/pmic_psc.h>
#include <lib/mtk_init/mtk_init.h>
#include <pmic_wrap_init_common.h>

#include "registers.h"

static const struct pmic_psc_reg mt6359p_psc_regs[] = {
	PMIC_PSC_REG(RG_PWRHOLD, MT6359P_PPCCTL0, 0),
	PMIC_PSC_REG(RG_CRST, MT6359P_PPCCTL1, 0),
	PMIC_PSC_REG(RG_SMART_RST_SDN_EN, MT6359P_STRUP_CON12, 9),
	PMIC_PSC_REG(RG_SMART_RST_MODE, MT6359P_STRUP_CON12, 10),
};

static const struct pmic_psc_config mt6359p_psc_config = {
	.read_field = pwrap_read_field,
	.write_field = pwrap_write_field,
	.regs = mt6359p_psc_regs,
	.reg_size = ARRAY_SIZE(mt6359p_psc_regs),
};

static int mt6359p_psc_init(void)
{
	return pmic_psc_register(&mt6359p_psc_config);
}

MTK_PLAT_SETUP_0_INIT(mt6359p_psc_init);

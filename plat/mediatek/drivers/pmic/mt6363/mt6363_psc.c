/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/pmic/pmic_psc.h>
#include <drivers/spmi_api.h>
#include <lib/mtk_init/mtk_init.h>

#include "registers.h"

static struct spmi_device *sdev;

static const struct pmic_psc_reg mt6363_psc_regs[] = {
	PMIC_PSC_REG(RG_PWRHOLD, MT6363_PPCCTL0, 0),
	PMIC_PSC_REG(RG_CRST, MT6363_PPCCTL1, 0),
	PMIC_PSC_REG(RG_SMART_RST_SDN_EN, MT6363_STRUP_CON12, 1),
	PMIC_PSC_REG(RG_SMART_RST_MODE, MT6363_STRUP_CON12, 2),
};

static int mt6363_psc_read_field(uint32_t reg, uint32_t *val, uint32_t mask, uint32_t shift)
{
	uint8_t rdata = 0;
	int ret = 0;

	if (!val)
		return -EINVAL;
	if (!sdev)
		return -ENODEV;
	ret = spmi_ext_register_readl(sdev, reg, &rdata, 1);
	if (ret < 0)
		return ret;

	rdata &= (mask << shift);
	*val = (rdata >> shift);

	return 0;
}

static int mt6363_psc_write_field(uint32_t reg, uint32_t val, uint32_t mask, uint32_t shift)
{
	uint8_t org = 0;
	int ret = 0;

	if (!sdev)
		return -ENODEV;
	ret = spmi_ext_register_readl(sdev, reg, &org, 1);
	if (ret < 0)
		return ret;

	org &= ~(mask << shift);
	org |= (val << shift);

	ret = spmi_ext_register_writel(sdev, reg, &org, 1);
	return ret;
}

static const struct pmic_psc_config mt6363_psc_config = {
	.read_field = mt6363_psc_read_field,
	.write_field = mt6363_psc_write_field,
	.regs = mt6363_psc_regs,
	.reg_size = ARRAY_SIZE(mt6363_psc_regs),
};

static int mt6363_psc_init(void)
{
	sdev = get_spmi_device(SPMI_MASTER_1, SPMI_SLAVE_4);
	if (!sdev)
		ERROR("%s: get spmi device fail\n", __func__);
	return pmic_psc_register(&mt6363_psc_config);
}

MTK_PLAT_SETUP_0_INIT(mt6363_psc_init);

#ifdef CONFIG_MTK_PMIC_SPT_SUPPORT
static int mt6363_spt_enable(void)
{
	/* Enable PMIC Self-Protection Timer(SPT) */
	return mt6363_psc_write_field(MT6363_RG_SELFWDT_EN_ADDR, MT6363_RG_SELFWDT_EN_MASK,
				      MT6363_RG_SELFWDT_EN_MASK, 0);
}

MTK_PLAT_RUNTIME_INIT(mt6363_spt_enable);
#endif

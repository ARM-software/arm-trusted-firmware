/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>

#include <common/debug.h>
#include <drivers/pmic/pmic_psc.h>
#ifdef CONFIG_MTK_PMIC_SHUTDOWN_CFG
#include <drivers/pmic/pmic_shutdown_cfg.h>
#endif

#define ERR_INVALID_ARGS	-EINVAL
#define ERR_NOT_CONFIGURED	-ENODEV

static const struct pmic_psc_config *pmic_psc;

static uint32_t read_pmic_psc_reg(enum pmic_psc_reg_name reg_name)
{
	uint32_t val = 0;
	const struct pmic_psc_reg *reg;

	if (reg_name >= pmic_psc->reg_size)
		return 0;

	reg = &pmic_psc->regs[reg_name];
	pmic_psc->read_field(reg->reg_addr, &val, reg->reg_mask, reg->reg_shift);
	return val;
}

static int set_pmic_psc_reg(enum pmic_psc_reg_name reg_name)
{
	const struct pmic_psc_reg *reg;

	if (reg_name >= pmic_psc->reg_size)
		return ERR_INVALID_ARGS;

	reg = &pmic_psc->regs[reg_name];
	pmic_psc->write_field(reg->reg_addr, 1, reg->reg_mask, reg->reg_shift);
	return 0;
}

static int clr_pmic_psc_reg(enum pmic_psc_reg_name reg_name)
{
	const struct pmic_psc_reg *reg;

	if (reg_name >= pmic_psc->reg_size)
		return ERR_INVALID_ARGS;

	reg = &pmic_psc->regs[reg_name];
	pmic_psc->write_field(reg->reg_addr, 0, reg->reg_mask, reg->reg_shift);
	return 0;
}

int enable_pmic_smart_reset(bool enable)
{
	if (!pmic_psc)
		return ERR_NOT_CONFIGURED;
	if (enable)
		set_pmic_psc_reg(RG_SMART_RST_MODE);
	else
		clr_pmic_psc_reg(RG_SMART_RST_MODE);
	return 0;
}

int enable_pmic_smart_reset_shutdown(bool enable)
{
	if (!pmic_psc)
		return ERR_NOT_CONFIGURED;
	if (enable)
		set_pmic_psc_reg(RG_SMART_RST_SDN_EN);
	else
		clr_pmic_psc_reg(RG_SMART_RST_SDN_EN);
	return 0;
}

int platform_cold_reset(void)
{
	if (!pmic_psc)
		return ERR_NOT_CONFIGURED;
	/* Some PMICs may not support cold reset */
	if (!pmic_psc->regs[RG_CRST].reg_addr)
		return ERR_NOT_CONFIGURED;
	set_pmic_psc_reg(RG_CRST);
	return 0;
}

int platform_power_hold(bool hold)
{
	int use_spmi_cmd_sdn = 0;

	if (!pmic_psc)
		return ERR_NOT_CONFIGURED;
	if (hold)
		set_pmic_psc_reg(RG_PWRHOLD);
	else {
#ifdef CONFIG_MTK_PMIC_SHUTDOWN_CFG
		use_spmi_cmd_sdn = pmic_shutdown_cfg();
#endif
		if (use_spmi_cmd_sdn == 1)
			spmi_shutdown();
		else
			clr_pmic_psc_reg(RG_PWRHOLD);
	}
	return 0;
}

int pmic_psc_register(const struct pmic_psc_config *psc)
{
	if (!psc || !psc->regs || !psc->read_field || !psc->write_field)
		return ERR_INVALID_ARGS;
	pmic_psc = psc;
	INFO("POWER_HOLD=0x%x\n", read_pmic_psc_reg(RG_PWRHOLD));
	return 0;
}

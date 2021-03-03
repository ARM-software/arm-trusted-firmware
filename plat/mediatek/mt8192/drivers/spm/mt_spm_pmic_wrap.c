/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>

#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_pmic_wrap.h>
#include <mt_spm_reg.h>
#include <plat_pm.h>
#include <platform_def.h>

/* PMIC_WRAP MT6359 */
#define VCORE_BASE_UV		40000
#define VOLT_TO_PMIC_VAL(volt)	(((volt) - VCORE_BASE_UV + 625 - 1) / 625)
#define PMIC_VAL_TO_VOLT(pmic)	(((pmic) * 625) + VCORE_BASE_UV)

#define NR_PMIC_WRAP_CMD	(NR_IDX_ALL)
#define SPM_DATA_SHIFT		16

#define BUCK_VGPU11_ELR0	0x15B4
#define TOP_SPI_CON0		0x0456
#define BUCK_TOP_CON1		0x1443
#define TOP_CON			0x0013
#define TOP_DIG_WPK		0x03a9
#define TOP_CON_LOCK		0x03a8
#define TOP_CLK_CON0		0x0134

struct pmic_wrap_cmd {
	unsigned long cmd_addr;
	unsigned long cmd_wdata;
};

struct pmic_wrap_setting {
	enum pmic_wrap_phase_id phase;
	struct pmic_wrap_cmd addr[NR_PMIC_WRAP_CMD];
	struct {
		struct {
			unsigned long cmd_addr;
			unsigned long cmd_wdata;
		} _[NR_PMIC_WRAP_CMD];
		const int nr_idx;
	} set[NR_PMIC_WRAP_PHASE];
};

static struct pmic_wrap_setting pw = {
	.phase = NR_PMIC_WRAP_PHASE,    /* invalid setting for init */
	.addr = { {0UL, 0UL} },
	.set[PMIC_WRAP_PHASE_ALLINONE] = {
		._[CMD_0]	= {BUCK_VGPU11_ELR0, VOLT_TO_PMIC_VAL(72500),},
		._[CMD_1]	= {BUCK_VGPU11_ELR0, VOLT_TO_PMIC_VAL(65000),},
		._[CMD_2]	= {BUCK_VGPU11_ELR0, VOLT_TO_PMIC_VAL(60000),},
		._[CMD_3]	= {BUCK_VGPU11_ELR0, VOLT_TO_PMIC_VAL(57500),},
		._[CMD_4]	= {TOP_SPI_CON0, 0x1,},
		._[CMD_5]	= {TOP_SPI_CON0, 0x0,},
		._[CMD_6]	= {BUCK_TOP_CON1, 0x0,},
		._[CMD_7]	= {BUCK_TOP_CON1, 0xf,},
		._[CMD_8]	= {TOP_CON, 0x3,},
		._[CMD_9]	= {TOP_CON, 0x0,},
		._[CMD_10]	= {TOP_DIG_WPK, 0x63,},
		._[CMD_11]	= {TOP_CON_LOCK, 0x15,},
		._[CMD_12]	= {TOP_DIG_WPK, 0x0,},
		._[CMD_13]	= {TOP_CON_LOCK, 0x0,},
		._[CMD_14]	= {TOP_CLK_CON0, 0x40,},
		._[CMD_15]	= {TOP_CLK_CON0, 0x0,},
		.nr_idx = NR_IDX_ALL,
	},
};

void _mt_spm_pmic_table_init(void)
{
	struct pmic_wrap_cmd pwrap_cmd_default[NR_PMIC_WRAP_CMD] = {
		{(uint32_t)SPM_DVFS_CMD0, (uint32_t)SPM_DVFS_CMD0,},
		{(uint32_t)SPM_DVFS_CMD1, (uint32_t)SPM_DVFS_CMD1,},
		{(uint32_t)SPM_DVFS_CMD2, (uint32_t)SPM_DVFS_CMD2,},
		{(uint32_t)SPM_DVFS_CMD3, (uint32_t)SPM_DVFS_CMD3,},
		{(uint32_t)SPM_DVFS_CMD4, (uint32_t)SPM_DVFS_CMD4,},
		{(uint32_t)SPM_DVFS_CMD5, (uint32_t)SPM_DVFS_CMD5,},
		{(uint32_t)SPM_DVFS_CMD6, (uint32_t)SPM_DVFS_CMD6,},
		{(uint32_t)SPM_DVFS_CMD7, (uint32_t)SPM_DVFS_CMD7,},
		{(uint32_t)SPM_DVFS_CMD8, (uint32_t)SPM_DVFS_CMD8,},
		{(uint32_t)SPM_DVFS_CMD9, (uint32_t)SPM_DVFS_CMD9,},
		{(uint32_t)SPM_DVFS_CMD10, (uint32_t)SPM_DVFS_CMD10,},
		{(uint32_t)SPM_DVFS_CMD11, (uint32_t)SPM_DVFS_CMD11,},
		{(uint32_t)SPM_DVFS_CMD12, (uint32_t)SPM_DVFS_CMD12,},
		{(uint32_t)SPM_DVFS_CMD13, (uint32_t)SPM_DVFS_CMD13,},
		{(uint32_t)SPM_DVFS_CMD14, (uint32_t)SPM_DVFS_CMD14,},
		{(uint32_t)SPM_DVFS_CMD15, (uint32_t)SPM_DVFS_CMD15,},
	};

	memcpy(pw.addr, pwrap_cmd_default, sizeof(pwrap_cmd_default));
}

void mt_spm_pmic_wrap_set_phase(enum pmic_wrap_phase_id phase)
{
	uint32_t idx, addr, data;

	if (phase >= NR_PMIC_WRAP_PHASE) {
		return;
	}

	if (pw.phase == phase) {
		return;
	}

	if (pw.addr[0].cmd_addr == 0UL) {
		_mt_spm_pmic_table_init();
	}

	pw.phase = phase;
	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);

	for (idx = 0U; idx < pw.set[phase].nr_idx; idx++) {
		addr = pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT;
		data = pw.set[phase]._[idx].cmd_wdata;
		mmio_write_32(pw.addr[idx].cmd_addr, addr | data);
	}
}

void mt_spm_pmic_wrap_set_cmd(enum pmic_wrap_phase_id phase, uint32_t idx,
			      uint32_t cmd_wdata)
{
	uint32_t addr;

	if (phase >= NR_PMIC_WRAP_PHASE) {
		return;
	}

	if (idx >= pw.set[phase].nr_idx) {
		return;
	}

	pw.set[phase]._[idx].cmd_wdata = cmd_wdata;
	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);

	if (pw.phase == phase) {
		addr = pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT;
		mmio_write_32(pw.addr[idx].cmd_addr, addr | cmd_wdata);
	}
}

uint64_t mt_spm_pmic_wrap_get_cmd(enum pmic_wrap_phase_id phase, uint32_t idx)
{
	if (phase >= NR_PMIC_WRAP_PHASE) {
		return 0UL;
	}

	if (idx >= pw.set[phase].nr_idx) {
		return 0UL;
	}

	return pw.set[phase]._[idx].cmd_wdata;
}

/*
 * Copyright (c) 2022, MediaTek Inc. All rights reserved.
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
#include <pmic.h>

/* PMIC_WRAP MT6359 */
#define NR_PMIC_WRAP_CMD	(NR_IDX_ALL)
#define SPM_DATA_SHIFT		(16U)

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

struct set_vsram {
	unsigned long cmd_addr;
	unsigned long cmd_wdata;
};

/* MT6366 */
#define VOLT_TO_PMIC_VAL_66(volt)	(((volt) - 50000 + 625 - 1) / 625)
#define BUCK_VCORE_ELR0_66		(0x14AA)
#define TOP_SPI_CON0_66			(0x44C)

static struct pmic_wrap_setting pw66 = {
	.phase = NR_PMIC_WRAP_PHASE,    /* invalid setting for init */
	.addr = { {0UL, 0UL} },
	.set[PMIC_WRAP_PHASE_ALLINONE] = {
		._[CMD_0] = { BUCK_VCORE_ELR0_66, VOLT_TO_PMIC_VAL_66(80000), },
		._[CMD_1] = { BUCK_VCORE_ELR0_66, VOLT_TO_PMIC_VAL_66(80000), },
		._[CMD_2] = { BUCK_VCORE_ELR0_66, VOLT_TO_PMIC_VAL_66(80000), },
		._[CMD_3] = { BUCK_VCORE_ELR0_66, VOLT_TO_PMIC_VAL_66(80000), },
		._[CMD_4] = { BUCK_VCORE_ELR0_66, VOLT_TO_PMIC_VAL_66(80000), },
		._[CMD_5] = { TOP_SPI_CON0_66, 0x1, },
		._[CMD_6] = { TOP_SPI_CON0_66, 0x0, },
		.nr_idx = NR_IDX_ALL,
	},
};

/* MT6357 */
#define VOLT_TO_PMIC_VAL_57(volt)	(((volt) - 51875 + 625 - 1) / 625)
#define BUCK_VCORE_ELR0_57		(0x152A)
#define TOP_SPI_CON0_57			(0x448)

static struct pmic_wrap_setting pw57 = {
	.phase = NR_PMIC_WRAP_PHASE,    /* invalid setting for init */
	.addr = { {0UL, 0UL} },
	.set[PMIC_WRAP_PHASE_ALLINONE] = {
		._[CMD_0] = { BUCK_VCORE_ELR0_57, VOLT_TO_PMIC_VAL_57(80000), },
		._[CMD_1] = { BUCK_VCORE_ELR0_57, VOLT_TO_PMIC_VAL_57(75000), },
		._[CMD_2] = { BUCK_VCORE_ELR0_57, VOLT_TO_PMIC_VAL_57(70000), },
		._[CMD_3] = { BUCK_VCORE_ELR0_57, VOLT_TO_PMIC_VAL_57(65000), },
		._[CMD_4] = { BUCK_VCORE_ELR0_57, VOLT_TO_PMIC_VAL_57(62500), },
		._[CMD_5] = { TOP_SPI_CON0_57, 0x1, },
		._[CMD_6] = { TOP_SPI_CON0_57, 0x0, },
		.nr_idx = NR_IDX_ALL,
	},
};

static struct pmic_wrap_setting *pw;

#define IS_PMIC_57() ((pmic_get_hwcid() >> 8) == 0x57)

void _mt_spm_pmic_table_init(void)
{
	struct pmic_wrap_cmd pwrap_cmd_default[NR_PMIC_WRAP_CMD] = {
		{ (uint32_t)SPM_DVFS_CMD0, (uint32_t)SPM_DVFS_CMD0, },
		{ (uint32_t)SPM_DVFS_CMD1, (uint32_t)SPM_DVFS_CMD1, },
		{ (uint32_t)SPM_DVFS_CMD2, (uint32_t)SPM_DVFS_CMD2, },
		{ (uint32_t)SPM_DVFS_CMD3, (uint32_t)SPM_DVFS_CMD3, },
		{ (uint32_t)SPM_DVFS_CMD4, (uint32_t)SPM_DVFS_CMD4, },
		{ (uint32_t)SPM_DVFS_CMD5, (uint32_t)SPM_DVFS_CMD5, },
		{ (uint32_t)SPM_DVFS_CMD6, (uint32_t)SPM_DVFS_CMD6, },
		{ (uint32_t)SPM_DVFS_CMD7, (uint32_t)SPM_DVFS_CMD7, },
		{ (uint32_t)SPM_DVFS_CMD8, (uint32_t)SPM_DVFS_CMD8, },
	};

	if (IS_PMIC_57()) {
		pw = &pw57;
	} else {
		pw = &pw66;
	}

	memcpy(pw->addr, pwrap_cmd_default, sizeof(pwrap_cmd_default));
}

void mt_spm_pmic_wrap_set_phase(enum pmic_wrap_phase_id phase)
{
	uint32_t idx, addr, data;

	if (phase < NR_PMIC_WRAP_PHASE) {
		if (pw == NULL || pw->addr[0].cmd_addr == 0) {
			_mt_spm_pmic_table_init();
		}

		if (pw->phase != phase) {
			pw->phase = phase;

			mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);

			for (idx = 0; idx < pw->set[phase].nr_idx; idx++) {
				addr = pw->set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT;
				data = pw->set[phase]._[idx].cmd_wdata;
				mmio_write_32(pw->addr[idx].cmd_addr, addr | data);
			}
		}
	}
}

void mt_spm_pmic_wrap_set_cmd(enum pmic_wrap_phase_id phase, uint32_t idx,
			      uint32_t cmd_wdata)
{
	uint32_t addr;

	if (phase >= NR_PMIC_WRAP_PHASE) {
		return;
	}

	if (pw == NULL || idx >= pw->set[phase].nr_idx) {
		return;
	}

	pw->set[phase]._[idx].cmd_wdata = cmd_wdata;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);
	if (pw->phase == phase) {
		addr = pw->set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT;
		mmio_write_32(pw->addr[idx].cmd_addr, addr | cmd_wdata);
	}
}

uint64_t mt_spm_pmic_wrap_get_cmd(enum pmic_wrap_phase_id phase, uint32_t idx)
{
	uint64_t ret = 0UL;

	if ((phase < NR_PMIC_WRAP_PHASE) &&
	    (pw != NULL && idx < pw->set[phase].nr_idx)) {
		ret = pw->set[phase]._[idx].cmd_wdata;
	}

	return ret;
}

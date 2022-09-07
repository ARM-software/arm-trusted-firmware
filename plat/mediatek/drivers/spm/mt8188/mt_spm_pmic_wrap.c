/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>

#include <common/debug.h>
#include <lib/mmio.h>
#include <plat/common/platform.h>

#include <lib/pm/mtk_pm.h>
#include "mt_spm.h"
#include "mt_spm_internal.h"
#include "mt_spm_pmic_wrap.h"
#include "mt_spm_reg.h"
#include <platform_def.h>

/* BIT operation */
#define _BITS_(h, l, v) ((GENMASK(h, l) & ((v) << (l))))

/* PMIC_WRAP */
#define VCORE_BASE_UV			(40000) /* PMIC MT6359 */
#define VOLT_TO_PMIC_VAL(volt)		(((volt) - VCORE_BASE_UV + 625 - 1) / 625)

#define NR_PMIC_WRAP_CMD		(NR_IDX_ALL)
#define SPM_DATA_SHIFT			(16)

#define BUCK_VGPU11_ELR0		(0x15B4)
#define TOP_SPI_CON0			(0x0456)
#define BUCK_TOP_CON1			(0x1443) /* PMIC MT6315 */
#define TOP_CON				(0x0013) /* PMIC MT6315 */
#define TOP_DIG_WPK			(0x03a9)
#define TOP_CON_LOCK			(0x03a8)
#define TOP_CLK_CON0			(0x0134) /* PMIC MT6359*/

struct pmic_wrap_cmd {
	uint32_t cmd_addr;
	uint32_t cmd_wdata;
};

struct pmic_wrap_setting {
	enum pmic_wrap_phase_id phase;
	struct pmic_wrap_cmd addr[NR_PMIC_WRAP_CMD];
	struct {
		struct {
			uint32_t cmd_addr;
			uint32_t cmd_wdata;
		} _[NR_PMIC_WRAP_CMD];
		const int nr_idx;
	} set[NR_PMIC_WRAP_PHASE];
};

static struct pmic_wrap_setting pw = {
	.phase = NR_PMIC_WRAP_PHASE,	/* invalid setting for init */
	.addr = {{0, 0} },
	.set[PMIC_WRAP_PHASE_ALLINONE] = {
		._[CMD_0]	= {BUCK_VGPU11_ELR0, _BITS_(6, 0, VOLT_TO_PMIC_VAL(75000)),},
		._[CMD_1]	= {BUCK_VGPU11_ELR0, _BITS_(6, 0, VOLT_TO_PMIC_VAL(65000)),},
		._[CMD_2]	= {BUCK_VGPU11_ELR0, _BITS_(6, 0, VOLT_TO_PMIC_VAL(60000)),},
		._[CMD_3]	= {BUCK_VGPU11_ELR0, _BITS_(6, 0, VOLT_TO_PMIC_VAL(55000)),},
		._[CMD_4]	= {TOP_SPI_CON0, _BITS_(0, 0, 1),},
		._[CMD_5]	= {TOP_SPI_CON0, _BITS_(0, 0, 0),},
		._[CMD_6]	= {BUCK_TOP_CON1, 0x0,},	/* MT6315-3: VMD NO LP */
		._[CMD_7]	= {BUCK_TOP_CON1, 0xF,},	/* MT6315-3: VMD LP */
		._[CMD_8]	= {TOP_CON, 0x3,},		/* MT6315-3: PMIC NO LP */
		._[CMD_9]	= {TOP_CON, 0x0,},		/* MT6315-3: PMIC LP */
		._[CMD_10]	= {TOP_DIG_WPK, 0x63,},		/* MT6315-2: PMIC_CON_DIG_WPK */
		._[CMD_11]	= {TOP_CON_LOCK, 0x15,},	/* MT6315-2: PMIC_CON_UNLOCK */
		._[CMD_12]	= {TOP_DIG_WPK, 0x0,},		/* MT6315-2: PMIC_CON_DIG_WPK */
		._[CMD_13]	= {TOP_CON_LOCK, 0x0,},		/* MT6315-2: PMIC_CON_LOCK */
		._[CMD_14]	= {TOP_CLK_CON0, 0x0040,},	/* MT6359: 6359_LDO_SW_SEL_H */
		._[CMD_15]	= {TOP_CLK_CON0, 0x0000,},	/* MT6359: 6359_LDO_SW_SEL_L */
		.nr_idx = NR_IDX_ALL,
	},
};

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
		{ (uint32_t)SPM_DVFS_CMD9, (uint32_t)SPM_DVFS_CMD9, },
		{ (uint32_t)SPM_DVFS_CMD10, (uint32_t)SPM_DVFS_CMD10, },
		{ (uint32_t)SPM_DVFS_CMD11, (uint32_t)SPM_DVFS_CMD11, },
		{ (uint32_t)SPM_DVFS_CMD12, (uint32_t)SPM_DVFS_CMD12, },
		{ (uint32_t)SPM_DVFS_CMD13, (uint32_t)SPM_DVFS_CMD13, },
		{ (uint32_t)SPM_DVFS_CMD14, (uint32_t)SPM_DVFS_CMD14, },
		{ (uint32_t)SPM_DVFS_CMD15, (uint32_t)SPM_DVFS_CMD15, },
	};

	memcpy(pw.addr, pwrap_cmd_default, sizeof(pwrap_cmd_default));
}

void mt_spm_pmic_wrap_set_phase(enum pmic_wrap_phase_id phase)
{
	int idx;

	if ((phase >= NR_PMIC_WRAP_PHASE) || (pw.phase == phase)) {
		return;
	}

	if (pw.addr[0].cmd_addr == 0) {
		_mt_spm_pmic_table_init();
	}

	pw.phase = phase;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);
	for (idx = 0; idx < pw.set[phase].nr_idx; idx++) {
		mmio_write_32(pw.addr[idx].cmd_addr,
			      (pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT) |
			      (pw.set[phase]._[idx].cmd_wdata));
	}
}

void mt_spm_pmic_wrap_set_cmd(enum pmic_wrap_phase_id phase, unsigned int idx,
			      unsigned int cmd_wdata)
{
	/* just set wdata value */
	if ((phase >= NR_PMIC_WRAP_PHASE) || (idx >= pw.set[phase].nr_idx)) {
		return;
	}

	pw.set[phase]._[idx].cmd_wdata = cmd_wdata;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY | BCLK_CG_EN_LSB);
	if (pw.phase == phase) {
		mmio_write_32(pw.addr[idx].cmd_addr,
			      (pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT) | cmd_wdata);
	}
}

uint64_t mt_spm_pmic_wrap_get_cmd(enum pmic_wrap_phase_id phase, unsigned int idx)
{
	/* just get wdata value */
	if ((phase >= NR_PMIC_WRAP_PHASE) || (idx >= pw.set[phase].nr_idx)) {
		return 0;
	}

	return pw.set[phase]._[idx].cmd_wdata;
}

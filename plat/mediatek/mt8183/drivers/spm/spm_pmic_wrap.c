/*
 * Copyright (c) 2019, ARM Limited and Contributors. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>
#include <lib/mmio.h>
#include <platform_def.h>
#include <spm.h>
#include <spm_pmic_wrap.h>
#include <lib/libc/string.h>

#define SLEEP_REG_MD_SPM_DVFS_CMD20	(SLEEP_REG_MD_BASE + 0x010)
#define SLEEP_REG_MD_SPM_DVFS_CMD21	(SLEEP_REG_MD_BASE + 0x014)
#define SLEEP_REG_MD_SPM_DVFS_CMD22	(SLEEP_REG_MD_BASE + 0x018)
#define SLEEP_REG_MD_SPM_DVFS_CMD23	(SLEEP_REG_MD_BASE + 0x01C)

/* PMIC_WRAP -> PMIC MT6358 */
#define VCORE_BASE_UV 50000
#define VOLT_TO_PMIC_VAL(volt)  (((volt) - VCORE_BASE_UV + 625 - 1) / 625)
#define PMIC_VAL_TO_VOLT(pmic)  (((pmic) * 625) + VCORE_BASE_UV)

#define DEFAULT_VOLT_VSRAM      (100000)
#define DEFAULT_VOLT_VCORE      (100000)
#define NR_PMIC_WRAP_CMD	(NR_IDX_ALL)
#define MAX_RETRY_COUNT		(100)
#define SPM_DATA_SHIFT		(16)

#define BUCK_VCORE_ELR0		0x14AA
#define BUCK_VPROC12_CON0	0x1408
#define BUCK_VPROC11_CON0	0x1388
#define TOP_SPI_CON0		0x044C
#define LDO_VSRAM_PROC12_CON0	0x1B88
#define LDO_VSRAM_PROC11_CON0	0x1B46
#define BUCK_VMODEM_ELR0	0x15A6

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
	.phase = NR_PMIC_WRAP_PHASE,
	.addr = {{0, 0} },
	.set[PMIC_WRAP_PHASE_ALLINONE] = {
		._[CMD_0]    = {BUCK_VCORE_ELR0, VOLT_TO_PMIC_VAL(70000),},
		._[CMD_1]    = {BUCK_VCORE_ELR0, VOLT_TO_PMIC_VAL(80000),},
		._[CMD_2]    = {BUCK_VPROC12_CON0, 0x3,},
		._[CMD_3]    = {BUCK_VPROC12_CON0, 0x1,},
		._[CMD_4]    = {BUCK_VPROC11_CON0, 0x3,},
		._[CMD_5]    = {BUCK_VPROC11_CON0, 0x1,},
		._[CMD_6]    = {TOP_SPI_CON0, 0x1,},
		._[CMD_7]    = {TOP_SPI_CON0, 0x0,},
		._[CMD_8]    = {BUCK_VPROC12_CON0, 0x0,},
		._[CMD_9]    = {BUCK_VPROC12_CON0, 0x1,},
		._[CMD_10]   = {BUCK_VPROC11_CON0, 0x0,},
		._[CMD_11]   = {BUCK_VPROC11_CON0, 0x1,},
		._[CMD_12]   = {LDO_VSRAM_PROC12_CON0, 0x0,},
		._[CMD_13]   = {LDO_VSRAM_PROC12_CON0, 0x1,},
		._[CMD_14]   = {LDO_VSRAM_PROC11_CON0, 0x0,},
		._[CMD_15]   = {LDO_VSRAM_PROC11_CON0, 0x1,},
		._[CMD_20]   = {BUCK_VMODEM_ELR0, VOLT_TO_PMIC_VAL(55000),},
		._[CMD_21]   = {BUCK_VCORE_ELR0, VOLT_TO_PMIC_VAL(60000),},
		._[CMD_22]   = {LDO_VSRAM_PROC11_CON0, 0x3,},
		._[CMD_23]   = {LDO_VSRAM_PROC11_CON0, 0x1,},
		.nr_idx = NR_IDX_ALL
	}
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
		{(uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD20,
		 (uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD20,},
		{(uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD21,
		 (uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD21,},
		{(uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD22,
		 (uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD22,},
		{(uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD23,
		 (uint32_t)SLEEP_REG_MD_SPM_DVFS_CMD23,}
	};

	memcpy(pw.addr, pwrap_cmd_default, sizeof(pwrap_cmd_default));
}

void mt_spm_pmic_wrap_set_phase(enum pmic_wrap_phase_id phase)
{
	uint32_t idx, addr, data;

	if (phase >= NR_PMIC_WRAP_PHASE)
		return;

	if (pw.phase == phase)
		return;

	if (pw.addr[0].cmd_addr == 0)
		_mt_spm_pmic_table_init();

	pw.phase = phase;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY |
		      BCLK_CG_EN_LSB | MD_BCLK_CG_EN_LSB);
	for (idx = 0; idx < pw.set[phase].nr_idx; idx++) {
		addr = pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT;
		data = pw.set[phase]._[idx].cmd_wdata;
		mmio_write_32(pw.addr[idx].cmd_addr, addr | data);
	}
}

void mt_spm_pmic_wrap_set_cmd(enum pmic_wrap_phase_id phase, uint32_t idx,
			      uint32_t cmd_wdata)
{
	uint32_t addr;

	if (phase >= NR_PMIC_WRAP_PHASE)
		return;

	if (idx >= pw.set[phase].nr_idx)
		return;

	pw.set[phase]._[idx].cmd_wdata = cmd_wdata;

	mmio_write_32(POWERON_CONFIG_EN, SPM_REGWR_CFG_KEY |
		      BCLK_CG_EN_LSB | MD_BCLK_CG_EN_LSB);
	if (pw.phase == phase) {
		addr = pw.set[phase]._[idx].cmd_addr << SPM_DATA_SHIFT;
		mmio_write_32(pw.addr[idx].cmd_addr, addr | cmd_wdata);
	}
}

uint64_t mt_spm_pmic_wrap_get_cmd(enum pmic_wrap_phase_id phase, uint32_t idx)
{
	if (phase >= NR_PMIC_WRAP_PHASE)
		return 0;

	if (idx >= pw.set[phase].nr_idx)
		return 0;

	return pw.set[phase]._[idx].cmd_wdata;
}


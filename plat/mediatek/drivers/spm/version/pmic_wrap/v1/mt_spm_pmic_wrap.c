/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <lib/libc/errno.h>

#include <lib/mmio.h>

#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>

#define SPM_DATA_SHIFT	16

static struct pmic_wrap_setting *pmic_wrap;

int mt_spm_pmic_wrap_set_phase(unsigned int phase)
{
	int idx;
	uint32_t cmd_addr, cmd_data;
	struct pmic_wrap_phase_setting *current_phase;

	if (!pmic_wrap)
		return -ENODEV;

	if (phase >= pmic_wrap->phase_nr_idx)
		return -EINVAL;

	current_phase = &pmic_wrap->phase[phase];

	for (idx = 0; idx < current_phase->nr_idx; idx++) {
		cmd_addr = current_phase->cmd[idx].cmd_addr;
		cmd_data = current_phase->cmd[idx].cmd_data;

		mmio_write_32(current_phase->cmd[idx].spm_pwrap_addr,
			      (cmd_addr << SPM_DATA_SHIFT) | cmd_data);
	}
	return 0;
}

int mt_spm_pmic_wrap_set_cmd(unsigned int phase,
			     unsigned int idx, unsigned int cmd_data)
{
	uint32_t cmd_addr;
	struct pmic_wrap_phase_setting *current_phase;

	if (!pmic_wrap)
		return -ENODEV;

	if (phase >= pmic_wrap->phase_nr_idx)
		return -EINVAL;

	if (idx >= pmic_wrap->phase[phase].nr_idx)
		return -EINVAL;

	current_phase = &pmic_wrap->phase[phase];
	current_phase->cmd[idx].cmd_data = cmd_data;
	cmd_addr = current_phase->cmd[idx].cmd_addr;

	mmio_write_32(current_phase->cmd[idx].spm_pwrap_addr,
		      (cmd_addr << SPM_DATA_SHIFT) | cmd_data);
	return 0;
}

unsigned long mt_spm_pmic_wrap_get_cmd(unsigned int phase, unsigned int idx)
{
	if (!pmic_wrap)
		return 0;

	if (phase >= pmic_wrap->phase_nr_idx)
		return 0;

	if (idx >= pmic_wrap->phase[phase].nr_idx)
		return 0;

	return pmic_wrap->phase[phase].cmd[idx].cmd_data;
}

void mt_spm_pmic_wrap_set_table(struct pmic_wrap_setting *pw)
{
	pmic_wrap = pw;
}

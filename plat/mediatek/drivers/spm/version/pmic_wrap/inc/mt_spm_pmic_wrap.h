/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_PMIC_WRAP_H
#define MT_SPM_PMIC_WRAP_H

struct pmic_wrap_cmd_setting {
	uint32_t spm_pwrap_addr;
	uint32_t cmd_addr;
	uint32_t cmd_data;
};

struct pmic_wrap_phase_setting {
	struct pmic_wrap_cmd_setting *cmd;
	unsigned int nr_idx;
};

struct pmic_wrap_setting {
	struct pmic_wrap_phase_setting *phase;
	const unsigned int phase_nr_idx;
};

int mt_spm_pmic_wrap_set_phase(unsigned int phase);
int mt_spm_pmic_wrap_set_cmd(unsigned int phase,
			     unsigned int idx, unsigned int cmd_data);
unsigned long mt_spm_pmic_wrap_get_cmd(unsigned int phase, unsigned int idx);
void mt_spm_pmic_wrap_set_table(struct pmic_wrap_setting *pw);

#endif /* MT_SPM_PMIC_WRAP_H */

/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdbool.h>

#include <lib/mmio.h>
#include <platform_def.h>

#include <mt_plat_spm_setting.h>
#include <mt_spm.h>
#include <mt_spm_internal.h>
#include <mt_spm_reg.h>
#include <pmic_wrap/inc/mt_spm_pmic_wrap.h>

/*
 * BIT Operation
 */
#define CMD_DATA(h, l, v) ((GENMASK(h, l) & ((v) << (l))))
#define VOLT_DATA(v) CMD_DATA(7, 0, VOLT_TO_PMIC_VAL(v))
/*
 * PMIC_WRAP
 */
#define VCORE_BASE_UV 0 /* PMIC MT6316 */
#define VOLT_TO_PMIC_VAL(volt)	(((volt) - VCORE_BASE_UV + 500 - 1) / 500)
#define PMIC_VAL_TO_VOLT(pmic)	(((pmic) * 500) + VCORE_BASE_UV)

#define NR_PMIC_WRAP_CMD	NR_IDX_ALL
#define MAX_RETRY_COUNT		100
#define SPM_DATA_SHIFT		16

/* MT6316 */
#define MT6316_TOP_VRCTL_VOSEL_VBUCK1	0x1448
#define MT6316_VCORE_VBUCK1_ON		0x1441
#define MT6316_VCORE_VBUCK1_OFF		0x1442

static struct pmic_wrap_cmd_setting cmd_table[NR_PMIC_WRAP_CMD] = {
	{SPM_PWRAP_CMD0, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(57500)},
	{SPM_PWRAP_CMD1, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(57500)},
	{SPM_PWRAP_CMD2, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(60000)},
	{SPM_PWRAP_CMD3, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(65000)},
	{SPM_PWRAP_CMD4, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(72500)},
	{SPM_PWRAP_CMD5, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(72500)},
	{SPM_PWRAP_CMD6, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(82500)},
	{SPM_PWRAP_CMD7, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(87500)},
	{SPM_PWRAP_CMD8, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(87500)},
	{SPM_PWRAP_CMD9, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(57500)},
	{SPM_PWRAP_CMD10, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(60000)},
	{SPM_PWRAP_CMD11, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(65000)},
	{SPM_PWRAP_CMD12, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(72500)},
	{SPM_PWRAP_CMD13, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(82500)},
	{SPM_PWRAP_CMD14, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(87500)},
	{SPM_PWRAP_CMD15, MT6316_VCORE_VBUCK1_ON, 1},
	{SPM_PWRAP_CMD16, MT6316_VCORE_VBUCK1_OFF, 1},
	{SPM_PWRAP_CMD17, MT6316_TOP_VRCTL_VOSEL_VBUCK1, VOLT_DATA(75000)},
	{SPM_PWRAP_CMD18, 0, 0},
	{SPM_PWRAP_CMD19, 0, 0},
	{SPM_PWRAP_CMD20, 0, 0},
	{SPM_PWRAP_CMD21, 0, 0},
	{SPM_PWRAP_CMD22, 0, 0},
	{SPM_PWRAP_CMD23, 0, 0},
	{SPM_PWRAP_CMD24, 0, 0},
	{SPM_PWRAP_CMD25, 0, 0},
	{SPM_PWRAP_CMD26, 0, 0},
	{SPM_PWRAP_CMD27, 0, 0},
	{SPM_PWRAP_CMD28, 0, 0},
	{SPM_PWRAP_CMD29, 0, 0},
	{SPM_PWRAP_CMD30, 0, 0},
	{SPM_PWRAP_CMD31, 0, 0},
};

static struct pmic_wrap_phase_setting phase_table[NR_PMIC_WRAP_PHASE] = {
	{
		.cmd = cmd_table,
		.nr_idx = NR_IDX_ALL,
	},
};

static struct pmic_wrap_setting pmic_wrap_table = {
	.phase = phase_table,
	.phase_nr_idx = NR_PMIC_WRAP_PHASE,
};

void plat_spm_pmic_wrap_init(void)
{
	mt_spm_pmic_wrap_set_table(&pmic_wrap_table);
	mt_spm_pmic_wrap_set_phase(PMIC_WRAP_PHASE_ALLINONE);
}

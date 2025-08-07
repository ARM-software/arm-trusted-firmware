/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_PMIC_LP_H
#define MT_SPM_PMIC_LP_H

#ifdef MTK_SUSPEND_VOL_BIN_SUPPORT
#include <platform_def.h>
#define SOC_VB_REG (EFUSEC_BASE + 0x580)
#define SUSPEND_VB_USE_SHIFT (0)
#define SUSPEND_VB_USE_MASK (0xf)
#endif
#ifdef MTK_AGING_FLAVOR_LOAD
#define SUSPEND_AGING_VAL_SHIFT (3) /* aging margin + 1 */
#define SUSPEND_AGING_VAL_DEFAULT (20) /* sign off - 5% */
#endif
#ifdef MTK_AGING_HV_FLAVOR_LOAD
#define SUSPEND_AGING_HV_VAL_DEFAULT (28) /* sign off + 5% */
#endif

enum SPM_PWR_TYPE { SPM_LP_ENTER, SPM_LP_RESUME };

enum {
	LP_MT6365 = 0,
	LP_MT6319_S7,
	LP_PMIC_SLAVE_NUM,
};

#ifdef MTK_SPM_PMIC_GS_DUMP
#ifdef MTK_SPM_PMIC_GS_DUMP_SUSPEND
extern struct pmic_gs pmic_gs_suspend[];
#endif
#ifdef MTK_SPM_PMIC_GS_DUMP_SODI3
extern struct pmic_gs pmic_gs_sodi3[];
#endif
#ifdef MTK_SPM_PMIC_GS_DUMP_DPIDLE
extern struct pmic_gs pmic_gs_dpidle[];
#endif
#endif
void set_vcore_lp_enable(bool enable);
bool get_vcore_lp_enable(void);
void set_vcore_lp_volt(unsigned int volt);
unsigned int get_vcore_lp_volt(void);

void set_vsram_lp_enable(bool enable);
bool get_vsram_lp_enable(void);
void set_vsram_lp_volt(uint32_t volt);
uint32_t get_vsram_lp_volt(void);

int do_spm_low_power(enum SPM_PWR_TYPE type, uint32_t cmd);

#endif

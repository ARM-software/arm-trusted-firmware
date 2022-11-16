/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_INTERNAL_H
#define MT_SPM_RC_INTERNAL_H

#ifdef MTK_PLAT_SPM_SRAM_SLP_UNSUPPORT
#define SPM_FLAG_SRAM_SLEEP_CTRL (SPM_FLAG_DISABLE_DRAMC_MCU_SRAM_SLEEP)
#define SPM_SRAM_SLEEP_RC_RES_RESTRICT	(0)
#else
#define SPM_FLAG_SRAM_SLEEP_CTRL	(0)
#define SPM_SRAM_SLEEP_RC_RES_RESTRICT	(0)
#endif

#define SPM_RC_UPDATE_COND_ID_MASK	(0xffff)
#define SPM_RC_UPDATE_COND_RC_ID_MASK	(0xffff)
#define SPM_RC_UPDATE_COND_RC_ID_SHIFT	(16)

#define SPM_RC_UPDATE_COND_RC_ID_GET(val) \
	((val >> SPM_RC_UPDATE_COND_RC_ID_SHIFT) & SPM_RC_UPDATE_COND_RC_ID_MASK)

#define SPM_RC_UPDATE_COND_ID_GET(val) (val & SPM_RC_UPDATE_COND_ID_MASK)

/* cpu buck/ldo constraint function */
bool spm_is_valid_rc_cpu_buck_ldo(unsigned int cpu, int state_id);
int spm_update_rc_cpu_buck_ldo(int state_id, int type, const void *val);
unsigned int spm_allow_rc_cpu_buck_ldo(int state_id);
int spm_run_rc_cpu_buck_ldo(unsigned int cpu, int state_id);
int spm_reset_rc_cpu_buck_ldo(unsigned int cpu, int state_id);
int spm_get_status_rc_cpu_buck_ldo(unsigned int type, void *priv);

/* spm resource dram constraint function */
bool spm_is_valid_rc_dram(unsigned int cpu, int state_id);
int spm_update_rc_dram(int state_id, int type, const void *val);
unsigned int spm_allow_rc_dram(int state_id);
int spm_run_rc_dram(unsigned int cpu, int state_id);
int spm_reset_rc_dram(unsigned int cpu, int state_id);
int spm_get_status_rc_dram(unsigned int type, void *priv);

/* spm resource syspll constraint function */
bool spm_is_valid_rc_syspll(unsigned int cpu, int state_id);
int spm_update_rc_syspll(int state_id, int type, const void *val);
unsigned int spm_allow_rc_syspll(int state_id);
int spm_run_rc_syspll(unsigned int cpu, int state_id);
int spm_reset_rc_syspll(unsigned int cpu, int state_id);
int spm_get_status_rc_syspll(unsigned int type, void *priv);

/* spm resource bus26m constraint function */
bool spm_is_valid_rc_bus26m(unsigned int cpu, int state_id);
int spm_update_rc_bus26m(int state_id, int type, const void *val);
unsigned int spm_allow_rc_bus26m(int state_id);
int spm_run_rc_bus26m(unsigned int cpu, int state_id);
int spm_reset_rc_bus26m(unsigned int cpu, int state_id);
int spm_get_status_rc_bus26m(unsigned int type, void *priv);

#endif

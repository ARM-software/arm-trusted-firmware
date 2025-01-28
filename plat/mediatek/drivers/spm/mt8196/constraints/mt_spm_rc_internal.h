/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_RC_INTERNAL_H
#define MT_SPM_RC_INTERNAL_H

#include <sleep_def.h>

#define SPM_SRAM_SLEEP_DEFAULT_FLAG	(SPM_FLAG_DISABLE_SSPM_SRAM_SLEEP)

#ifdef MTK_PLAT_SPM_SRAM_SLP_UNSUPPORT
#define SPM_FLAG_SRAM_SLEEP_CTRL	(SPM_FLAG_DISABLE_SSPM_SRAM_SLEEP)
#else
#define SPM_FLAG_SRAM_SLEEP_CTRL	(SPM_SRAM_SLEEP_DEFAULT_FLAG)
#endif
#define SPM_SRAM_SLEEP_RC_RES_RESTRICT	0

#define SPM_RC_UPDATE_COND_ID_MASK	0xffff
#define SPM_RC_UPDATE_COND_RC_ID_MASK	0xffff
#define SPM_RC_UPDATE_COND_RC_ID_SHIFT	16

#define SPM_RC_UPDATE_COND_RC_ID_GET(val) \
		(((val) >> SPM_RC_UPDATE_COND_RC_ID_SHIFT) & \
		  SPM_RC_UPDATE_COND_RC_ID_MASK)

#define SPM_RC_UPDATE_COND_ID_GET(val) \
		((val) & SPM_RC_UPDATE_COND_ID_MASK)

/* CPU buck/ldo constraint function */
int spm_is_valid_rc_cpu_buck_ldo(int cpu, int state_id);
int spm_update_rc_cpu_buck_ldo(int state_id, int type,
			       const void *val);
uint32_t spm_allow_rc_cpu_buck_ldo(int state_id);
int spm_run_rc_cpu_buck_ldo(int cpu, int state_id);
int spm_reset_rc_cpu_buck_ldo(int cpu, int state_id);
int spm_get_status_rc_cpu_buck_ldo(uint32_t type, void *priv);

/* SPM resource syspll constraint function */
bool spm_is_valid_rc_syspll(uint32_t cpu, int state_id);
int spm_update_rc_syspll(int state_id, int type, const void *val);
uint32_t spm_allow_rc_syspll(int state_id);
int spm_run_rc_syspll(uint32_t cpu, int state_id);
int spm_reset_rc_syspll(uint32_t cpu, int state_id);
int spm_get_status_rc_syspll(uint32_t type, void *priv);

/* SPM resource bus26m constraint function */
bool spm_is_valid_rc_bus26m(uint32_t cpu, int state_id);
int spm_update_rc_bus26m(int state_id, int type, const void *val);
uint32_t spm_allow_rc_bus26m(int state_id);
int spm_run_rc_bus26m(uint32_t cpu, int state_id);
int spm_reset_rc_bus26m(uint32_t cpu, int state_id);
int spm_get_status_rc_bus26m(uint32_t type, void *priv);

/* SPM resource vcore constraint function */
bool spm_is_valid_rc_vcore(uint32_t cpu, int state_id);
int spm_update_rc_vcore(int state_id, int type, const void *val);
uint32_t spm_allow_rc_vcore(int state_id);
int spm_run_rc_vcore(uint32_t cpu, int state_id);
int spm_reset_rc_vcore(uint32_t cpu, int state_id);
int spm_get_status_rc_vcore(uint32_t type, void *priv);

#endif /* MT_SPM_RC_INTERNAL_H */

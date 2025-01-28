/*
 * Copyright (c) 2025, Mediatek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MT_SPM_VCOREFS_EXP_H
#define MT_SPM_VCOREFS_EXP_H

int spm_vcorefs_plat_init(uint32_t dvfsrc_flag,
uint32_t dvfsrc_vmode, uint32_t *dram_type);
int spm_vcorefs_plat_kick(void);
int spm_vcorefs_get_opp_type(uint32_t *val);
int spm_vcorefs_get_fw_type(uint32_t *val);
int spm_vcorefs_get_dram_freq(uint32_t gear, uint32_t *freq);
int spm_vcorefs_get_vcore_opp_num(uint32_t *val);
int spm_vcorefs_get_dram_opp_num(uint32_t *val);
int spm_vcorefs_get_vcore_info(uint32_t idx, uint32_t *val);
int spm_vcorefs_qos_mode(uint32_t mode);
int spm_vcorefs_pause_enable(uint32_t enable);
void spm_vcorefs_plat_suspend(void);
void spm_vcorefs_plat_resume(void);
void dvfsrc_md_ddr_turbo(int is_turbo);

#endif /* MT_SPM_VCOREFS_EXP_H */

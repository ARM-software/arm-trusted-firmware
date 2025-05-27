/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_BL31_INTERFACE_H__
#define __MTK_BL31_INTERFACE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

enum mtk_bl31_status {
	MTK_BL31_STATUS_SUCCESS = 0,
	MTK_BL31_STATUS_INVALID_PARAM = -1,
	MTK_BL31_STATUS_NOT_SUPPORTED = -2,
	MTK_BL31_STATUS_INVALID_RANGE = -3,
	MTK_BL31_STATUS_PERMISSION_DENY = -4,
	MTK_BL31_STATUS_LOCK_FAIL = -5,
};

int mtk_bl31_map_to_sip_error(enum mtk_bl31_status status);

enum mtk_bl31_memory_type {
	MTK_BL31_DEV_RW_SEC = 0,
};

int mtk_bl31_mmap_add_dynamic_region(unsigned long long base_pa, size_t size,
				     enum mtk_bl31_memory_type attr);
int mtk_bl31_mmap_remove_dynamic_region(uintptr_t base_va, size_t size);

/* UFS definitions */
enum ufs_mtk_mphy_op {
	UFS_MPHY_BACKUP = 0,
	UFS_MPHY_RESTORE,
};

enum ufs_notify_change_status {
	PRE_CHANGE,
	POST_CHANGE,
};

/* UFS interfaces */
void ufs_mphy_va09_cg_ctrl(bool enable);
void ufs_device_reset_ctrl(bool rst_n);
void ufs_crypto_hie_init(void);
void ufs_ref_clk_status(uint32_t on, enum ufs_notify_change_status stage);
void ufs_sram_pwr_ctrl(bool on);
void ufs_device_pwr_ctrl(bool vcc_on, uint64_t ufs_version);
void ufs_mphy_ctrl(enum ufs_mtk_mphy_op op);
void ufs_mtcmos_ctrl(bool on);

/* UFS functions implemented in the public ATF repo */
int ufs_rsc_ctrl_mem(bool hold);
int ufs_rsc_ctrl_pmic(bool hold);
void ufs_device_pwr_ctrl_soc(bool vcc_on, uint64_t ufs_version);
int ufs_spm_mtcmos_power(bool on);
int ufs_phy_spm_mtcmos_power(bool on);
bool ufs_is_clk_status_off(void);
void ufs_set_clk_status(bool on);

/* EMI interfaces */
uint64_t emi_mpu_read_addr(unsigned int region, unsigned int offset);
uint64_t emi_mpu_read_enable(unsigned int region);
uint64_t emi_mpu_read_aid(unsigned int region, unsigned int aid_shift);
uint64_t emi_mpu_check_ns_cpu(void);
enum mtk_bl31_status emi_mpu_set_protection(uint32_t start, uint32_t end,
					    unsigned int region);
enum mtk_bl31_status emi_kp_set_protection(size_t start, size_t end, unsigned int region);
enum mtk_bl31_status emi_kp_clear_violation(unsigned int emiid);
enum mtk_bl31_status emi_clear_protection(unsigned int region);
enum mtk_bl31_status emi_clear_md_violation(void);
uint64_t emi_mpu_check_region_info(unsigned int region, uint64_t *sa, uint64_t *ea);
uint64_t emi_mpu_page_base_region(void);
uint64_t emi_mpu_smc_hp_mod_check(void);
enum mtk_bl31_status slb_clear_violation(unsigned int id);
enum mtk_bl31_status emi_clear_violation(unsigned int id, unsigned int type);
enum mtk_bl31_status slc_parity_select(unsigned int id, unsigned int port);
enum mtk_bl31_status slc_parity_clear(unsigned int id);
enum mtk_bl31_status emi_mpu_set_aid(unsigned int region, unsigned int num);
void emi_protection_init(void);

/* CPU QoS interfaces */
void cpu_qos_change_dcc(uint32_t on, uint32_t is_auto);
void *cpu_qos_handle_cluster_on_event(const void *arg);

/* SMMU sid interfaces */
int smmu_sid_init(void);

#endif /* __MTK_BL31_INTERFACE_H__ */

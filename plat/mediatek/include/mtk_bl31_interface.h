/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef __MTK_BL31_INTERFACE_H__
#define __MTK_BL31_INTERFACE_H__

#include <stdbool.h>
#include <stdint.h>

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

#endif /* __MTK_BL31_INTERFACE_H__ */

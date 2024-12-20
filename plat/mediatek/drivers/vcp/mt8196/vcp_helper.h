/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VCP_HELPER_H
#define VCP_HELPER_H

#define MTK_VCP_SRAM_SIZE		(0x60000)

/* Export extern API */
uint32_t get_mmup_fw_size(void);
uint64_t get_mmup_l2tcm_offset(void);

/* SMC calls OPS */
enum mtk_tinysys_vcp_kernel_op {
	MTK_TINYSYS_VCP_KERNEL_OP_RESET_SET = 0,
	MTK_TINYSYS_VCP_KERNEL_OP_RESET_RELEASE,
	MTK_TINYSYS_VCP_KERNEL_OP_COLD_BOOT_VCP,
	MTK_TINYSYS_MMUP_KERNEL_OP_RESET_SET,
	MTK_TINYSYS_MMUP_KERNEL_OP_RESET_RELEASE,
	MTK_TINYSYS_MMUP_KERNEL_OP_SET_L2TCM_OFFSET,
	MTK_TINYSYS_MMUP_KERNEL_OP_SET_FW_SIZE,
	MTK_TINYSYS_MMUP_KERNEL_OP_COLD_BOOT_MMUP,
	MTK_TINYSYS_VCP_KERNEL_OP_NUM,
};

#endif /* VCP_HELPER_H */

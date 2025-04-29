/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

/* MTK header */
#include <drivers/pmic/pmic_swap_api.h>
#include <mtk_bl31_interface.h>
#include <mtk_sip_svc.h>

/* UFS generic control flags */
#define UFS_MTK_SIP_VA09_PWR_CTRL		BIT(0)
#define UFS_MTK_SIP_DEVICE_RESET		BIT(1)
#define UFS_MTK_SIP_CRYPTO_CTRL			BIT(2)
#define UFS_MTK_SIP_REF_CLK_NOTIFICATION	BIT(3)
#define UFS_MTK_SIP_SRAM_PWR_CTRL		BIT(5)
#define UFS_MTK_SIP_GET_VCC_INFO		BIT(6)
#define UFS_MTK_SIP_DEVICE_PWR_CTRL		BIT(7)
#define UFS_MTK_SIP_MPHY_CTRL			BIT(8)
#define UFS_MTK_SIP_MTCMOS_CTRL			BIT(9)

enum {
	VCC_NONE = 0,
	VCC_1,
	VCC_2,
};

static void ufs_get_vcc_info(struct smccc_res *smccc_ret)
{
	if (smccc_ret == NULL)
		return;

	if (is_second_pmic_pp_swap())
		smccc_ret->a1 = VCC_2;
	else
		smccc_ret->a1 = VCC_1;
}

static u_register_t ufs_knl_ctrl(u_register_t x1,
				 u_register_t x2,
				 u_register_t x3,
				 u_register_t x4,
				 void *handle,
				 struct smccc_res *smccc_ret)
{
	uint64_t ret = 0;

	switch (x1) {
	case UFS_MTK_SIP_VA09_PWR_CTRL:
		ufs_mphy_va09_cg_ctrl((bool)!!x2);
		break;
	case UFS_MTK_SIP_DEVICE_RESET:
		ufs_device_reset_ctrl((bool)!!x2);
		break;
	case UFS_MTK_SIP_CRYPTO_CTRL:
		ufs_crypto_hie_init();
		break;
	case UFS_MTK_SIP_REF_CLK_NOTIFICATION:
		ufs_ref_clk_status(x2, x3);
		break;
	case UFS_MTK_SIP_SRAM_PWR_CTRL:
		ufs_sram_pwr_ctrl(x2);
		break;
	case UFS_MTK_SIP_GET_VCC_INFO:
		ufs_get_vcc_info(smccc_ret);
		break;
	case UFS_MTK_SIP_DEVICE_PWR_CTRL:
		ufs_device_pwr_ctrl(x2, x3);
		break;
	case UFS_MTK_SIP_MPHY_CTRL:
		ufs_mphy_ctrl(x2);
		break;
	case UFS_MTK_SIP_MTCMOS_CTRL:
		ufs_mtcmos_ctrl(x2);
		break;
	default:
		ret = -1;
		WARN("[UFS] invalid argument 0x%lx from kernel\n", x1);
		break;
	}

	return ret;
}

static u_register_t ufs_bl_ctrl(u_register_t x1,
				u_register_t x2,
				u_register_t x3,
				u_register_t x4,
				void *handle,
				struct smccc_res *smccc_ret)
{
	uint64_t ret = 0;

	switch (x1) {
	case UFS_MTK_SIP_DEVICE_RESET:
		ufs_device_reset_ctrl(x2);
		break;
	default:
		ret = -1;
		WARN("[UFS] invalid argument 0x%lx from bootloader\n", x1);
		break;
	}

	return ret;
}

DECLARE_SMC_HANDLER(MTK_SIP_KERNEL_UFS_CONTROL, ufs_knl_ctrl);
DECLARE_SMC_HANDLER(MTK_SIP_BL_UFS_CONTROL, ufs_bl_ctrl);

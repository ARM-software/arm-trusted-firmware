/*
 * Copyright (c) 2025, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <common/debug.h>

/* MTK header */
#include <drivers/pmic/pmic_swap_api.h>
#include <lpm_v2/mt_lp_api.h>
#include <lpm_v2/mt_lp_rq.h>
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

/* SPM resource control */
#define RSC_MEM			(MT_LP_RQ_DRAM | MT_LP_RQ_EMI)
#define RSC_PMIC		MT_LP_RQ_PMIC

static int ufs_rsc_ctrl(unsigned int rsc, bool hold)
{
	static struct mt_lp_resource_user ufs_res_user;
	int ret = -1;

	if (!ufs_res_user.uid) {
		ret = mt_lp_resource_user_register("UFS", &ufs_res_user);

		if (ret) {
			WARN("%s: register lp resource failed\n", __func__);
			return ret;
		}
	}

	if (hold)
		ret = ufs_res_user.request(&ufs_res_user, rsc);
	else
		ret = ufs_res_user.release(&ufs_res_user);

	VERBOSE("%s: rsc=%d, hold=%d\n", __func__, rsc, hold);

	if (ret)
		WARN("%s: RSC_%d %s failed\n", __func__, rsc, hold ? "request" : "release");

	return ret;
}

int ufs_rsc_ctrl_mem(bool hold)
{
	return ufs_rsc_ctrl(RSC_MEM, hold);
}

int ufs_rsc_ctrl_pmic(bool hold)
{
	return ufs_rsc_ctrl(RSC_PMIC, hold);
}

/* UFS clock status */
static uint32_t ufs_clk_sta = UFS_REF_CLK_ON;

bool ufs_is_clk_status_off(void)
{
	return ufs_clk_sta == UFS_REF_CLK_OFF;
}

void ufs_set_clk_status(bool on)
{
	if (on)
		ufs_clk_sta = UFS_REF_CLK_ON;
	else
		ufs_clk_sta = UFS_REF_CLK_OFF;
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
		ufs_mphy_va09_cg_ctrl(!!x2);
		break;
	case UFS_MTK_SIP_DEVICE_RESET:
		ufs_device_reset_ctrl(!!x2);
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
#if defined(CONFIG_MTK_MTCMOS)
		ufs_mtcmos_ctrl(!!x2);
#endif
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

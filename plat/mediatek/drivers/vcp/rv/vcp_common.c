/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <errno.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>

#include <arch_helpers.h>
#include <common/debug.h>
#include <lib/mmio.h>
#include <lib/xlat_tables/xlat_tables_v2.h>
#include <smccc_helpers.h>

#include "mmup_common.h"
#include <mtk_mmap_pool.h>
#include <mtk_sip_svc.h>
#include "vcp_helper.h"
#include "vcp_reg.h"

#define MODULE_TAG "[VCP]"

static const mmap_region_t vcp_mmap[] MTK_MMAP_SECTION = {
	MAP_REGION_FLAT(VCP_R_CFGREG, MTK_VCP_REG_BANK_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(VCP_R_CFGREG_CORE0, MTK_VCP_REG_BANK_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(VCP_R_CFGREG_CORE1, MTK_VCP_REG_BANK_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	MAP_REGION_FLAT(VCP_R_SEC_CTRL, MTK_VCP_REG_BANK_SIZE,
		MT_DEVICE | MT_RW | MT_SECURE),
	{0}
};
DECLARE_MTK_MMAP_REGIONS(vcp_mmap);

/* vcp-mmup l2tcm memory offset */
static uint64_t g_l2tcm_offset;
static uint32_t g_mmup_fw_size;

static bool get_vcp_pwr_status(void)
{
#if defined(SPM_BASE)
	uint32_t spm_pwr_sta = mmio_read_32(SPM_BASE + VCP_POWER_STATUS);

	if (!(spm_pwr_sta & (MMUP_PWR_STA_EN << MMUP_PWR_STA_BIT))) {
		ERROR("%s: pwr_sta:%x, bit:%d disable\n", MODULE_TAG,
		      spm_pwr_sta, MMUP_PWR_STA_BIT);
		return false;
	}
#endif
	return true;
}

uint32_t get_mmup_fw_size(void)
{
	return g_mmup_fw_size;
}

uint64_t get_mmup_l2tcm_offset(void)
{
	return g_l2tcm_offset;
}

static bool vcp_cold_boot_reset(void)
{
	mmio_write_32(VCP_GPR2_CFGREG_SEC, 0);
	mmio_write_32(VCP_GPR3_CFGREG_SEC, 0);

	return true;
}

static bool mmup_cold_boot_reset(void)
{
	mmio_write_32(VCP_GPR0_CFGREG_SEC, 0);
	mmio_write_32(VCP_GPR1_CFGREG_SEC, 0);

	return true;
}

static bool vcp_set_mmup_l2tcm_offset(uint64_t l2tcm_offset)
{
	g_l2tcm_offset = l2tcm_offset;

	if (g_l2tcm_offset > MTK_VCP_SRAM_SIZE) {
		g_l2tcm_offset = 0;
		return false;
	}

	return true;
}

static bool vcp_set_mmup_fw_size(uint64_t fw_size)
{
	g_mmup_fw_size = fw_size;

	if (g_mmup_fw_size > MTK_VCP_SRAM_SIZE - g_l2tcm_offset) {
		g_mmup_fw_size = 0;
		return false;
	}

	return true;
}

static bool vcp_smc_rstn_set(bool boot_ok)
{
	if (mmio_read_32(VCP_GPR_C0_H0_REBOOT) != 0 &&
	    mmio_read_32(VCP_R_CORE0_STATUS) != 0 &&
	    (mmio_read_32(VCP_R_GIPC_IN_SET) & B_GIPC3_SETCLR_1) == 0 &&
	    (mmio_read_32(VCP_R_GIPC_IN_CLR) & B_GIPC3_SETCLR_1) == 0 &&
	    mmio_read_32(VCP_GPR_C0_H0_REBOOT) != VCP_CORE_RDY_TO_REBOOT &&
	    mmio_read_32(VCP_GPR_C0_H1_REBOOT) != VCP_CORE_RDY_TO_REBOOT) {
		ERROR("%s: [%s] mmup reset set fail!GIPC 0x%x 0x%x REBOOT 0x%x 0x%x\n",
		      MODULE_TAG, __func__, mmio_read_32(VCP_R_GIPC_IN_SET),
		      mmio_read_32(VCP_R_GIPC_IN_CLR),
		      mmio_read_32(VCP_GPR_C0_H0_REBOOT),
		      mmio_read_32(VCP_GPR_C0_H1_REBOOT));
		return false;
	}

	mmio_write_32(VCP_R_CORE0_SW_RSTN_SET, BIT(0));

	/* reset sec control */
	mmio_write_32(VCP_R_SEC_CTRL_2, 0);

	/* reset domain setting */
	mmio_write_32(VCP_R_S_DOM_EN0_31, 0x0);
	mmio_write_32(VCP_R_S_DOM_EN32_63, 0x0);
	mmio_write_32(VCP_R_NS_DOM_EN0_31, 0x0);
	mmio_write_32(VCP_R_NS_DOM_EN32_63, 0x0);

	/* reset sec setting */
	mmio_clrbits_32(VCP_R_DYN_SECURE,
			RESET_NS_SECURE_B_REGION << VCP_NS_SECURE_B_REGION_ENABLE);

	if (boot_ok) {
		mmio_write_32(VCP_GPR_C0_H0_REBOOT, VCP_CORE_REBOOT_OK);
		mmio_write_32(VCP_GPR_C0_H1_REBOOT, VCP_CORE_REBOOT_OK);
	}

	dsbsy();
	return true;
}

static bool vcp_smc_rstn_clr(void)
{
	if ((mmio_read_32(VCP_R_CORE0_SW_RSTN_SET) & BIT(0)) == 1) {
		ERROR("%s: [%s] mmup not reset set !\n", MODULE_TAG, __func__);
		return false;
	}

	mmio_clrsetbits_32(VCP_R_SEC_DOMAIN,
			   ~(VCP_DOMAIN_MASK << VCP_HWCCF_DOMAIN), VCP_DOMAIN_SET);

	/* enable IOVA Mode */
	mmio_write_32(VCP_R_AXIOMMUEN_DEV_APC, BIT(0));

	/* reset secure setting */
	mmio_setbits_32(VCP_R_SEC_CTRL_2, CORE0_SEC_BIT_SEL);
	mmio_clrbits_32(VCP_R_DYN_SECURE, VCP_NS_I0 | VCP_NS_D0);
	mmio_clrbits_32(VCP_R_DYN_SECURE_TH1, VCP_NS_I1 | VCP_NS_D1);

	/* start vcp */
	mmio_write_32(VCP_R_CORE0_SW_RSTN_CLR, BIT(0));
	dsbsy();
	return true;
}

static u_register_t tinysys_vcp_kernel_control(u_register_t arg0,
					       u_register_t arg1,
					       u_register_t arg2,
					       u_register_t arg3,
					       void *handle,
					       struct smccc_res *smccc_ret)
{
	uint32_t request_ops;
	uint64_t ret = MTK_SIP_E_SUCCESS;

	if (!get_vcp_pwr_status())
		return MTK_SIP_E_NOT_SUPPORTED;

	request_ops = (uint32_t)arg0;

	switch (request_ops) {
	case MTK_TINYSYS_VCP_KERNEL_OP_RESET_SET:
		ret = vcp_smc_rstn_set((bool)!!arg1);
		break;
	case MTK_TINYSYS_VCP_KERNEL_OP_RESET_RELEASE:
		ret = vcp_smc_rstn_clr();
		break;
	case MTK_TINYSYS_VCP_KERNEL_OP_COLD_BOOT_VCP:
		ret = vcp_cold_boot_reset();
		break;
	case MTK_TINYSYS_MMUP_KERNEL_OP_RESET_SET:
		ret = mmup_smc_rstn_set((bool)!!arg1);
		break;
	case MTK_TINYSYS_MMUP_KERNEL_OP_RESET_RELEASE:
		ret = mmup_smc_rstn_clr();
		break;
	case MTK_TINYSYS_MMUP_KERNEL_OP_SET_L2TCM_OFFSET:
		ret = vcp_set_mmup_l2tcm_offset(arg1);
		break;
	case MTK_TINYSYS_MMUP_KERNEL_OP_SET_FW_SIZE:
		ret = vcp_set_mmup_fw_size(arg1);
		break;
	case MTK_TINYSYS_MMUP_KERNEL_OP_COLD_BOOT_MMUP:
		ret = mmup_cold_boot_reset();
		break;
	default:
		ERROR("%s: %s, unknown request_ops = %x\n", MODULE_TAG, __func__, request_ops);
		ret = MTK_SIP_E_INVALID_PARAM;
		break;
	}

	return ret;
}

/* Register SiP SMC service */
DECLARE_SMC_HANDLER(MTK_SIP_KERNEL_VCP_CONTROL, tinysys_vcp_kernel_control);

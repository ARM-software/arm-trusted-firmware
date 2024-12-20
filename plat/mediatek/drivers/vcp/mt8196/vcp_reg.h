/*
 * Copyright (c) 2024, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef VCP_REG_H
#define VCP_REG_H

#include <platform_def.h>

#define MTK_VCP_REG_BASE		(IO_PHYS + 0x21800000)
#define MTK_VCP_REG_BANK_SIZE		(0x1000)

/*******************************************************************************
 * VCP power related setting
 ******************************************************************************/
#define VCP_POWER_STATUS		(0xE60)
#define MMUP_PWR_STA_BIT		(30)
#define MMUP_PWR_STA_EN			((uint32_t)(0x3))

/*******************************************************************************
 * VCP registers
 ******************************************************************************/
/* cfgreg */
#define VCP_R_CFGREG			(MTK_VCP_REG_BASE + 0x3d0000)

#define VCP_R_CORE0_SW_RSTN_CLR		(VCP_R_CFGREG + 0x0000)
#define VCP_R_CORE0_SW_RSTN_SET		(VCP_R_CFGREG + 0x0004)
#define VCP_R_CORE1_SW_RSTN_CLR		(VCP_R_CFGREG + 0x0008)
#define VCP_R_CORE1_SW_RSTN_SET		(VCP_R_CFGREG + 0x000c)
#define VCP_R_GIPC_IN_SET		(VCP_R_CFGREG + 0x0028)
#define VCP_R_GIPC_IN_CLR		(VCP_R_CFGREG + 0x002c)
#define B_GIPC3_SETCLR_1		BIT(13)

/* cfgreg_core0 */
#define VCP_R_CFGREG_CORE0		(MTK_VCP_REG_BASE + 0x20a000)

#define VCP_R_CORE0_STATUS		(VCP_R_CFGREG_CORE0 + 0x0070)

#define CORE0_R_GPR5			(VCP_R_CFGREG_CORE0 + 0x0054)
#define VCP_GPR_C0_H0_REBOOT		CORE0_R_GPR5
#define CORE0_R_GPR6			(VCP_R_CFGREG_CORE0 + 0x0058)
#define VCP_GPR_C0_H1_REBOOT		CORE0_R_GPR6
#define VCP_CORE_RDY_TO_REBOOT		(0x34)
#define VCP_CORE_REBOOT_OK		BIT(0)

/* cfgreg_core1 */
#define VCP_R_CFGREG_CORE1		(MTK_VCP_REG_BASE + 0x20d000)

#define VCP_R_CORE1_STATUS		(VCP_R_CFGREG_CORE1 + 0x0070)
#define CORE1_R_GPR5			(VCP_R_CFGREG_CORE1 + 0x0054)
#define VCP_GPR_CORE1_REBOOT		CORE1_R_GPR5

/* sec */
#define VCP_R_SEC_CTRL			(MTK_VCP_REG_BASE + 0x270000)
#define VCP_OFFSET_ENABLE_P		BIT(13)
#define VCP_OFFSET_ENABLE_B		BIT(12)
#define VCP_R_SEC_CTRL_2		(VCP_R_SEC_CTRL + 0x0004)
#define CORE0_SEC_BIT_SEL		BIT(0)
#define CORE1_SEC_BIT_SEL		BIT(8)
#define VCP_GPR0_CFGREG_SEC		(VCP_R_SEC_CTRL + 0x0040)
#define VCP_GPR1_CFGREG_SEC		(VCP_R_SEC_CTRL + 0x0044)
#define VCP_GPR2_CFGREG_SEC		(VCP_R_SEC_CTRL + 0x0048)
#define VCP_GPR3_CFGREG_SEC		(VCP_R_SEC_CTRL + 0x004C)
#define VCP_R_SEC_DOMAIN		(VCP_R_SEC_CTRL + 0x0080)
#define VCP_DOMAIN_ID			U(13)
#define VCP_DOMAIN_MASK			U(0xF)
#define VCP_CORE0_TH0_PM_AXI_DOMAIN	(0)
#define VCP_CORE0_TH0_DM_AXI_DOMAIN	(4)
#define VCP_S_DMA0_DOMAIN		(12)
#define VCP_HWCCF_DOMAIN		(16)
#define VCP_CORE0_TH1_PM_AXI_DOMAIN	(20)
#define VCP_CORE0_TH1_DM_AXI_DOMAIN	(24)
#define VCP_DOMAIN_SET			((VCP_DOMAIN_ID << VCP_CORE0_TH0_PM_AXI_DOMAIN) | \
					 (VCP_DOMAIN_ID << VCP_CORE0_TH0_DM_AXI_DOMAIN) | \
					 (VCP_DOMAIN_ID << VCP_CORE0_TH1_PM_AXI_DOMAIN) | \
					 (VCP_DOMAIN_ID << VCP_CORE0_TH1_DM_AXI_DOMAIN) | \
					 (VCP_DOMAIN_ID << VCP_S_DMA0_DOMAIN))
#define VCP_R_SEC_DOMAIN_MMPC		(VCP_R_SEC_CTRL + 0x0084)
#define VCP_CORE_MMPC_PM_AXI_DOMAIN	(0)
#define VCP_CORE_MMPC_DM_AXI_DOMAIN	(4)
#define VCP_DOMAIN_SET_MMPC		((VCP_DOMAIN_ID << VCP_CORE_MMPC_PM_AXI_DOMAIN) | \
					(VCP_DOMAIN_ID << VCP_CORE_MMPC_DM_AXI_DOMAIN))
#define R_L2TCM_OFFSET_RANGE_0_LOW	(VCP_R_SEC_CTRL + 0x00B0)
#define R_L2TCM_OFFSET_RANGE_0_HIGH	(VCP_R_SEC_CTRL + 0x00B4)
#define R_L2TCM_OFFSET			(VCP_R_SEC_CTRL + 0x00D0)
#define VCP_R_DYN_SECURE		(VCP_R_SEC_CTRL + 0x01d0)
#define VCP_NS_I0			BIT(4)
#define VCP_NS_D0			BIT(6)
#define VCP_NS_SECURE_B_REGION_ENABLE	(24)
#define RESET_NS_SECURE_B_REGION	U(0xFF)
#define VCP_R_DYN_SECURE_TH1		(VCP_R_SEC_CTRL + 0x01d4)
#define VCP_NS_I1			BIT(5)
#define VCP_NS_D1			BIT(7)
#define VCP_R_S_DOM_EN0_31		(VCP_R_SEC_CTRL + 0x0200)
#define VCP_R_S_DOM_EN32_63		(VCP_R_SEC_CTRL + 0x0204)
#define VCP_R_NS_DOM_EN0_31		(VCP_R_SEC_CTRL + 0x0208)
#define VCP_R_NS_DOM_EN32_63		(VCP_R_SEC_CTRL + 0x020c)
/* IOMMU */
#define VCP_R_AXIOMMUEN_DEV_APC		(VCP_R_SEC_CTRL + 0x0088)
#define VCP_R_CFG_DEVAPC_AO_BASE	(MTK_VCP_REG_BASE + 0x2d0000)

#endif /* VCP_REG_H */

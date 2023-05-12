/*
 * Copyright (c) 2023, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef APUSYS_RV_H
#define APUSYS_RV_H

#include <platform_def.h>

#define APU_SEC_FW_IOVA			(0x200000UL)

/* APU_SCTRL_REVISER */
#define UP_NORMAL_DOMAIN_NS		(APU_REVISER + 0x0000)
#define UP_PRI_DOMAIN_NS		(APU_REVISER + 0x0004)
#define UP_IOMMU_CTRL			(APU_REVISER + 0x0008)
#define UP_CORE0_VABASE0		(APU_REVISER + 0x000c)
#define UP_CORE0_MVABASE0		(APU_REVISER + 0x0010)
#define UP_CORE0_VABASE1		(APU_REVISER + 0x0014)
#define UP_CORE0_MVABASE1		(APU_REVISER + 0x0018)
#define UP_CORE0_VABASE2		(APU_REVISER + 0x001c)
#define UP_CORE0_MVABASE2		(APU_REVISER + 0x0020)
#define UP_CORE0_VABASE3		(APU_REVISER + 0x0024)
#define UP_CORE0_MVABASE3		(APU_REVISER + 0x0028)
#define USERFW_CTXT			(APU_REVISER + 0x1000)
#define SECUREFW_CTXT			(APU_REVISER + 0x1004)
#define UP_NORMAL_DOMAIN		(7)
#define UP_NORMAL_NS			(1)
#define UP_PRI_DOMAIN			(5)
#define UP_PRI_NS			(1)
#define UP_DOMAIN_SHIFT			(0)
#define UP_NS_SHIFT			(4)
#define MMU_EN				BIT(0)
#define MMU_CTRL			BIT(1)
#define MMU_CTRL_LOCK			BIT(2)
#define VLD				BIT(0)
#define PARTIAL_ENABLE			BIT(1)
#define THREAD_NUM_PRI			(1)
#define THREAD_NUM_NORMAL		(0)
#define THREAD_NUM_SHIFT		(2)
#define VASIZE_1MB			BIT(0)
#define CFG_4GB_SEL_EN			BIT(2)
#define CFG_4GB_SEL			(0)
#define MVA_34BIT_SHIFT			(2)

/* APU_MD32_SYSCTRL */
#define MD32_SYS_CTRL			(APU_MD32_SYSCTRL + 0x0000)
#define UP_INT_EN2			(APU_MD32_SYSCTRL + 0x000c)
#define MD32_CLK_CTRL			(APU_MD32_SYSCTRL + 0x00b8)
#define UP_WAKE_HOST_MASK0		(APU_MD32_SYSCTRL + 0x00bc)
#define UP_WAKE_HOST_MASK1		(APU_MD32_SYSCTRL + 0x00c0)
#define MD32_SYS_CTRL_RST		(0)
#define MD32_G2B_CG_EN			BIT(11)
#define MD32_DBG_EN			BIT(10)
#define MD32_DM_AWUSER_IOMMU_EN		BIT(9)
#define MD32_DM_ARUSER_IOMMU_EN		BIT(7)
#define MD32_PM_AWUSER_IOMMU_EN		BIT(5)
#define MD32_PM_ARUSER_IOMMU_EN		BIT(3)
#define MD32_SOFT_RSTN			BIT(0)
#define MD32_CLK_EN			(1)
#define MD32_CLK_DIS			(0)
#define WDT_IRQ_EN			BIT(0)
#define MBOX0_IRQ_EN			BIT(21)
#define MBOX1_IRQ_EN			BIT(22)
#define MBOX2_IRQ_EN			BIT(23)
#define RESET_DEALY_US			(10)
#define DBG_APB_EN			BIT(31)

/* APU_AO_CTRL */
#define MD32_PRE_DEFINE			(APU_AO_CTRL + 0x0000)
#define MD32_BOOT_CTRL			(APU_AO_CTRL + 0x0004)
#define MD32_RUNSTALL			(APU_AO_CTRL + 0x0008)
#define PREDEFINE_NON_CACHE		(0)
#define PREDEFINE_TCM			(1)
#define PREDEFINE_CACHE			(2)
#define PREDEFINE_CACHE_TCM		(3)
#define PREDEF_1G_OFS			(0)
#define PREDEF_2G_OFS			(2)
#define PREDEF_3G_OFS			(4)
#define PREDEF_4G_OFS			(6)
#define MD32_RUN			(0)
#define MD32_STALL			(1)

/* APU_MD32_WDT */
#define WDT_INT				(APU_MD32_WDT + 0x0)
#define WDT_CTRL0			(APU_MD32_WDT + 0x4)
#define WDT_INT_W1C			(1)
#define WDT_EN				BIT(31)

/* APU MBOX */
#define MBOX_FUNC_CFG			(0xb0)
#define MBOX_DOMAIN_CFG			(0xe0)
#define MBOX_CTRL_LOCK			BIT(0)
#define MBOX_NO_MPU_SHIFT		(16)
#define MBOX_RX_NS_SHIFT		(16)
#define MBOX_RX_DOMAIN_SHIFT		(17)
#define MBOX_TX_NS_SHIFT		(24)
#define MBOX_TX_DOMAIN_SHIFT		(25)
#define MBOX_SIZE			(0x100)
#define MBOX_NUM			(8)

#define APU_MBOX(i)		(((i) < MBOX_NUM) ? (APU_MBOX0 + MBOX_SIZE * (i)) : \
						  (APU_MBOX1 + MBOX_SIZE * ((i) - MBOX_NUM)))
#define APU_MBOX_FUNC_CFG(i)	(APU_MBOX(i) + MBOX_FUNC_CFG)
#define APU_MBOX_DOMAIN_CFG(i)	(APU_MBOX(i) + MBOX_DOMAIN_CFG)

void apusys_rv_mbox_mpu_init(void);
int apusys_kernel_apusys_rv_setup_reviser(void);
int apusys_kernel_apusys_rv_reset_mp(void);
int apusys_kernel_apusys_rv_setup_boot(void);
int apusys_kernel_apusys_rv_start_mp(void);
int apusys_kernel_apusys_rv_stop_mp(void);
int apusys_kernel_apusys_rv_setup_sec_mem(void);
int apusys_kernel_apusys_rv_disable_wdt_isr(void);
int apusys_kernel_apusys_rv_clear_wdt_isr(void);
int apusys_kernel_apusys_rv_cg_gating(void);
int apusys_kernel_apusys_rv_cg_ungating(void);

#endif /* APUSYS_RV_H */

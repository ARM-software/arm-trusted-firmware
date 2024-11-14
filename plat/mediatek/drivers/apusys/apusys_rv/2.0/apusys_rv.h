/*
 * Copyright (c) 2023-2024, MediaTek Inc. All rights reserved.
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

enum APU_PWR_OP {
	APU_PWR_OFF = 0,
	APU_PWR_ON  = 1,
};

/* APU_LOGTOP */
#define APU_LOGTOP_CON			(APU_LOGTOP + 0x0)
#define APU_LOG_BUF_T_SIZE		(APU_LOGTOP + 0x78)
#define APU_LOG_BUF_W_PTR		(APU_LOGTOP + 0x80)
#define APU_LOG_BUF_R_PTR		(APU_LOGTOP + 0x84)
#define HW_SEMA2			(APU_ARE_REG_BASE + 0x0E08)
#define HW_SEMA_USER			(0x2)
#define HW_SEMA_LOGGER_USER		(0x3)
#define MAX_SMC_OP_NUM			(0x3)
#define LOGTOP_OP_MASK			(0xFF)
#define LOGTOP_OP_SHIFT			(8)
enum {
	SMC_OP_APU_LOG_BUF_NULL = 0,
	SMC_OP_APU_LOG_BUF_T_SIZE,
	SMC_OP_APU_LOG_BUF_W_PTR,
	SMC_OP_APU_LOG_BUF_R_PTR,
	SMC_OP_APU_LOG_BUF_CON,
	SMC_OP_APU_LOG_BUF_NUM
};

struct smccc_res;

void apusys_rv_mbox_mpu_init(void);
int apusys_infra_dcm_setup(void);
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
int apusys_kernel_apusys_rv_setup_apummu(void);
int apusys_kernel_apusys_rv_pwr_ctrl(enum APU_PWR_OP op);
int apusys_kernel_apusys_logtop_reg_dump(uint32_t op, struct smccc_res *smccc_ret);
int apusys_kernel_apusys_logtop_reg_write(uint32_t op, uint32_t write_val,
					  struct smccc_res *smccc_ret);
int apusys_kernel_apusys_logtop_reg_w1c(uint32_t op, struct smccc_res *smccc_ret);
int apusys_rv_cold_boot_clr_mbox_dummy(void);
int apusys_rv_setup_ce_bin(void);

#endif /* APUSYS_RV_H */

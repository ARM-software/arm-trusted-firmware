/*
 * Copyright (c) 2019, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_PRIVATE_H
#define MTSPMC_PRIVATE_H

/*
 * per_cpu/cluster helper
 */
struct per_cpu_reg {
	int cluster_addr;
	int cpu_stride;
};

#define per_cpu(cluster, cpu, reg)	(reg[cluster].cluster_addr + \
					(cpu << reg[cluster].cpu_stride))
#define per_cluster(cluster, reg)	(reg[cluster].cluster_addr)

/* SPMC related registers */
#define SPM_POWERON_CONFIG_EN		(SPM_BASE + 0x000)
/* bit-fields of SPM_POWERON_CONFIG_EN */
#define BCLK_CG_EN			(1 << 0)
#define MD_BCLK_CG_EN			(1 << 1)
#define PROJECT_CODE			(0xb16 << 16)

#define SPM_PWR_STATUS			(SPM_BASE + 0x180)
#define SPM_PWR_STATUS_2ND		(SPM_BASE + 0x184)

#define SPM_BYPASS_SPMC			(SPM_BASE + 0x2b4)
#define SPM_SPMC_DORMANT_ENABLE		(SPM_BASE + 0x2b8)

#define SPM_MP0_CPUTOP_PWR_CON		(SPM_BASE + 0x204)
#define SPM_MP0_CPU0_PWR_CON		(SPM_BASE + 0x208)
#define SPM_MP0_CPU1_PWR_CON		(SPM_BASE + 0x20C)
#define SPM_MP0_CPU2_PWR_CON		(SPM_BASE + 0x210)
#define SPM_MP0_CPU3_PWR_CON		(SPM_BASE + 0x214)
#define SPM_MP1_CPUTOP_PWR_CON		(SPM_BASE + 0x218)
#define SPM_MP1_CPU0_PWR_CON		(SPM_BASE + 0x21C)
#define SPM_MP1_CPU1_PWR_CON		(SPM_BASE + 0x220)
#define SPM_MP1_CPU2_PWR_CON		(SPM_BASE + 0x224)
#define SPM_MP1_CPU3_PWR_CON		(SPM_BASE + 0x228)
#define SPM_MP0_CPUTOP_L2_PDN		(SPM_BASE + 0x240)
#define SPM_MP0_CPUTOP_L2_SLEEP_B	(SPM_BASE + 0x244)
#define SPM_MP0_CPU0_L1_PDN		(SPM_BASE + 0x248)
#define SPM_MP0_CPU1_L1_PDN		(SPM_BASE + 0x24C)
#define SPM_MP0_CPU2_L1_PDN		(SPM_BASE + 0x250)
#define SPM_MP0_CPU3_L1_PDN		(SPM_BASE + 0x254)
#define SPM_MP1_CPUTOP_L2_PDN		(SPM_BASE + 0x258)
#define SPM_MP1_CPUTOP_L2_SLEEP_B	(SPM_BASE + 0x25C)
#define SPM_MP1_CPU0_L1_PDN		(SPM_BASE + 0x260)
#define SPM_MP1_CPU1_L1_PDN		(SPM_BASE + 0x264)
#define SPM_MP1_CPU2_L1_PDN		(SPM_BASE + 0x268)
#define SPM_MP1_CPU3_L1_PDN		(SPM_BASE + 0x26C)

#define SPM_CPU_EXT_BUCK_ISO		(SPM_BASE + 0x290)
/* bit-fields of SPM_CPU_EXT_BUCK_ISO */
#define MP0_EXT_BUCK_ISO		(1 << 0)
#define MP1_EXT_BUCK_ISO		(1 << 1)
#define MP_EXT_BUCK_ISO			(1 << 2)

/* bit-fields of SPM_PWR_STATUS */
#define PWR_STATUS_MD			(1 << 0)
#define PWR_STATUS_CONN			(1 << 1)
#define PWR_STATUS_DDRPHY		(1 << 2)
#define PWR_STATUS_DISP			(1 << 3)
#define PWR_STATUS_MFG			(1 << 4)
#define PWR_STATUS_ISP			(1 << 5)
#define PWR_STATUS_INFRA		(1 << 6)
#define PWR_STATUS_VDEC			(1 << 7)
#define PWR_STATUS_MP0_CPUTOP		(1 << 8)
#define PWR_STATUS_MP0_CPU0		(1 << 9)
#define PWR_STATUS_MP0_CPU1		(1 << 10)
#define PWR_STATUS_MP0_CPU2		(1 << 11)
#define PWR_STATUS_MP0_CPU3		(1 << 12)
#define PWR_STATUS_MCUSYS		(1 << 14)
#define PWR_STATUS_MP1_CPUTOP		(1 << 15)
#define PWR_STATUS_MP1_CPU0		(1 << 16)
#define PWR_STATUS_MP1_CPU1		(1 << 17)
#define PWR_STATUS_MP1_CPU2		(1 << 18)
#define PWR_STATUS_MP1_CPU3		(1 << 19)
#define PWR_STATUS_VEN			(1 << 21)
#define PWR_STATUS_MFG_ASYNC		(1 << 23)
#define PWR_STATUS_AUDIO		(1 << 24)
#define PWR_STATUS_C2K			(1 << 28)
#define PWR_STATUS_MD_INFRA		(1 << 29)


/* bit-fields of SPM_*_PWR_CON */
#define PWRCTRL_PWR_RST_B		(1 << 0)
#define PWRCTRL_PWR_ISO			(1 << 1)
#define PWRCTRL_PWR_ON			(1 << 2)
#define PWRCTRL_PWR_ON_2ND		(1 << 3)
#define PWRCTRL_PWR_CLK_DIS		(1 << 4)
#define PWRCTRL_PWR_SRAM_CKISO		(1 << 5)
#define PWRCTRL_PWR_SRAM_ISOINT_B	(1 << 6)
#define PWRCTRL_PWR_SRAM_PD_SLPB_CLAMP	(1 << 7)
#define PWRCTRL_PWR_SRAM_PDN		(1 << 8)
#define PWRCTRL_PWR_SRAM_SLEEP_B	(1 << 12)
#define PWRCTRL_PWR_SRAM_PDN_ACK	(1 << 24)
#define PWRCTRL_PWR_SRAM_SLEEP_B_ACK	(1 << 28)

/* per_cpu registers for SPM_MP?_CPU?_PWR_CON */
static const struct per_cpu_reg SPM_CPU_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2 },
	[1] = { .cluster_addr = SPM_MP1_CPU0_PWR_CON, .cpu_stride = 2 },
};

/* per_cluster registers for SPM_MP?_CPUTOP_PWR_CON */
static const struct per_cpu_reg SPM_CLUSTER_PWR[] = {
	[0] = { .cluster_addr = SPM_MP0_CPUTOP_PWR_CON },
	[1] = { .cluster_addr = SPM_MP1_CPUTOP_PWR_CON },
};

/* APB Module infracfg_ao */
#define INFRA_TOPAXI_PROTECTEN_1	(INFRACFG_AO_BASE + 0x250)
#define INFRA_TOPAXI_PROTECTEN_STA1_1	(INFRACFG_AO_BASE + 0x258)
#define INFRA_TOPAXI_PROTECTEN_1_SET	(INFRACFG_AO_BASE + 0x2A8)
#define INFRA_TOPAXI_PROTECTEN_1_CLR	(INFRACFG_AO_BASE + 0x2AC)

/* bit-fields of INFRA_TOPAXI_PROTECTEN_1_SET */
#define MP0_CPUTOP_PROT_STEP1_0_MASK	((1 << 10)|(1 << 12)| \
					 (1 << 13)|(1 << 26))
#define MP1_CPUTOP_PROT_STEP1_0_MASK	((1 << 11)|(1 << 14)| \
					 (1 << 15)|(1 << 27))

/* bit-fields of INFRA_TOPAXI_PROTECTEN_STA1_1 */
#define MP0_CPUTOP_PROT_STEP1_0_ACK_MASK	((1 << 10)|(1 << 12)| \
						(1 << 13)|(1 << 26))
#define MP1_CPUTOP_PROT_STEP1_0_ACK_MASK	((1 << 11)|(1 << 14)| \
						(1 << 15)|(1 << 27))


/*
 * MCU configuration registers
 */

/* bit-fields of MCUCFG_MP?_AXI_CONFIG */
#define MCUCFG_AXI_CONFIG_BROADCASTINNER	(1 << 0)
#define MCUCFG_AXI_CONFIG_BROADCASTOUTER	(1 << 1)
#define MCUCFG_AXI_CONFIG_BROADCASTCACHEMAINT	(1 << 2)
#define MCUCFG_AXI_CONFIG_SYSBARDISABLE		(1 << 3)
#define MCUCFG_AXI_CONFIG_ACINACTM		(1 << 4)
#define MCUCFG_AXI_CONFIG_AINACTS		(1 << 5)


#define MCUCFG_MP0_MISC_CONFIG2 ((uintptr_t)&mt8183_mcucfg->mp0_misc_config[2])
#define MCUCFG_MP0_MISC_CONFIG3 ((uintptr_t)&mt8183_mcucfg->mp0_misc_config[3])
#define MCUCFG_MP1_MISC_CONFIG2 ((uintptr_t)&mt8183_mcucfg->mp1_misc_config[2])
#define MCUCFG_MP1_MISC_CONFIG3 ((uintptr_t)&mt8183_mcucfg->mp1_misc_config[3])

#define MCUCFG_CPUSYS0_SPARKVRETCNTRL	(MCUCFG_BASE + 0x1c00)
/* bit-fields of MCUCFG_CPUSYS0_SPARKVRETCNTRL */
#define CPU0_SPARK_VRET_CTRL		(0x3f << 0)
#define CPU1_SPARK_VRET_CTRL		(0x3f << 8)
#define CPU2_SPARK_VRET_CTRL		(0x3f << 16)
#define CPU3_SPARK_VRET_CTRL		(0x3f << 24)

/* SPARK control in little cores */
#define MCUCFG_CPUSYS0_CPU0_SPMC_CTL	(MCUCFG_BASE + 0x1c30)
#define MCUCFG_CPUSYS0_CPU1_SPMC_CTL	(MCUCFG_BASE + 0x1c34)
#define MCUCFG_CPUSYS0_CPU2_SPMC_CTL	(MCUCFG_BASE + 0x1c38)
#define MCUCFG_CPUSYS0_CPU3_SPMC_CTL	(MCUCFG_BASE + 0x1c3c)
/* bit-fields of MCUCFG_CPUSYS0_CPU?_SPMC_CTL */
#define SW_SPARK_EN			(1 << 0)
#define SW_NO_WAIT_Q			(1 << 1)

/* the MCUCFG which BIG cores used is at (MCUCFG_BASE + 0x2000) */
#define MCUCFG_MP2_BASE			(MCUCFG_BASE + 0x2000)
#define MCUCFG_MP2_PWR_RST_CTL		(MCUCFG_MP2_BASE + 0x8)
/* bit-fields of MCUCFG_MP2_PWR_RST_CTL */
#define SW_RST_B			(1 << 0)
#define TOPAON_APB_MASK			(1 << 1)

#define MCUCFG_MP2_CPUCFG		(MCUCFG_MP2_BASE + 0x208)

#define MCUCFG_MP2_RVADDR0		(MCUCFG_MP2_BASE + 0x290)
#define MCUCFG_MP2_RVADDR1		(MCUCFG_MP2_BASE + 0x298)
#define MCUCFG_MP2_RVADDR2		(MCUCFG_MP2_BASE + 0x2c0)
#define MCUCFG_MP2_RVADDR3		(MCUCFG_MP2_BASE + 0x2c8)

/* SPMC control */
#define MCUCFG_MP0_SPMC (MCUCFG_BASE + 0x788)
#define MCUCFG_MP2_SPMC (MCUCFG_MP2_BASE + 0x2a0)
#define MCUCFG_MP2_COQ  (MCUCFG_MP2_BASE + 0x2bC)

/* per_cpu registers for MCUCFG_MP?_MISC_CONFIG2 */
static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_MISC_CONFIG2, .cpu_stride = 3 },
};

/* per_cpu registers for MCUCFG_MP?_MISC_CONFIG3 */
static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	[0] = { .cluster_addr = MCUCFG_MP0_MISC_CONFIG3 },
	[1] = { .cluster_addr = MCUCFG_MP2_CPUCFG },
};

/* SPARK control in BIG cores */
#define MCUCFG_MP2_PTP3_CPU0_SPMC0	(MCUCFG_MP2_BASE + 0x430)
#define MCUCFG_MP2_PTP3_CPU0_SPMC1	(MCUCFG_MP2_BASE + 0x434)
#define MCUCFG_MP2_PTP3_CPU1_SPMC0	(MCUCFG_MP2_BASE + 0x438)
#define MCUCFG_MP2_PTP3_CPU1_SPMC1	(MCUCFG_MP2_BASE + 0x43c)
#define MCUCFG_MP2_PTP3_CPU2_SPMC0	(MCUCFG_MP2_BASE + 0x440)
#define MCUCFG_MP2_PTP3_CPU2_SPMC1	(MCUCFG_MP2_BASE + 0x444)
#define MCUCFG_MP2_PTP3_CPU3_SPMC0	(MCUCFG_MP2_BASE + 0x448)
#define MCUCFG_MP2_PTP3_CPU3_SPMC1	(MCUCFG_MP2_BASE + 0x44c)
/* bit-fields of MCUCFG_MP2_PTP3_CPU?_SPMC? */
#define SW_SPARK_EN			(1 << 0)
#define SW_NO_WAIT_Q			(1 << 1)

#define MCUCFG_MP2_SPARK2LDO		(MCUCFG_MP2_BASE + 0x700)
/* bit-fields of MCUCFG_MP2_SPARK2LDO */
#define SPARK_VRET_CTRL			(0x3f << 0)
#define CPU0_SPARK_LDO_AMUXSEL		(0xf  << 6)
#define CPU1_SPARK_LDO_AMUXSEL		(0xf  << 10)
#define CPU2_SPARK_LDO_AMUXSEL		(0xf  << 14)
#define CPU3_SPARK_LDO_AMUXSEL		(0xf  << 18)

/* per_cpu registers for SPARK */
static const struct per_cpu_reg MCUCFG_SPARK[] = {
	[0] = { .cluster_addr = MCUCFG_CPUSYS0_CPU0_SPMC_CTL, .cpu_stride = 2 },
	[1] = { .cluster_addr = MCUCFG_MP2_PTP3_CPU0_SPMC0, .cpu_stride = 3 },
};

/* per_cpu registers for SPARK2LDO */
static const struct per_cpu_reg MCUCFG_SPARK2LDO[] = {
	[0] = { .cluster_addr = MCUCFG_CPUSYS0_SPARKVRETCNTRL },
	[1] = { .cluster_addr = MCUCFG_MP2_SPARK2LDO },
};

#endif /* MTSPMC_PRIVATE_H */

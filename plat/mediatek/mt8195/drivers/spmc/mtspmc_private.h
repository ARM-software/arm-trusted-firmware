/*
 * Copyright (c) 2020, MediaTek Inc. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#ifndef MTSPMC_PRIVATE_H
#define MTSPMC_PRIVATE_H

#include <lib/utils_def.h>
#include <platform_def.h>

unsigned long read_cpuectlr(void);
void write_cpuectlr(unsigned long cpuectlr);

unsigned long read_cpupwrctlr_el1(void);
void write_cpupwrctlr_el1(unsigned long cpuectlr);

/*
 * per_cpu/cluster helper
 */
struct per_cpu_reg {
	unsigned int cluster_addr;
	unsigned int cpu_stride;
};

#define per_cpu(cluster, cpu, reg)	\
	(reg[cluster].cluster_addr + (cpu << reg[cluster].cpu_stride))

#define per_cluster(cluster, reg)	(reg[cluster].cluster_addr)

#define SPM_REG(ofs)			(uint32_t)(SPM_BASE + (ofs))
#define MCUCFG_REG(ofs)			(uint32_t)(MCUCFG_BASE + (ofs))
#define INFRACFG_AO_REG(ofs)		(uint32_t)(INFRACFG_AO_BASE + (ofs))

/* === SPMC related registers */
#define SPM_POWERON_CONFIG_EN		SPM_REG(0x000)
/* bit-fields of SPM_POWERON_CONFIG_EN */
#define PROJECT_CODE			(U(0xb16) << 16)
#define BCLK_CG_EN			BIT(0)

#define SPM_PWR_STATUS			SPM_REG(0x16c)
#define SPM_PWR_STATUS_2ND		SPM_REG(0x170)
#define SPM_CPU_PWR_STATUS		SPM_REG(0x174)

/* bit-fields of SPM_PWR_STATUS */
#define MD				BIT(0)
#define CONN				BIT(1)
#define DDRPHY				BIT(2)
#define DISP				BIT(3)
#define MFG				BIT(4)
#define ISP				BIT(5)
#define INFRA				BIT(6)
#define VDEC				BIT(7)
#define MP0_CPUTOP			BIT(8)
#define MP0_CPU0			BIT(9)
#define MP0_CPU1			BIT(10)
#define MP0_CPU2			BIT(11)
#define MP0_CPU3			BIT(12)
#define MCUSYS				BIT(14)
#define MP0_CPU4			BIT(15)
#define MP0_CPU5			BIT(16)
#define MP0_CPU6			BIT(17)
#define MP0_CPU7			BIT(18)
#define VEN				BIT(21)

/* === SPMC related registers */
#define SPM_MCUSYS_PWR_CON		MCUCFG_REG(0xd200)
#define SPM_MP0_CPUTOP_PWR_CON		MCUCFG_REG(0xd204)
#define SPM_MP0_CPU0_PWR_CON		MCUCFG_REG(0xd208)
#define SPM_MP0_CPU1_PWR_CON		MCUCFG_REG(0xd20c)
#define SPM_MP0_CPU2_PWR_CON		MCUCFG_REG(0xd210)
#define SPM_MP0_CPU3_PWR_CON		MCUCFG_REG(0xd214)
#define SPM_MP0_CPU4_PWR_CON		MCUCFG_REG(0xd218)
#define SPM_MP0_CPU5_PWR_CON		MCUCFG_REG(0xd21c)
#define SPM_MP0_CPU6_PWR_CON		MCUCFG_REG(0xd220)
#define SPM_MP0_CPU7_PWR_CON		MCUCFG_REG(0xd224)

/* bit fields of SPM_*_PWR_CON */
#define PWR_ON_ACK			BIT(31)
#define VPROC_EXT_OFF			BIT(7)
#define DORMANT_EN			BIT(6)
#define RESETPWRON_CONFIG		BIT(5)
#define PWR_CLK_DIS			BIT(4)
#define PWR_ON				BIT(2)
#define PWR_RST_B			BIT(0)

/**** per_cpu registers for SPM_MP0_CPU?_PWR_CON */
static const struct per_cpu_reg SPM_CPU_PWR[] = {
	{ .cluster_addr = SPM_MP0_CPU0_PWR_CON, .cpu_stride = 2U }
};

/**** per_cluster registers for SPM_MP0_CPUTOP_PWR_CON */
static const struct per_cpu_reg SPM_CLUSTER_PWR[] = {
	{ .cluster_addr = SPM_MP0_CPUTOP_PWR_CON, .cpu_stride = 0U }
};

/* === MCUCFG related registers */
/* aa64naa32 */
#define MCUCFG_MP0_CLUSTER_CFG5		MCUCFG_REG(0xc8e4)
/* reset vectors */
#define MCUCFG_MP0_CLUSTER_CFG8		MCUCFG_REG(0xc900)
#define MCUCFG_MP0_CLUSTER_CFG10	MCUCFG_REG(0xc908)
#define MCUCFG_MP0_CLUSTER_CFG12	MCUCFG_REG(0xc910)
#define MCUCFG_MP0_CLUSTER_CFG14	MCUCFG_REG(0xc918)
#define MCUCFG_MP0_CLUSTER_CFG16	MCUCFG_REG(0xc920)
#define MCUCFG_MP0_CLUSTER_CFG18	MCUCFG_REG(0xc928)
#define MCUCFG_MP0_CLUSTER_CFG20	MCUCFG_REG(0xc930)
#define MCUCFG_MP0_CLUSTER_CFG22	MCUCFG_REG(0xc938)

/* MCUSYS DREQ BIG VPROC ISO control */
#define DREQ20_BIG_VPROC_ISO		MCUCFG_REG(0xad8c)

/**** per_cpu registers for MCUCFG_MP0_CLUSTER_CFG? */
static const struct per_cpu_reg MCUCFG_BOOTADDR[] = {
	{ .cluster_addr = MCUCFG_MP0_CLUSTER_CFG8, .cpu_stride = 3U }
};

/**** per_cpu registers for MCUCFG_MP0_CLUSTER_CFG5 */
static const struct per_cpu_reg MCUCFG_INITARCH[] = {
	{ .cluster_addr = MCUCFG_MP0_CLUSTER_CFG5, .cpu_stride = 0U }
};

#define MCUCFG_INITARCH_CPU_BIT(cpu)	BIT(16U + cpu)
/* === CPC control */
#define MCUCFG_CPC_FLOW_CTRL_CFG	MCUCFG_REG(0xa814)
#define MCUCFG_CPC_SPMC_PWR_STATUS	MCUCFG_REG(0xa840)

/* bit fields of CPC_FLOW_CTRL_CFG */
#define CPC_CTRL_ENABLE			BIT(16)
#define SSPM_CORE_PWR_ON_EN		BIT(7) /* for cpu-hotplug */
#define SSPM_ALL_PWR_CTRL_EN		BIT(13) /* for cpu-hotplug */
#define GIC_WAKEUP_IGNORE(cpu)		BIT(21 + cpu)

/* bit fields of CPC_SPMC_PWR_STATUS */
#define CORE_SPMC_PWR_ON_ACK		GENMASK(11, 0)

/* === APB Module infracfg_ao */
#define INFRA_TOPAXI_PROTECTEN		INFRACFG_AO_REG(0x0220)
#define INFRA_TOPAXI_PROTECTEN_STA0	INFRACFG_AO_REG(0x0224)
#define INFRA_TOPAXI_PROTECTEN_STA1	INFRACFG_AO_REG(0x0228)
#define INFRA_TOPAXI_PROTECTEN_SET	INFRACFG_AO_REG(0x02a0)
#define INFRA_TOPAXI_PROTECTEN_CLR	INFRACFG_AO_REG(0x02a4)
#define INFRA_TOPAXI_PROTECTEN_1	INFRACFG_AO_REG(0x0250)
#define INFRA_TOPAXI_PROTECTEN_STA0_1	INFRACFG_AO_REG(0x0254)
#define INFRA_TOPAXI_PROTECTEN_STA1_1	INFRACFG_AO_REG(0x0258)
#define INFRA_TOPAXI_PROTECTEN_1_SET	INFRACFG_AO_REG(0x02a8)
#define INFRA_TOPAXI_PROTECTEN_1_CLR	INFRACFG_AO_REG(0x02ac)

/* bit fields of INFRA_TOPAXI_PROTECTEN */
#define MP0_SPMC_PROT_STEP1_0_MASK	BIT(12)
#define MP0_SPMC_PROT_STEP1_1_MASK	(BIT(26) | BIT(12))

/* === SPARK */
#define VOLTAGE_04			U(0x40)
#define VOLTAGE_05			U(0x60)

#define PTP3_CPU0_SPMC_SW_CFG		MCUCFG_REG(0x200)
#define CPU0_ILDO_CONTROL5		MCUCFG_REG(0x334)
#define CPU0_ILDO_CONTROL8		MCUCFG_REG(0x340)

/* bit fields of CPU0_ILDO_CONTROL5 */
#define ILDO_RET_VOSEL			GENMASK(7, 0)

/* bit fields of PTP3_CPU_SPMC_SW_CFG */
#define SW_SPARK_EN			BIT(0)

/* bit fields of CPU0_ILDO_CONTROL8 */
#define ILDO_BYPASS_B			BIT(0)

static const struct per_cpu_reg MCUCFG_SPARK[] = {
	{ .cluster_addr = PTP3_CPU0_SPMC_SW_CFG, .cpu_stride = 11U }
};

static const struct per_cpu_reg ILDO_CONTROL5[] = {
	{ .cluster_addr = CPU0_ILDO_CONTROL5, .cpu_stride = 11U }
};

static const struct per_cpu_reg ILDO_CONTROL8[] = {
	{ .cluster_addr = CPU0_ILDO_CONTROL8, .cpu_stride = 11U }
};

#endif /* MTSPMC_PRIVATE_H */
